#include "lz77.h"
#include "bitio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define WINDOW_SIZE 4096
#define MAX_MATCH   258
#define MIN_MATCH   3
#define HT_SIZE     65536

static inline uint16_t
hash3 (const uint8_t *p)
{
  return ((uint16_t) p[0] << 10) ^ (p[1] << 4) ^ p[2];
}

int
lz77_compress (const char *in_path, const char *out_path)
{
  FILE *fin = fopen (in_path, "rb");
  if (!fin)
    return -1;
  FILE *fout = fopen (out_path, "wb");
  if (!fout)
    {
      fclose (fin);
      return -1;
    }
  BitWriter bw;
  bw_init (&bw, fout);
  uint8_t window[WINDOW_SIZE] = { 0 };
  uint32_t ht_heads[HT_SIZE];
  uint32_t ht_next[WINDOW_SIZE];
  memset (ht_heads, 0xFF, sizeof (ht_heads));
  uint8_t buf[WINDOW_SIZE];
  size_t buf_len = fread (buf, 1, sizeof (buf), fin);
  size_t win_pos = 0;
  while (buf_len > 0)
    {
      size_t best_len = 0;
      size_t best_dist = 0;
      if (buf_len >= MIN_MATCH)
	{
	  uint16_t h = hash3 (buf);
	  uint32_t pos = ht_heads[h];
	  while (pos != 0xFFFFFFFF)
	    {
	      if (window[pos % WINDOW_SIZE] == buf[0] &&
		  window[(pos + 1) % WINDOW_SIZE] == buf[1] &&
		  window[(pos + 2) % WINDOW_SIZE] == buf[2])
		{
		  size_t len = 3;
		  while (len < MAX_MATCH && len < buf_len &&
			 window[(pos + len) % WINDOW_SIZE] == buf[len])
		    len++;
		  if (len > best_len)
		    {
		      best_len = len;
		      best_dist = win_pos - pos;
		    }
		}
	      pos = ht_next[pos % WINDOW_SIZE];
	    }
	}
      if (best_len >= MIN_MATCH)
	{
	  bw_write (&bw, 1, 1);
	  bw_write (&bw, (uint32_t) (best_dist - 1), 12);
	  bw_write (&bw, (uint32_t) (best_len - 3), 8);
	  for (size_t i = 0; i < best_len; i++)
	    {
	      window[win_pos % WINDOW_SIZE] = buf[i];
	      if (i + 2 < best_len)
		{
		  uint16_t h = hash3 (&buf[i]);
		  ht_next[win_pos % WINDOW_SIZE] = ht_heads[h];
		  ht_heads[h] = win_pos;
		}
	      win_pos++;
	    }
	  memmove (buf, buf + best_len, buf_len - best_len);
	  buf_len -= best_len;
	}
      else
	{
	  bw_write (&bw, 0, 1);
	  bw_write (&bw, buf[0], 8);
	  window[win_pos % WINDOW_SIZE] = buf[0];
	  if (buf_len >= 3)
	    {
	      uint16_t h = hash3 (buf);
	      ht_next[win_pos % WINDOW_SIZE] = ht_heads[h];
	      ht_heads[h] = win_pos;
	    }
	  win_pos++;
	  memmove (buf, buf + 1, buf_len - 1);
	  buf_len--;
	}
      if (buf_len < sizeof (buf) - 3)
	{
	  size_t rd = fread (buf + buf_len, 1, sizeof (buf) - buf_len, fin);
	  buf_len += rd;
	}
    }
  bw_finish (&bw);
  fclose (fin);
  fclose (fout);
  return 0;
}

int
lz77_decompress (const char *in_path, const char *out_path)
{
  FILE *fin = fopen (in_path, "rb");
  if (!fin)
    return -1;
  FILE *fout = fopen (out_path, "wb");
  if (!fout)
    {
      fclose (fin);
      return -1;
    }
  BitReader br;
  br_init (&br, fin);
  uint8_t window[WINDOW_SIZE] = { 0 };
  size_t win_pos = 0;
  while (1)
    {
      int flag = br_read (&br, 1);
      if (flag == -1)
	break;
      if (flag == 0)
	{
	  int lit = br_read (&br, 8);
	  if (lit == -1)
	    break;
	  fputc ((uint8_t) lit, fout);
	  window[win_pos % WINDOW_SIZE] = (uint8_t) lit;
	  win_pos++;
	}
      else
	{
	  int d_enc = br_read (&br, 12);
	  int l_enc = br_read (&br, 8);
	  if (d_enc == -1 || l_enc == -1)
	    break;
	  uint32_t dist = (uint32_t) d_enc + 1;
	  uint32_t len = (uint32_t) l_enc + 3;
	  uint32_t src = (win_pos + WINDOW_SIZE - dist) % WINDOW_SIZE;
	  for (uint32_t i = 0; i < len; i++)
	    {
	      uint8_t b = window[(src + i) % WINDOW_SIZE];
	      fputc (b, fout);
	      window[(win_pos + i) % WINDOW_SIZE] = b;
	    }
	  win_pos += len;
	}
    }
  fclose (fin);
  fclose (fout);
  return 0;
}
