#ifndef ARMCONSTANTS_H
#define ARMCONSTANTS_H

#include <inttypes.h>

/* cortex core ids */
/*
#define STM32VL_CORE_ID 0x1ba01477
#define STM32L_CORE_ID 0x2ba01477
#define STM32F3_CORE_ID 0x2ba01477
#define STM32F4_CORE_ID 0x2ba01477
#define STM32F0_CORE_ID 0xbb11477
#define CORE_M3_R1 0x1BA00477
#define CORE_M3_R2 0x4BA00477
*/
#define CORE_M4_R0 0x2BA01477

/*
* Chip IDs are explained in the appropriate programming manual for the
* DBGMCU_IDCODE register (0xE0042000)
*/
// stm32 chipids, only lower 12 bits..
#define STM32_CHIPID_F1_MEDIUM 0x410
#define STM32_CHIPID_F2 0x411
#define STM32_CHIPID_F1_LOW 0x412
#define STM32_CHIPID_F3 0x422
#define STM32_CHIPID_F37x 0x432
#define STM32_CHIPID_F4 0x413
#define STM32_CHIPID_F1_HIGH 0x414
#define STM32_CHIPID_L1_MEDIUM 0x416
#define STM32_CHIPID_F1_CONN 0x418
#define STM32_CHIPID_F1_VL_MEDIUM 0x420
#define STM32_CHIPID_F1_VL_HIGH 0x428
#define STM32_CHIPID_F1_XL 0x430
#define STM32_CHIPID_F0 0x440


/* Cortex™-M3 Technical Reference Manual */
/* Debug Halting Control and Status Register */
//#define _DHCSR 0xe000edf0
//#define _DCRSR 0xe000edf4
//#define _DCRDR 0xe000edf8
//#define _DBGKEY 0xa05f0000


// cortex m3 technical reference manual
//#define REG_CORE_ID 0xE000ED00
#define REG_CPUID 0xE0042000
#define DBGMCU_CR 0xe0042004
//#define CM3_REG_FP_CTRL 0xE0002000
//#define CM3_REG_FP_COMP0 0xE0002008

#endif // ARMCONSTANTS_H
