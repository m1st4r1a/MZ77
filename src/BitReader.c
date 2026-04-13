#include "dt.h"

void
BitReader_init (BitReader *br, FD fp)
{
  br->fp = fp;
  br->buffer = 0;
  br->bits = 0;
}

DWORD
BitReader_read (BitReader *br, int n)
{
  while (br->bits < n)
    {
      int c = fgetc (br->fp);
      if (c == EOF)
	return -1;
      br->buffer = (br->buffer << 8) | (DWORD) c;
      br->bits += 8;
    }
  br->bits -= n;
  return (DWORD) ((br->buffer >> br->bits) & ((1ULL << n) - 1));
}
