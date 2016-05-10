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

#define BANK_SIZE   8
#define BANK_WIDTH  8
// #define DEBUG

// Dreaded globals!
volatile int listenfd  = 0;
volatile int sessionfd = 0;


// Housekeeping if program interrupted
void intHandler(int var)
{
  printf("SIGINT : Closing down session\n");
  close(sessionfd);
  close(listenfd);
  trigger_close();
  exit(-1);
}

void printMemory(uint32_t memsize, uint8_t* memptr){
  for (uint32_t cntr = 0; cntr < memsize; ++cntr){
    printf("%02X", memptr[cntr]);
  }
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
  #endif /* DEBUG */

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
  printf("Payload received: ");
  printMemory(byteCounter, dest);
  printf("\n");
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
        printf("Write membank[%d] = ", memBankIndex);
        printMemory(scratchVariable, memBank[memBankIndex]);
        printf("\n");
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
        for (cntr = 0 ; cntr < BANK_WIDTH; ++cntr) {
          printf("%02X", memBank[memBankIndex][cntr]);
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

        // wow.....just.....wow
        if        (!strncmp("vandu64", opString, 512)) {
          opFunction = &neonExecute_vandu64;
        } else if (!strncmp("veoru64", opString, 512)) {
          opFunction = &neonExecute_veoru64;
        } else if (!strncmp("vmuli8 ", opString, 512)) {
          opFunction = &neonExecute_vmuli8 ;
        } else if (!strncmp("vaddi8 ", opString, 512)) {
          opFunction = &neonExecute_vaddi8 ;
        } else if (!strncmp("_vsubi8 ", opString, 512)) {
          opFunction = &neonExecute_vsubi8 ;
        } else if (!strncmp("vmuli16", opString, 512)) {
          opFunction = &neonExecute_vmuli16;
        } else if (!strncmp("vaddi16", opString, 512)) {
          opFunction = &neonExecute_vaddi16;
        } else if (!strncmp("vsubi16", opString, 512)) {
          opFunction = &neonExecute_vsubi16;
        } else if (!strncmp("vmuli32", opString, 512)) {
          opFunction = &neonExecute_vmuli32;
        } else if (!strncmp("vaddi32", opString, 512)) {
          opFunction = &neonExecute_vaddi32;
        } else if (!strncmp("vsubi32", opString, 512)) {
          opFunction = &neonExecute_vsubi32;
        } else if (!strncmp("vaddi64", opString, 512)) {
          opFunction = &neonExecute_vaddi64;
        } else if (!strncmp("vsubi64", opString, 512)) {
          opFunction = &neonExecute_vsubi64;
        }

        break;

      case 'e':

        // Check which bank
        #ifdef DEBUG
          printf("Performing Operation...\n");
        #endif /* DEBUG */

        (*opFunction)((uint8_t**)memBank);

        break;

      case 'x':

        printf("Exit command received - closing down.\n");
        closeServer = 0x01;

        break;

      default:

        printf("Unkonwn command received %c\n Closing down server.\n", cmdIdentifier);
        closeServer = 0x01;
        intHandler(-1);
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
