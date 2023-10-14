#include "L2Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache mainCache;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/

void initCache() { 
  for (int i = 0; i < (L2_SIZE / BLOCK_SIZE); i++) {
    if (i < (L1_SIZE/ BLOCK_SIZE)){
      mainCache.L1Lines[i].Valid = 0; 
    }
    mainCache.L2Lines[i].Valid = 0; 
    }
  }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t tag, index, offset, newMemAddress, oldMemAddress;
  CacheLine *targetLine;
  
  // [18-tag][8-index][6-offset]
  offset = address & MASK_OFFSET; // Removes the tag + index 

  index = address & MASK_L1INDEX; // Removes the tag + offset 
  index = index >> OFFSET_BITS; // Fix Displacement

  tag = address >> (L1_INDEX_BITS + OFFSET_BITS); // Removes index + offset

  newMemAddress = address & ~(MASK_OFFSET); // Removes the offset

  targetLine = &mainCache.L1Lines[index]; // Set pointer to target line

  // miss or invalid
  if ((!targetLine->Valid) || (targetLine->Tag != tag)) { 

    // Case newTag != oldTag
    if ((targetLine->Valid) && (targetLine->Dirty)) { 
      // Removes tag
      oldMemAddress = newMemAddress & ~(MASK_L1TAG);
      // Bitwise OR between [0][new_index][new_offset], [old_tag][0][0]
      oldMemAddress |= (targetLine->Tag << (L1_INDEX_BITS + OFFSET_BITS));
      // Writes back whole old CacheLine to L2
      accessL2(oldMemAddress, &(L1Cache[index * BLOCK_SIZE]), MODE_WRITE); 
    }

    // Copies the block starting at newMemAddress into the L1
    accessL2(newMemAddress, &(L1Cache[index * BLOCK_SIZE]), MODE_READ); 

    targetLine->Tag = tag; // Updates the tag
    targetLine->Dirty = 0; // L1Cache is in sync with L2Cache
    targetLine->Valid = 1; // Cache Line is valid

  } // if miss, then replaced with the currect block and write back if dirty

  // Line in Cache
  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L1Cache[index * BLOCK_SIZE + offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L1Cache[index * BLOCK_SIZE + offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    targetLine->Dirty = 1; // L1Cache is not in sync with L2Cache 
  }
}

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t tag, index, offset, newMemAddress, oldMemAddress;
  CacheLine *targetLine;
  
  // [17-tag][9-index][6-offset]
  offset = address & MASK_OFFSET; // Removes the tag + index 

  index = address & MASK_L2INDEX; // Removes the tag + offset
  index = index >> OFFSET_BITS; // Fix Displacement

  tag = address >> (L2_INDEX_BITS + OFFSET_BITS); // Removes index + offset

  newMemAddress = address & (~MASK_OFFSET); // Removes the offset

  targetLine = &mainCache.L2Lines[index]; // Set pointer to target line

  // miss or invalid
  if ((!targetLine->Valid) || (targetLine->Tag != tag)) { 

    // Case newTag != oldTag
    if ((targetLine->Valid) && (targetLine->Dirty)) { 
      // Removes tag
      oldMemAddress = newMemAddress & ~(MASK_L2TAG);
      // Bitwise OR between: [0][new_index][new_offset] and [old_tag][0][0]
      oldMemAddress |= (targetLine->Tag << (L2_INDEX_BITS + OFFSET_BITS));
      // Writes back whole old CacheLine to DRAM
      accessDRAM(oldMemAddress, &(L2Cache[index * BLOCK_SIZE]), MODE_WRITE); 
    }

    // Copies the block starting at newMemAddress into the L2
    accessDRAM(newMemAddress, &(L2Cache[index * BLOCK_SIZE]), MODE_READ); 

    targetLine->Tag = tag; // Updates the tag
    targetLine->Dirty = 0; // L2Cache is in sync with DRAM 
    targetLine->Valid = 1; // Cache Line is valid

  } // if miss, then replaced with the currect block and write back if dirty

  // Line in Cache
  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L2Cache[index * BLOCK_SIZE + offset]), WORD_SIZE);
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L2Cache[index * BLOCK_SIZE + offset]), data, WORD_SIZE);
    time += L2_WRITE_TIME;
    targetLine->Dirty = 1; // L2Cache is not in sync with DRAM 
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
