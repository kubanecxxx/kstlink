#include <QCoreApplication>
#include <qarm_cm3.h>
//#include "armConstants.h"

unsigned int log_level = 3;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

/*
    QArm3::Debug_t deb;

    deb.FLASH_BASE = STM32_FLASH_BASE;
    deb.RAM_BASE = STM32_SRAM_BASE;
    deb.REG_CPUID = CM3_REG_CPUID;
    deb.REG_FP_CTRL = CM3_REG_FP_CTRL;
    deb.REG_COMP_BASE = CM3_REG_FP_COMP0;
    deb.COMP_REG_COUNT = 8;
    deb.DHCSR = _DHCSR;
    deb.DCRDR = _DCRDR;
    deb.DCRSR = _DCRSR;
    deb.DBGKEY = _DBGKEY;
*/

    new QArm3(&a);

    return a.exec();
}
