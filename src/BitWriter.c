#include "dt.h"

void
BitWriter_init (BitWriter *bw, FD fd)
{
  bw->fd = fd;
  bw->buffer = bw->bits_in_buffer = 0;
}

void
BitWriter_flush (BitWriter *bw)
{
  while (bw->bits_in_buffer >= 8)
    {
      BYTE byte = (bw->buffer >> (bw->bits_in_buffer - 8)) & 0xFF;
      fputc (byte, bw->fd);
      bw->bits_in_buffer -= 8;
    }
}

void
BitWriter_finish (BitWriter *bw)
{
  if (bw->bits_in_buffer > 0)
    {
      BYTE byte = (bw->buffer << (8 - bw->bits_in_buffer)) & 0xFF;
      fputc (byte, bw->fd);
    }
}

void
BitWriter_write (BitWriter *bw, DWORD value, int bits)
{
  bw->buffer = (bw->buffer << bits) | (value & ((1U << bits) - 1));
  bw->bits_in_buffer += bits;
  BitWriter_flush (bw);
}
