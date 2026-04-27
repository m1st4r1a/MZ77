#ifndef DT_H
#define DT_H 1

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <limits.h>
#include <pthread.h>
#include <gtk/gtk.h>

#define WINDOW_SIZE 4194304
#define MAX_MATCH   256
#define HT_SIZE     65536

typedef uint8_t  BYTE;
typedef uint8_t *PBYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t *PDWORD;
typedef FILE *FD;
typedef char *STR;

enum Mode { MODE_COMPRESS, MODE_DECOMPRESS };
enum TaskStatus { STATUS_PENDING, STATUS_RUNNING, STATUS_DONE, STATUS_ERROR };


typedef struct {
    enum Mode mode;
    enum TaskStatus status;
    union {
        struct { char in_path[PATH_MAX]; char out_path[PATH_MAX]; } paths;
        struct { size_t original; size_t compressed; double ratio; } stats;
        struct { char msg[256]; } error;
    } data;
} TaskItem;

typedef struct TaskNode {
    TaskItem task;
    struct TaskNode *next;
} TaskNode;


typedef struct {
    TaskNode *head, *tail;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    atomic_int count;
    atomic_bool shutdown;
} TaskQueue;
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
  GtkWidget *list_box, *progress, *status, *start_btn, *mode_switch,
    *mode_label;
  GMainLoop *loop;
  atomic_int busy, completed, total;
  TaskQueue queue;
  atomic_bool existing;
} AppData;
typedef struct {
    AppData *app;
    gboolean run;
    char msg[32];
} StateData;


typedef struct HashNode {
    DWORD position;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *buckets[HT_SIZE];
} LZHashTable;

#endif
