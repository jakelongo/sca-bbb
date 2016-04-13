// GPIO trigger source

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>

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

static uint32_t *trigger_gpioBase;
static uint8_t   trigger_active = 0;

int trigger_map(void){

    // variable for mem file descriptor
    int memfd;

    // check if we have root permissions to be doing this!
    if ( getuid() != 0 ) {
        perror("trigger_initMap : this program requires root permissions to run!");
        return -1;
    }

    // open mem mapper
    memfd = open("/dev/mem", O_RDWR);

    // check if there was a problem
    if (memfd == -1){
        perror("initMap : Unabe to open /dev/mem");
        return -1;
    }

    // map the GPIO2 physical address as R/W
    trigger_gpioBase = (uint32_t*)mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, GPIO2_BASE);

    // check for any problems
    if (gpioBase == MAP_FAILED){
        close(memfd);
        perror("initMap : mmap to gpio base failed");
        return -1;
    }

    // set the global map active variable...this is shocking but...meh
    trigger_active = 1;
    return 1;
}

int trigger_unmap(void){

  // check if the memory has been mapped
  if (trigger_active) {
    // unmap the memory and clear the global flag
    munmap(trigger_gpioBase, MMAP_SIZE);
    mapActive = 0;
    return 1;

  } else {
    // if it's not open...then we just ignore the call
    return 0;
  }

}

// setup triggger for output signal
int trigger_setup(void){
    uint32_t *gpioOE = NULL;
    gpioOE   = trigger_gpioBase + (GPIO_OE/4);
    *gpioOE  &= ~(1 << PIN_BIT);
    return 0;
}

// set the trigger to logic 1
int trigger_set(void){
    uint32_t* gpioSET  = NULL;
    gpioSET   = trigger_gpioBase + (GPIO_SET/4);
    *gpioSET  = (1 << PIN_BIT);
    return 0;
}

// set the trigger to logic 0
int trigger_clr(void){
    uint32_t* gpioCLR  = NULL;
    gpioCLR   = trigger_gpioBase + (GPIO_CLR/4);
    *gpioCLR  = (1 << PIN_BIT);
    return 0;
}
