// Author: Jake Longo

// Description: targate device socket 'server' for controlling execution
// on the target device.

// Standard header files
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

// socket header files
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Custom header
#include "socketNeon.h"
#include "trigger.h"

// Function declarations
#include "neonExecute_INSTR.h"

#define ascii2Bin(x) ( ((x)>='0' && (x)<='9') ? (x)-'0' : 10+tolower(x)-'a' )
#define bin2Ascii(x) ( ((x)<=9) ? (x)+'0' : (x)+'A'-10 )

#define DATABUFFSIZE 4096*16*sizeof(uint8_t)

#define BANK_SIZE   4
#define BANK_WIDTH  4
// #define DEBUG

// Dreaded globals!
volatile int listenfd  = 0;
volatile int sessionfd = 0;

char *operationString[] = {"vmul", "vadd", "vsub", "veor", "vceq"};


// Housekeeping if program interrupted
void intHandler(int var)
{
  printf("SIGINT : Closing down session\n");
  close(sessionfd);
  close(listenfd);
  trigger_close();
  exit(-1);
}

uint32_t getData(int sessionfd, char* dest, uint32_t maxBuffer)
{
  bitPacket   streamLength;
  uint32_t    byteCounter;
  char        terminationChar;

  byteCounter = 0;
  while (byteCounter < 4)
  {
    byteCounter += read(sessionfd, &(streamLength.chr[byteCounter]), 4-byteCounter);
  }

  #ifdef DEBUG
  printf("Payload length = %d\n", streamLength.data);
  #endif /* DEBUG *

  if (streamLength.data > maxBuffer)
  {
    printf("Payload length is greater than max buffer %d\n", streamLength.data);
  }

  byteCounter = 0;
  while (byteCounter < streamLength.data)
  {
    byteCounter += read(sessionfd, &(dest[byteCounter]), streamLength.data-byteCounter);
  }

  #ifdef DEBUG
  printf("Payload received\n");
  #endif /* DEBUG */

  byteCounter = 0;
  while (byteCounter < 1)
  {
    byteCounter += read(sessionfd, &terminationChar, 1-byteCounter);
  }

  if (terminationChar != '\n')
  {
    printf("Invalid termination character\n");
    printf("Received Instead - %02X\n", terminationChar);
    return -1;
  }

  return streamLength.data;

}

uint32_t putData(int sessionfd, char* src, uint32_t buffLen)
{
  bitPacket   streamLength;
  uint32_t    byteCounter;
  char        terminationChar;

  streamLength.data = buffLen;

  byteCounter = 0;
  while (byteCounter < 4) {
    byteCounter += write(sessionfd, &(streamLength.chr[byteCounter]), 4-byteCounter);
  }

  byteCounter = 0;
  while (byteCounter < streamLength.data) {
    byteCounter += write(sessionfd, &(src[byteCounter]), streamLength.data-byteCounter);
  }

  #ifdef DEBUG
  printf("Payload Sent\n");
  #endif /* DEBUG */

  terminationChar = '\n';

  byteCounter = 0;
  while (byteCounter < 1) {
    byteCounter += write(sessionfd, &terminationChar, 1-byteCounter);
  }

  return streamLength.data;

}


