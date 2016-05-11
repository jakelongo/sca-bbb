// GPIO trigger source

 /* GPIO Libs */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>

#include "trigger.h"
 /* GPIO Libs */

/* GPIO Stuff */
#define MMAP_OFFSET (0x44C00000)
#define MMAP_SIZE   (0x481AEFFF-MMAP_OFFSET)

#define GPIO0_BASE 0x44E07000
#define GPIO1_BASE 0x4804C000
#define GPIO2_BASE 0x481AC000

#define GPIO_SIZE  0x00001FFF

#define GPIO_OE  0x134
#define GPIO_SET 0x194
#define GPIO_CLR 0x190
#define GPIO_DO  0x13C

#define PIN_BIT 3

static uint32_t    *gpioBase;
static char         trigger_mapActive = 0;

int trigger_init(void){
  int memfd;

  memfd = open("/dev/mem", O_RDWR);
  if (memfd == -1){
    perror("trigger_init: Unabe to open /dev/mem");
    return -1;
  }

  gpioBase = (uint32_t*)mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, GPIO2_BASE);

  if (gpioBase == MAP_FAILED){
    close(memfd);
    perror("trigger_init: mmap to gpio base failed");
    return -1;
  }

  trigger_mapActive = 1;
  return 1;
}

int trigger_close(void){

  if (trigger_mapActive) {
    munmap(gpioBase, MMAP_SIZE);
    trigger_mapActive = 0;
    return 1;
  } else {
    return 0;
  }

}

int trigger_setup(void){

  uint32_t *gpioOE = NULL;

  if (trigger_mapActive) {
     gpioOE   = gpioBase + (GPIO_OE/4);
    *gpioOE  &= ~(1 << PIN_BIT);
    return 1;
  } else {
    perror("trigger_setup: trigger memory map not active!");
    return 0;
  }

}

int trigger_set(void){

  uint32_t* gpioSET  = NULL;

  if (trigger_mapActive) {
     gpioSET  = gpioBase + (GPIO_SET/4);
    *gpioSET  = (1 << PIN_BIT);
    return 1;
  } else {
    perror("trigger_set: trigger memory map not active!");
    return 0;
  }

}


int trigger_clr(void){

  uint32_t* gpioCLR  = NULL;

  if (trigger_mapActive) {
     gpioCLR  = gpioBase + (GPIO_CLR/4);
    *gpioCLR  = (1 << PIN_BIT);
    return 1;
  } else {
    perror("trigger_clr: trigger memory map not active!");
    return 0;
  }

}

/* GPIO Stuff */
