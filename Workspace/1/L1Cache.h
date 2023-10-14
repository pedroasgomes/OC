#ifndef L1CACHE_H
#define L1CACHE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../Cache.h"

#define OFFSET_BITS (int)(log2(BLOCK_SIZE))
#define L1_INDEX_BITS (int)(log2(L1_SIZE / BLOCK_SIZE))
#define L1_TAG_BITS (32 - L1_INDEX_BITS - OFFSET_BITS)

#define MASK_OFFSET  ((1U << OFFSET_BITS) - 1)
#define MASK_L1INDEX (((1U << L1_INDEX_BITS) - 1) << OFFSET_BITS)
#define MASK_L1TAG   (~(MASK_L1INDEX | MASK_OFFSET))

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
} CacheLine;

typedef struct Cache {
  uint32_t init;
  CacheLine L1Lines[L1_SIZE / BLOCK_SIZE];
} Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
