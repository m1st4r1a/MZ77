#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "dt.h"
#include "hashtable.h"
#include "BitWriter.h"

static inline uint32_t
hash_bytes (const BYTE *p)
{
  return ((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8) | p[2];
}

static bool
find_match (LZHashTable *ht, PBYTE window, DWORD win_pos,
	    PBYTE lookahead, DWORD lookahead_len,
	    PDWORD out_dist, PDWORD out_len)
{
  if (lookahead_len < 3)
    return false;
  uint32_t hash = hash_bytes (lookahead);
  HashNode *node = ht->buckets[hash % HT_SIZE];
  DWORD best_len = 0, best_dist = 0;
  while (node)
    {
      DWORD pos = node->position;
      DWORD len = 0;
      while (len < lookahead_len && len < MAX_MATCH &&
	     window[(pos + len) % WINDOW_SIZE] == lookahead[len])
	len++;
      if (len > best_len)
	{
	  best_len = len;
	  best_dist = win_pos - pos;
	}
      node = node->next;
    }

  *out_len = best_len;
  *out_dist = best_dist;
  return best_len >= 3;
}

DWORD
compress (STR input_path, STR output_path)
{
  /* Fixed: Removed trailing spaces in mode strings */
  FD in = fopen (input_path, "rb");
  if (!in)
    return ENOENT;
  FD out = fopen (output_path, "wb");
  if (!out)
    {
      fclose (in);
      return ENOENT;
    }

  /* Use static to prevent stack overflow */
  static BYTE window[WINDOW_SIZE] = { 0 };
  LZHashTable ht;
  HashTable_init (&ht);

  BitWriter bw;
  BitWriter_init (&bw, out);

  DWORD win_pos = 0;
  BYTE lookahead[256];
  /* Read initial buffer */
  DWORD la_len = fread (lookahead, 1, sizeof (lookahead), in);

  while (la_len > 0)
    {
      DWORD dist = 0, len = 0;
      bool is_match =
	find_match (&ht, window, win_pos, lookahead, la_len, &dist, &len);

      if (is_match)
	{
	  BitWriter_write (&bw, 1, 1);
	  BitWriter_write (&bw, dist - 1, 12);
	  BitWriter_write (&bw, len - 1, 8);
	}
      else
	{
	  BitWriter_write (&bw, 0, 1);
	  BitWriter_write (&bw, lookahead[0], 8);
	  len = 1;
	}

      /* 1. Update Sliding Window */
      /* 2. Update Hash Table (Only if enough bytes remain to form a 3-byte hash) */
      for (DWORD i = 0; i < len; i++)
	{
	  window[(win_pos + i) % WINDOW_SIZE] = lookahead[i];

	  /* SAFE BOUNDS CHECK: Ensure lookahead has at least 3 bytes starting from i */
	  if (la_len > i + 2)
	    {
	      HashTable_insert (&ht, hash_bytes (&lookahead[i]), win_pos + i);
	    }
	}
      win_pos = (win_pos + len) % WINDOW_SIZE;

      /* Shift lookahead buffer */
      memmove (lookahead, lookahead + len, la_len - len);
      la_len -= len;

      /* Refill lookahead */
      if (la_len < sizeof (lookahead))
	{
	  DWORD rd =
	    fread (lookahead + la_len, 1, sizeof (lookahead) - la_len, in);
	  la_len += rd;
	}
    }
  BitWriter_finish (&bw);
  HashTable_clear (&ht);	/* Free hash table nodes */
  fclose (in);
  fclose (out);
  return EXIT_SUCCESS;
}
