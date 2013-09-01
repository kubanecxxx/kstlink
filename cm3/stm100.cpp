#include "stm100.h"
#include "include.h"
#include "stm32f10x.h"
#include "qstlink.h"
#include <QFile>

#define _KEY1    0x45670123
#define _KEY2    0xCDEF89AB

stm100::stm100(QStLink & father, const pages_t & Pages):
    stmAbstract(father,Pages)
{
    QFile file(":/loaders/loaders/stm100/stm100.bin");
    if (!file.open(QFile::ReadOnly))
        ERR("Cannot open loader binary file");

    loader = file.readAll();
    file.close();

    FLASH_CONST.CR = &FLASH->CR;
    FLASH_CONST.SR = &FLASH->SR;
    FLASH_CONST.KEYR = &FLASH->KEYR;
    FLASH_CONST.KEY1 = _KEY1;
    FLASH_CONST.KEY2 = _KEY2;

    FLASH_CONST.SR_BITS.BUSY = FLASH_SR_BSY;

    FLASH_CONST.CR_BITS.MASS_ERASE = FLASH_CR_MER;
    FLASH_CONST.CR_BITS.PAGE_ERASE = FLASH_CR_PER;
    FLASH_CONST.CR_BITS.LOCK = FLASH_CR_LOCK;
    FLASH_CONST.CR_BITS.START = FLASH_CR_STRT;
    FLASH_CONST.CR_BITS.PROG = FLASH_CR_PG;
}

void stm100::ErasePageSetup(int pageNumber)
{
    uint32_t address = pageNumber * pages[0];
    par.WriteRamRegister(&FLASH->AR, address);
}


