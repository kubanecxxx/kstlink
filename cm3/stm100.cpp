#include "stm100.h"
#include "include.h"
#include "stm32f10x.h"
#include "qstlink.h"
#include <QFile>

#define REG_RAMPOINTER      0
#define REG_FLASHPOINTER    1
#define REG_STATUS          2
#define REG_DATALENGTH      4
#define REG_PC              15
#define SEGMENT_SIZE        0x400

#define _KEY1    0x45670123
#define _KEY2    0xCDEF89AB

stm100::stm100(QStLink & father, const pages_t & Pages):
    par(father),
    pages(Pages)
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

void stm100::WriteFlash(uint32_t start, const QByteArray &data) throw (QString)
{
    par.SysReset();
    par.CoreStop();
    if (IsBusy())
        throw(QString("stm100 WriteFlash memory is busy"));

    QByteArray cpy(data);
    while (cpy.count() % 2)
        cpy.append('\0');

    EraseRange(start,start+data.count(),false);

    FlashUnlock();
    IsLocked();
    par.WriteRamRegister(FLASH_CONST.CR,FLASH_CONST.CR_BITS.PROG);

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
        par.CoreRun();

        //wait for core halted
        int timeout = 0;
        usleep(1000);
        while(!par.IsCoreHalted())
        {
            usleep(5000);
            if(timeout++ == 200)
            {
                throw(QString("stm100 WriteFlash memory timeout"));
            }
        }

        par.ProgrammingProcess((++graph * 100)/graph2);
    }

    par.WriteRamRegister(FLASH_CONST.CR,0);
    FlashLock();
}

bool stm100::IsLocked()
{
    uint32_t word = par.ReadMemoryRegister(FLASH_CONST.CR);

    if (IsBitOnMask(word,FLASH_CONST.CR_BITS.LOCK))
        locked = true;
    else
        locked = false;

    return locked;
}

bool stm100::IsBusy()
{
    uint32_t word = par.ReadMemoryRegister(FLASH_CONST.SR);
    if (IsBitOnMask(word,FLASH_CONST.SR_BITS.BUSY))
        busy = true;
    else
        busy = false;

    return busy;
}

void stm100::EraseRange(uint32_t start, uint32_t stop, bool verify) throw (QString)
{
    uint32_t firstpage = GetPage(start);
    uint32_t lastpage = GetPage(stop);
    uint32_t pagecount;

    pagecount = lastpage - firstpage + 1;

    int graph = pagecount;
    int graph2 = 0;

    while(pagecount--)
    {
        ErasePage(firstpage);

        if (verify)
            if (!VerifyErased(firstpage))
                throw(QString("stm100 EraseRange %1 page verification failed").arg(firstpage));

        firstpage++;
        par.ErasingProgress((++graph2 *100)/graph);
    }
}

void stm100::ErasePageSetup(int pageNumber)
{
    uint32_t address = pageNumber * pages[0];
    par.WriteRamRegister(&FLASH->AR, address);
}

void stm100::ErasePage(int pageNumber) throw (QString)
{
    if (IsBusy())
       throw(QString("stm100 ErasePage - Flash is busy"));

    FlashUnlock();

    uint32_t cr;
    cr = FLASH_CONST.CR_BITS.PAGE_ERASE;
    par.WriteRamRegister(FLASH_CONST.CR,cr);

    ErasePageSetup(pageNumber);

    cr = par.ReadMemoryRegister(FLASH_CONST.CR);
    cr |= FLASH_CONST.CR_BITS.START;
    par.WriteRamRegister(FLASH_CONST.CR,cr);

    while(IsBusy())
        usleep(4000);

    par.WriteRamRegister(FLASH_CONST.CR,0);
    FlashLock();
}

bool stm100::VerifyErased(int PageNum)
{
    QByteArray arr;
    INFO("Verifying flash memory erased");

    uint32_t address;
    uint32_t size = 0;

    if (PageNum == -1)
    {
        address = FLASH_BASE;
        for (int i = 0 ;i < pages.count(); i++)
        {
            size += pages.at(i);
        }
    }
    else
    {
        address = GetBaseAddr(PageNum) + FLASH_BASE;
        size = pages.at(PageNum);
    }

    par.ReadRam(address, size,arr);

    for(int i = 0 ; i < arr.count(); i++)
    {
        if(arr.at(i) != -1)
        {
            return false;
        }
    }

    return true;
}

void stm100::EraseMass() throw (QString)
{
    if (IsBusy())
        throw(QString("stm100 EraseMass - Flash is busy"));

    FlashUnlock();

    uint32_t cr;
    cr = FLASH_CONST.CR_BITS.MASS_ERASE;
    par.WriteRamRegister(FLASH_CONST.CR,cr);

    cr = par.ReadMemoryRegister(FLASH_CONST.CR);
    cr |= FLASH_CONST.CR_BITS.START;
    par.WriteRamRegister(FLASH_CONST.CR,cr);

    while (IsBusy())
        usleep(4000);
}

void stm100::FlashUnlock()
{
    par.WriteRamRegister(FLASH_CONST.KEYR,FLASH_CONST.KEY1);
    par.WriteRamRegister(FLASH_CONST.KEYR,FLASH_CONST.KEY2);
    locked = false;
}

void stm100::FlashLock()
{
    uint32_t word = par.ReadMemoryRegister(FLASH_CONST.CR);
    word |= FLASH_CONST.CR_BITS.LOCK;
    par.WriteRamRegister(FLASH_CONST.CR,word);
    locked = true;
}

int stm100::GetPage(uint32_t addr)
{
    uint32_t low = 0;
    uint32_t high = pages.at(0);
    addr -= FLASH_BASE;

    for (int i = 0 ; i < pages.count() - 1; i++)
    {
        if (addr >= low && addr < high)
        {
            return i;
        }

        low += pages.at(i);
        high += pages.at(i+1);
    }
    ERR("GetPage:address is out of range");
    return -1;
}

uint32_t stm100::GetBaseAddr(int page)
{
    uint32_t addr = 0;
    for (int i= 0; i < page; i++)
    {
        addr += pages.at(i);
    }

    return addr;
}
