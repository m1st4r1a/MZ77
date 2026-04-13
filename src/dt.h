#ifndef DT_H
#define DT_H 1
#include<stdio.h>
#include<stdint.h>
#include<stdatomic.h>
#include<limits.h>
#include<gtk/gtk.h>
#include<glib.h>
#include<getopt.h>

#define WINDOW_SIZE 4194304  // 4MB
#define MAX_MATCH 256


typedef uint8_t BYTE;
typedef uint8_t * PBYTE;
typedef uint16_t WORD;
typedef uint16_t * PWORD;
typedef uint32_t DWORD;
typedef uint32_t* PDWORD;
typedef uint64_t QUADWORD;
typedef uint64_t* PQUADWORD;
typedef FILE * FD;
typedef char * STR;

enum Mode
{ COMPRESS, DECOMPRESS, NONE };
typedef struct {
	FILE *fd;
	uint32_t buffer;
	int bits_in_buffer;
} BitWriter;
typedef struct {
    FILE *fp;
    uint32_t buffer;
    int bits;
} BitReader;
typedef struct
{
  GtkWidget *in_label, *out_label, *progress, *status, *run_btn, *mode_switch, *mode_label;
  atomic_int busy, progress_pct;
  char in_path[PATH_MAX], out_path[PATH_MAX];
} AppData;
typedef struct { AppData *app; GtkWindow *win; gboolean is_input; } PickerData;


#endif
