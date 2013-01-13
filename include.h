#ifndef INCLUDE_H
#define INCLUDE_H

#include "qlog.h"
#include <inttypes.h>

#define IsBitOn(x,bit)  ((x >> bit) & 1)
#define IsBitOnMask(x,bit)  ((x & bit))
#define BitSet(x,bit)   (x | (1 << bit))
#define BitClear(x,bit) (x & ~(1<<bit))

#define FLASH_BASE 0x08000000
#define SRAM_BASE 0x20000000

#endif // INCLUDE_H
