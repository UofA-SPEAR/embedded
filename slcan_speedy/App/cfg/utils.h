#ifndef CAN_UTILS_H__
#define CAN_UTILS_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void bin2hex(uint8_t *dst, unsigned char c);
uint8_t nibble2bin(uint8_t s);
uint8_t hex2bin(char *s);


#ifdef __cplusplus
}
#endif

#endif