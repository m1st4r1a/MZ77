#ifndef BITWRITER_H
#define BITWRITER_H
#include "dt.h"
typedef struct {
	FILE *fd;
	uint32_t buffer;
	int bits_in_buffer;
} BitWriter;
void BitWriter_init (BitWriter *bw, FD fd);
void BitWriter_flush (BitWriter *bw);
void BitWriter_finish (BitWriter *bw);
void BitWriter_write (BitWriter *bw, DWORD value, int bits);
#endif
