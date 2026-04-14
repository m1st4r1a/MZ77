#ifndef HASHTABLE_H
#define HASHTABLE_H 1
#include "dt.h"
void HashTable_init(LZHashTable *ht);
void HashTable_insert(LZHashTable *ht, uint32_t hash, DWORD pos);
void HashTable_clear(LZHashTable *ht);

#endif
