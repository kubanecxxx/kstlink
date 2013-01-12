#include "armConstants.h"

const REGS_t REGS =
{
    .FP_CTRL = CM3_REG_FP_CTRL,
    .FP_COMP0 = CM3_REG_FP_COMP0,
    .CPUID = REG_CPUID,
    .DHCSR = _DHCSR,
    .DCRSR = _DCRSR,
    .DCRDR = _DCRDR,
    .DBGKEY = _DBGKEY
};
