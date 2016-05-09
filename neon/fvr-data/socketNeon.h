#ifndef __SOCKETAES_H__
# define __SOCKETAES_H__

#include <stdint.h>

typedef union bitpacketUnion {
  char chr[4];
  uint32_t data;
} bitPacket;


uint32_t getData(int sessionfd, char* dest, uint32_t maxBuffer);
uint32_t putData(int sessionfd, char* src, uint32_t buffLen);


#endif /* __SOCKETAES_H__ */
