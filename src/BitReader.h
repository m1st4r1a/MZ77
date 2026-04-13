#ifndef BITREADER_H
#define BITREADER_H 1
#include "dt.h"
void BitReader_init(BitReader *br, FD fp);
DWORD BitReader_read(BitReader *br, int n);
#endif
