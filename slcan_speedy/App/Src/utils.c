#include "utils.h"

void bin2hex(uint8_t *dst, unsigned char c) {
    uint8_t nibble;

    nibble = ((c & 0xf0) >> 4) + '0';
    if (nibble >= 0x3a)
	nibble += 7;
    *dst++ = nibble;

    nibble = (c & 0x0f) + '0';
    if (nibble >= 0x3a)
	nibble += 7;
    *dst = nibble;
}

uint8_t nibble2bin(uint8_t s) {

    if (s >= '0' && s <= '9')
	return s - '0';
    if (s >= 'A' && s <= 'F')
	return s - 'A' + 10;
    return s - 'a' + 10;
}

uint8_t hex2bin(char *s) {
    uint8_t x;

    x = nibble2bin(*s++) << 4;
    x |= nibble2bin(*s);
    return x;
}