#include "stm407.h"
#include "stm32f4xx.h"
#include <QFile>
#include "include.h"
#include "qstlink.h"

#define _KEY1    0x45670123
#define _KEY2    0xcdef89ab

#define REG_RAMPOINTER      0
#define REG_FLASHPOINTER    1
#define REG_STATUS          2
#define REG_DATALENGTH      4
#define REG_PC              15
#define SEGMENT_SIZE        0x400

stm407::stm407(QStLink & par, const pages_t & Pages):
    stm100(par,Pages)
{
    IsLocked();
    QFile file(":/loaders/loaders/stm407/stm407.bin");
    if (!file.open(QFile::ReadOnly))
        ERR("Cannot open loader binary file");

    loader = file.readAll();
    file.close();

    FLASH_CONST.KEYR = &FLASH->KEYR;
    FLASH_CONST.SR = &FLASH->SR;
    FLASH_CONST.CR = &FLASH->CR;
    FLASH_CONST.KEY1 = _KEY1;
    FLASH_CONST.KEY2 = _KEY2;

    FLASH_CONST.SR_BITS.BUSY = FLASH_SR_BSY;

    FLASH_CONST.CR_BITS.LOCK = FLASH_CR_LOCK;
    FLASH_CONST.CR_BITS.MASS_ERASE = FLASH_CR_MER;
    FLASH_CONST.CR_BITS.PAGE_ERASE = FLASH_CR_SER;
    FLASH_CONST.CR_BITS.START = FLASH_CR_STRT;
    FLASH_CONST.CR_BITS.PROG = FLASH_CR_PG | FLASH_CR_PSIZE_1;
}

#if 0
void stm407::WriteFlash(uint32_t start , const QByteArray & data) throw (QString)
{
    par.SysReset();
    par.CoreStop();
    if (IsBusy())
        throw(QString("stm407 WriteFlash memory is busy"));

    QByteArray cpy(data);
    while (cpy.count() % 4)
        cpy.append('\0');

    try{
    EraseRange(start,start+cpy.count(),true);
    } catch (QString data)
    {
        ERR(data);
    }

    FlashUnlock();
    par.WriteRamRegister(&FLASH->CR,FLASH_CR_PG | FLASH_CR_PSIZE_1);

    //load loader
    par.WriteRam(SRAM_BASE, loader);
    par.WriteRegister(REG_FLASHPOINTER,start);
    uint32_t flash_sr = (uint64_t) FLASH_CONST.SR;
    par.WriteRegister(REG_STATUS, flash_sr);

    /*
     * divide into 512byte segments
     */
    int segments = (data.count() + SEGMENT_SIZE - 1) / SEGMENT_SIZE;

    int graph = 0;
    int graph2 = segments;

    for (int i = 0 ; i < segments; i++ )
    {
        QByteArray seg = cpy.left(SEGMENT_SIZE);
        cpy.remove(0,SEGMENT_SIZE);
        //writeram segment
        uint32_t ramAddr = SRAM_BASE + 0x200;
        par.WriteRam(ramAddr, seg);
        //setuploader
        par.WriteRegister(REG_RAMPOINTER, ramAddr);
        par.WriteRegister(REG_DATALENGTH,ramAddr + seg.count());
        par.WriteRegister(REG_PC,SRAM_BASE);

        //force thumb mode
        uint32_t xpsr = par.ReadRegister(16);
        xpsr |= 1<<24;
        par.WriteRegister(16,xpsr);
        //run flashloader
/*
        while(1)
        {
            par.ReadAllRegisters(NULL);
            par.CoreSingleStep();
        }
*/
                par.CoreRun();

        //wait for core halted
        int timeout = 0;
        usleep(1000);
        while(!par.IsCoreHalted())
        {
            usleep(5000);
            if(timeout++ == 200)
            {
                throw(QString("stm407 WriteFlash memory timeout"));
            }
        }

        par.ProgrammingProcess((++graph * 100)/graph2);
    }

    par.WriteRamRegister(&FLASH->CR,0);
    FlashLock();

#if 0
    for (int i = 0; i < data.count(); i++)
    {
        QByteArray buf = data.mid(4 * i , 4);
        par.WriteRam(start+=4,buf);
        while(IsBusy())
            usleep(10);

        QByteArray ver;
        par.ReadRam(start -4,4,ver);

        asm("nop");
    }

    par.WriteRamRegister(&FLASH->CR,0);
    FlashLock();
#endif
}
#endif

void stm407::ErasePageSetup(int PageNumber)
{
    uint32_t cr = par.ReadMemoryRegister(FLASH_CONST.CR);
    cr |= (PageNumber << 3) ;
    par.WriteRamRegister(&FLASH->CR,cr);
}