void commandHandler(int sessionfd)
{
  char        cmdIdentifier;
  bitPacket   streamLength;

  uint32_t    scratchVariable;
  uint32_t    memBankIndex;
  uint8_t     memBank[BANK_SIZE][BANK_WIDTH];
  char        opString[512];

  // Internal flags
  uint8_t       closeServer = 0;
  unsigned int  cntr;
  void          (*opFunction)(uint8_t**);

  #ifdef DEBUG
  printf("Initialising GPIO\n");
  #endif /* DEBUG */

  trigger_init();
  trigger_setup();

  while(!closeServer)
  {
    read(sessionfd, &cmdIdentifier, 1);
    #ifdef DEBUG
    printf("CMD=%c \n", cmdIdentifier);
    #endif /* DEBUG */


    switch(cmdIdentifier)
    {

      // Set data in register banks
      case 'w':

        // Check which bank (0-2)
        memBankIndex = 0;
        cntr      = 0;

        while (cntr < 1) {
          cntr += read(sessionfd, (char*) &memBankIndex, 1 - cntr);
        }

        #ifdef DEBUG
        printf("Write bank index received, membank[%d]\n", memBankIndex);
        #endif /* DEBUG */

        if (memBankIndex > (BANK_SIZE-1)) {
          printf("Invalid bank index: %d", memBankIndex);
          return;
        }

        scratchVariable = getData(sessionfd, memBank[memBankIndex], (BANK_WIDTH*BANK_SIZE)-(BANK_WIDTH*memBankIndex));

        #ifdef DEBUG
        for (int wordCntr = 0; wordCntr < (scratchVariable/BANK_WIDTH); ++wordCntr) {
          printf("Write membank[%d] = ", memBankIndex+wordCntr);
          for (cntr = 0 ; cntr < BANK_WIDTH; ++i) {
            printf("%02X", memBank[memBankIndex+wordCntr][i]);
          }
          printf("\n");
        }
        #endif /* DEBUG */

        break;

      // Read out register
      case 'r':

        // Check which bank (0-2)
        memBankIndex = 0;
        cntr = 0;
        while (cntr < 1) {
          cntr += read(sessionfd, (char*) &memBankIndex, 1 - cntr);
        }

        #ifdef DEBUG
        printf("Read bank index received, membank[%d]\n", memBankIndex);
        #endif /* DEBUG */

        if (memBankIndex > (BANK_SIZE-1)) {
          printf("Invalid bank index: %d", memBankIndex);
          return;
        }

        putData(sessionfd, memBank[memBankIndex], BANK_WIDTH);

        #ifdef DEBUG
        printf("Read membank[%d] = ", memBankIndex);
        for (cntr = 0 ; cntr < BANK_WIDTH; ++i) {
          printf("%02X", memBank[memBankIndex][i]);
        }
        printf("\n");
        #endif /* DEBUG */

        break;

      // Select operation from operationString[]
      case 'o':

        // Check what operation to perform
        scratchVariable = getData(sessionfd, opString, 512);

        #ifdef DEBUG
          printf("Operation \"%s\" selected\n", opString);
        #endif /* DEBUG */

        // Set up function pointer for exec call
        if        (!strncmp("vmul", opString, 511)) {
          opFunction = &neonExecute_vmulu32;
        } else if (!strncmp("vadd", opString, 511)) {
          opFunction = &neonExecute_vaddu32;
        } else if (!strncmp("vsub", opString, 511)) {
          opFunction = &neonExecute_vsubu32;
        } else if (!strncmp("vceq", opString, 511)) {
          opFunction = &neonExecute_vcequ32;
        } else if (!strncmp("veor", opString, 511)) {
          opFunction = &neonExecute_veoru32;
        }

        break;

      case 'e':

        // Check which bank
        memBankIndex = 0;
        cntr = 0;
        while (cntr < 1) {
          cntr += read(sessionfd, (char*) &memBankIndex, 1 - cntr);
        }

        #ifdef DEBUG
          printf("Performing Operation...");
        #endif /* DEBUG */

        (*opFunction)((uint8_t**)memBank);

        #ifdef DEBUG
        printf("Read bank index received, membank[%d]\n", memBankIndex);
        #endif /* DEBUG */

        if (memBankIndex > (BANK_SIZE-1)) {
          printf("Invalid bank index: %d", memBankIndex);
          return;
        }

        #ifdef DEBUG
        cntr = 0;
        printf("Done\n");
        printf("Read membank[0] = ");
        for (cntr = 0 ; cntr < BANK_WIDTH; ++i) {
          printf("%02X", memBank[0][i]);
        }
        printf("\n");
        #endif /* DEBUG */

        break;

      case 'x':

        printf("Exit command received - closing down.\n");
        closeServer = 0x01;

        break;

      default:

        printf("Unkonwn command received %c\n Closing down server.\n", cmdIdentifier);
        closeServer = 0x01;
        intHandler();
        break;

    }

  }

  trigger_close();
  close(sessionfd);

}

int main(int argc, char *argv[])
{
  /* Socket Setup */
  // default socket number
  int socketNumber = 5000;

  if (argc > 1){
    socketNumber = atoi(argv[1]);
  }

  #ifdef DEBUG
  printf("starting server on socket: %d\n", socketNumber);
  #endif /* DEBUG */

  // Set up socket first
  struct sockaddr_in serv_addr;

  signal(SIGINT, intHandler);

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&serv_addr,  '0', sizeof(serv_addr));

  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port        = htons(socketNumber);

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  listen(listenfd, 10);

  while (1) {
    sessionfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    #ifdef DEBUG
    printf("client connected!\n");
    #endif /* DEBUG */

    /* End of Socket Setup */
    commandHandler(sessionfd);

    #ifdef DEBUG
    printf("closing connection!\n");
    #endif /* DEBUG */

    sleep(1);
  }

  // Destructor for socket
  close(listenfd);

}
