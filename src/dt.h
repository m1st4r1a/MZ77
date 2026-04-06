#ifndef DT_H
#define DT_H 1
#include<stdio.h>
#include<stdint.h>
// data types and constants

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


#endif
