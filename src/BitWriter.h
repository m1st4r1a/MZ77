#ifndef BITWRITER_H
#define BITWRITER_H 1
#include "dt.h"
void BitWriter_init (BitWriter *bw, FD fd);
void BitWriter_flush (BitWriter *bw);
void BitWriter_finish (BitWriter *bw);
void BitWriter_write (BitWriter *bw, DWORD value, int bits);
#endif
