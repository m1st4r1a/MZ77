#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include<errno.h>
#include "dt.h"
#include "BitWriter.h"

static DWORD
find_longest_match (PBYTE window, DWORD win_pos, DWORD win_len,
		    PBYTE lookahead, DWORD lookahead_len,
		    PDWORD out_dist, PDWORD out_len)
{
  DWORD best_len = 0, best_dist = 0, max_search = win_len;
  for (DWORD i = 0; i < max_search; i++)
    {
      DWORD pos = (win_pos - max_search + i) % WINDOW_SIZE;
      DWORD len = 0;
      while (len < lookahead_len && len < MAX_MATCH &&
	     window[(pos + len) % WINDOW_SIZE] == lookahead[len])
	len++;
      if (len > best_len)
	{
	  best_len = len;
	  best_dist = max_search - i;
	}
    }
  *out_dist = best_dist;
  *out_len = best_len;
  return best_len >= 3;
}

DWORD
compress (STR input_path, STR output_path)
{
  FD input_fd = fopen (input_path, "rb");
  if (!input_fd)
    return ENOENT;
  FD output_fd = fopen (output_path, "wb");
  static BYTE window[WINDOW_SIZE];
  (void) memset (window, 0, WINDOW_SIZE);
  BitWriter bw = { 0 };
  BitWriter_init (&bw, output_fd);
  DWORD window_filled = 0;
  BYTE LookAhead[256];
  DWORD LookAheadLength = fread (LookAhead, 1, sizeof LookAhead, input_fd);
  while (LookAheadLength > 0)
    {
      DWORD distance = 0, length = 0;
      bool is_match =
	find_longest_match (window, window_filled, window_filled, LookAhead,
			    LookAheadLength, &distance, &length);
      if (is_match)
	{
	  BitWriter_write (&bw, 1, 1);	// Match flag
	  BitWriter_write (&bw, distance - 1, 12);	// Distance
	  BitWriter_write (&bw, length - 1, 8);	// Length
	}
      else
	{
	  BitWriter_write (&bw, 0, 1);
	  BitWriter_write (&bw, LookAhead[0], 8);
	  length = 1;
	}
      for (DWORD i = 0; i < length; i++)
	window[(window_filled + i) % WINDOW_SIZE] = LookAhead[i];
      window_filled = (window_filled + length) % WINDOW_SIZE;
      memmove (LookAhead, LookAhead + length, LookAheadLength - length);
      LookAheadLength -= length;
      if (LookAheadLength < sizeof (LookAhead))
	{
	  DWORD read = fread (LookAhead + LookAheadLength, 1,
			      sizeof (LookAhead) - LookAheadLength, input_fd);
	  LookAheadLength += read;
	}
    }
  BitWriter_finish (&bw);
  fclose (input_fd);
  fclose (output_fd);
  return EXIT_SUCCESS;
}
