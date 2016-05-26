// Author: Jake Longo

// Description: target device socket 'server' for executing NEON
// instructions on the target device.

// Note: I'm sure there are plenty of bugs but this is designed
// to facilitate signal exploration and not 'typical' use cases.
// (ie., give it undefined input and you should expect garbage!)

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

// WolfSSL headers
#include <wolfssl/wolfcrypt/aes.h>

// Custom header
#include "trigger.h"

typedef union bitpacketUnion {
  char chr[4];
  uint32_t data;
} bitPacket;

#define ascii2Bin(x) ( ((x)>='0' && (x)<='9') ? (x)-'0' : 10+tolower(x)-'a' )
#define bin2Ascii(x) ( ((x)<=9) ? (x)+'0' : (x)+'A'-10 )

// Up to 64KB
#define DATABUFFSIZE 4096*16*sizeof(uint8_t)

// #define DEBUG

// Dreaded globals!
volatile int listenfd  = 0;
volatile int sessionfd = 0;

// bailing function in case something bad happens!
void bail(void) {
  close(sessionfd);
  close(listenfd);
  trigger_unmap();
}


// Housekeeping if program is interrupted
void intHandler(int var)
{
  printf("SIGINT : Closing down session\n");
  bail();
  exit(-1);
}

void printMemory(uint32_t memsize, uint8_t* memptr){
  for (uint32_t cntr = 0; cntr < memsize; ++cntr){
    printf("%02X", memptr[cntr]);
  }
}

int32_t getData(int sessionfd, char* dest, uint32_t maxBuffer)
{
  bitPacket   streamLength;
  uint32_t    byteCounter;
  char        terminationChar;

  byteCounter = 0;
  while (byteCounter < 4) {
    byteCounter += read(sessionfd, &(streamLength.chr[byteCounter]), 4-byteCounter);
  }

  #ifdef DEBUG
  printf("Payload length = %d\n", streamLength.data);
  #endif /* DEBUG */

  if (streamLength.data > maxBuffer) {
    printf("Payload length is greater than max buffer %d\n", streamLength.data);
    return -1;
  }

  byteCounter = 0;
  while (byteCounter < streamLength.data) {
    byteCounter += read(sessionfd, &(dest[byteCounter]), streamLength.data-byteCounter);
  }

  #ifdef DEBUG
  printf("Payload received\n");
  #endif /* DEBUG */

  byteCounter = 0;
  while (byteCounter < 1) {
    byteCounter += read(sessionfd, &terminationChar, 1-byteCounter);
  }

  if (terminationChar != '\n') {
    printf("Invalid termination character\n");
    printf("Received Instead - %02X\n", terminationChar);
    return -1;
  }

  return streamLength.data;
}

int32_t putData(int sessionfd, char* src, uint32_t buffLen)
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

void aesEncrypt(Aes *aesCtx, uint8_t *key, uint8_t *pt, uint8_t *ct, uint8_t *iv, uint8_t keySize, uint32_t buffSize){

  int32_t ret;

  ret = wc_AesSetKey(aesCtx, key, keySize, iv, AES_ENCRYPTION);

  if (0 != ret) {
    printf("Error setting key in wolfSSL\n");
    return;
  }

  ret = wc_AesCbcEncrypt(aesCtx, ct, pt, buffSize);

  if (0 != ret) {
    printf("Error on encryption call to wolfSSL\n");
    return;
  }

}


void commandHandler(int sessionfd)
{
  char        cmdIdentifier;
  bitPacket   streamLength;

  int32_t     scratchVariable;
  char        opString[512];

  uint8_t     closeServer = 0;
  uint32_t    cntr;

  uint8_t     ptBuffer[DATABUFFSIZE];
  uint8_t     ctBuffer[DATABUFFSIZE];
  uint8_t     keyBuffer[32];
  uint8_t     ivBuffer[32];

  uint32_t    activeBuffer;
  uint8_t     keySize;

  Aes         aesCtx;

  #ifdef DEBUG
  printf("Initialising GPIO\n");
  #endif /* DEBUG */

  trigger_map();
  trigger_setup();

  // ensure the buffer is considered 'empty' at the start
  activeBuffer = 0;
  keySize      = 16;

  // set the IV to zero
  memset(ivBuffer, 0x00, 16);

  while(!closeServer)
  {

    read(sessionfd, &cmdIdentifier, 1);

    #ifdef DEBUG
    printf("CMD=%c \n", cmdIdentifier);
    #endif /* DEBUG */

    switch(cmdIdentifier) {

      // Fill memory buffer
      case 'k':

        scratchVariable = getData(sessionfd, keyBuffer, 32);

        // if there was an error while receiving the payload
        if (scratchVariable < 0) {
          switch(scratchVariable) {
            case 16:
              keySize = 16;
              break;

            case 24:
              keySize = 24;
              break;

            case 32:
              keySize = 32;
              break;

            default:
              printf("Error, key received is of an incorrect length: %d\n", scratchVariable);
              keySize = 16;
              break;
          }

        #ifdef DEBUG
        printf("Key Size @ %d\n", keySize);
        #endif /* DEBUG */

        }

        break;

      // Fill the IV buffer
      case 'i':

        scratchVariable = getData(sessionfd, ivBuffer, 16);

        // if there was an error while receiving the payload
        if (scratchVariable < 0) {
          printf("Error in receiving IV!\n");
        }

        break;

      // Fill memory buffer
      case 'm':

        scratchVariable = getData(sessionfd, ptBuffer, DATABUFFSIZE);

        // if there was an error while receiving the payload
        if (scratchVariable < 0) {
          printf("Error in receiving payload!");
        } else {
          activeBuffer = (uint32_t) scratchVariable;
        }

        break;

      // Read out the ciphertext buffer
      case 'r':

        scratchVariable = putData(sessionfd, ctBuffer, activeBuffer);

        break;

      case 'e':

        // Encrypt!
        #ifdef DEBUG
          printf("Performing encryption...\n");
        #endif /* DEBUG */



        break;

      case 'x':

        #ifdef DEBUG
        printf("Exit command received - closing down.\n");
        #endif /* DEBUG */
        closeServer = 0x01;

        break;

      default:

        printf("Unkonwn command received %c\n Closing down server.\n", cmdIdentifier);
        closeServer = 0x01;
        intHandler(-1);
        break;

    }

  }

  trigger_unmap();
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
