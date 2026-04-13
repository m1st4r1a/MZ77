#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include<errno.h>
#include "dt.h"
#include "BitReader.h"

DWORD
decompress (STR input_path, STR output_path)
{
  FD in = fopen (input_path, "rb");
  if (!in)
    return ENOENT;
  FD out = fopen (output_path, "wb");
  if (!out)
    {
      fclose (in);
      return ENOENT;
    }
  static BYTE window[WINDOW_SIZE] = { 0 };
  DWORD win_pos = 0;
  BitReader br;
  BitReader_init (&br, in);
  while (true)
    {
      int flag = BitReader_read (&br, 1);
      if (flag == -1)
	break;
      if (flag == 0)		// literal
	{
	  int lit = BitReader_read (&br, 8);
	  if (lit == -1)
	    break;
	  fputc ((BYTE) lit, out);
	  window[win_pos % WINDOW_SIZE] = (BYTE) lit;
	  win_pos++;
	}
      else
	{
	  int dist_enc = BitReader_read (&br, 12);
	  int len_enc = BitReader_read (&br, 8);
	  if (dist_enc == -1 || len_enc == -1)
	    break;
	  DWORD dist = (DWORD) dist_enc + 1;
	  DWORD len = (DWORD) len_enc + 1;
	  DWORD src = (win_pos - dist) % WINDOW_SIZE;
	  for (DWORD i = 0; i < len; i++)
	    {
	      BYTE b = window[(src + i) % WINDOW_SIZE];
	      fputc (b, out);
	      window[(win_pos + i) % WINDOW_SIZE] = b;
	    }
	  win_pos += len;
	}
    }
  fclose (in), fclose (out);
  return EXIT_SUCCESS;
}
