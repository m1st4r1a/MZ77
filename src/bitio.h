#ifndef BITIO
#define BITIO 1
#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *fp;
    uint64_t buf;
    int bits;
} BitReader;

typedef struct {
    FILE *fp;
    uint64_t buf;
    int bits;
} BitWriter;

void br_init(BitReader *br, FILE *fp);
int  br_read(BitReader *br, int n); // Returns -1 on EOF/error

void bw_init(BitWriter *bw, FILE *fp);
void bw_write(BitWriter *bw, uint32_t val, int n);
void bw_flush(BitWriter *bw);
void bw_finish(BitWriter *bw);
#endif
