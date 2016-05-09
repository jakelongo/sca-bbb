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
  closeMap();
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

  uint32_t    scratchVariable;
  char        cmdIdentifier;
  bitPacket   streamLength;

  uint8_t    memBank[BANK_SIZE][BANK_WIDTH];
  char       opString[512];

  // Internal flags
  uint8_t closeServer = 0;

  unsigned int i;

  unsigned int bankIndex = 0;
  void (*opFunction)(uint8_t**);

  #ifdef DEBUG
  printf("Initialising GPIO\n");
  #endif /* DEBUG */

  initMap();
  setupTrigger();

  while(!closeServer)
  {
    read(sessionfd, &cmdIdentifier, 1);
    #ifdef DEBUG
    printf("CMD=%c \n", cmdIdentifier);
    #endif /* DEBUG */


    switch(cmdIdentifier)
    {

      // Set key and check the key length
      case 'w':

        // Check which bank (0-2)
        bankIndex = 0;
        i = 0;
        while (i < 1) {
          i += read(sessionfd, (char*) &bankIndex, 1-i);
        }

        #ifdef DEBUG
        printf("Write bank index received, membank[%d]\n", bankIndex);
        #endif /* DEBUG */

        if (bankIndex > (BANK_SIZE-1)) {
          printf("Invalid bank index: %d", bankIndex);
          return;
        }

        scratchVariable = getData(sessionfd, memBank[bankIndex], BANK_WIDTH);

        #ifdef DEBUG
        printf("Write membank[%d] = ", bankIndex);
        for (i = 0 ; i < BANK_WIDTH; ++i) {
          printf("%02X", memBank[bankIndex][i]);
        }
        printf("\n");
        #endif /* DEBUG */

        break;

      // Read out register
      case 'r':

        // Check which bank (0-2)
        bankIndex = 0;
        i = 0;
        while (i < 1) {
          i += read(sessionfd, (char*) &bankIndex, 1-i);
        }

        #ifdef DEBUG
        printf("Read bank index received, membank[%d]\n", bankIndex);
        #endif /* DEBUG */

        if (bankIndex > (BANK_SIZE-1)) {
          printf("Invalid bank index: %d", bankIndex);
          return;
        }

        putData(sessionfd, memBank[bankIndex], BANK_WIDTH);

        #ifdef DEBUG
        printf("Read membank[%d] = ", bankIndex);
        for (i = 0 ; i < BANK_WIDTH; ++i) {
          printf("%02X", memBank[bankIndex][i]);
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
        if        (!strncmp("vmul", opString)) {
          opFunction = &neonExecute_vmulu32;
        } else if (!strncmp("vadd", opString)) {
          opFunction = &neonExecute_vaddu32;
        } else if (!strncmp("vsub", opString)) {
          opFunction = &neonExecute_vsubu32;
        } else if (!strncmp("vceq", opString)) {
          opFunction = &neonExecute_vcequ32;
        } else if (!strncmp("veor", opString)) {
          opFunction = &neonExecute_veoru32;
        }

        break;

      case 'e':

        // Check which bank
        bankIndex = 0;
        i = 0;
        while (i < 1) {
          i += read(sessionfd, (char*) &bankIndex, 1-i);
        }

        #ifdef DEBUG
          printf("Performing Operation...");
        #endif /* DEBUG */

        (*opFunction)((uint8_t**)memBank);

        #ifdef DEBUG
        printf("Read bank index received, membank[%d]\n", bankIndex);
        #endif /* DEBUG */

        if (bankIndex > (BANK_SIZE-1)) {
          printf("Invalid bank index: %d", bankIndex);
          return;
        }

        #ifdef DEBUG
        i = 0;
        printf("Done\n");
        printf("Read membank[0] = ");
        for (i = 0 ; i < BANK_WIDTH; ++i) {
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

        break;

    }

  }

  closeMap();
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

    /* End of Socket Setup */
    commandHandler(sessionfd);

    sleep(1);
  }

  // Destructor for socket
  close(listenfd);

}
