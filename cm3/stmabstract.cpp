#include "stmabstract.h"
#include "qstlink.h"

#define REG_RAMPOINTER      0
#define REG_FLASHPOINTER    1
#define REG_STATUS          2
#define REG_DATALENGTH      4
#define REG_PC              15
#define SEGMENT_SIZE        0x400

stmAbstract::stmAbstract(QStLink & father,const pages_t & _pages):
    par(father),
    pages(_pages)
{
}

void stmAbstract::WriteFlash(uint32_t start, const QByteArray &data) throw (QString)
{
#if 1
    par.SysReset();
    par.CoreStop();
    if (IsBusy())
        throw(QString("stmAbstract WriteFlash memory is busy"));

    QByteArray cpy(data);
    while (cpy.count() % 4)
        cpy.append('\0');

    EraseRange(start,start+cpy.count(),false);

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

    int segments = (cpy.count() + SEGMENT_SIZE - 1) / SEGMENT_SIZE;

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
                throw(QString("stmAbstract WriteFlash memory timeout"));
            }
        }

        par.ProgrammingProcess((++graph * 100)/graph2);
    }

    par.WriteRamRegister(FLASH_CONST.CR,0);
    FlashLock();
#endif
}

bool stmAbstract::IsLocked()
{
    uint32_t word = par.ReadMemoryRegister(FLASH_CONST.CR);

    if (IsBitOnMask(word,FLASH_CONST.CR_BITS.LOCK))
        locked = true;
    else
        locked = false;

    return locked;
}

bool stmAbstract::IsBusy()
{
    uint32_t word = par.ReadMemoryRegister(FLASH_CONST.SR);
    if (IsBitOnMask(word,FLASH_CONST.SR_BITS.BUSY))
        busy = true;
    else
        busy = false;

    return busy;
}

void stmAbstract::EraseRange(uint32_t start, uint32_t stop, bool verify) throw (QString)
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
                throw(QString("stmAbstract EraseRange %1 page verification failed").arg(firstpage));

        firstpage++;
        par.ErasingProgress((++graph2 *100)/graph);
    }
}

void stmAbstract::ErasePage(int pageNumber) throw (QString)
{
    if (IsBusy())
       throw(QString("stmAbstract ErasePage - Flash is busy"));

    FlashUnlock();

    IsLocked();
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

bool stmAbstract::VerifyErased(int PageNum)
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

void stmAbstract::EraseMass() throw (QString)
{
    if (IsBusy())
        throw(QString("stmAbstract EraseMass - Flash is busy"));

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

void stmAbstract::FlashUnlock() throw (QString)
{
    par.WriteRamRegister(FLASH_CONST.KEYR,FLASH_CONST.KEY1);
    par.WriteRamRegister(FLASH_CONST.KEYR,FLASH_CONST.KEY2);

    if (IsLocked())
        throw (QString("Cannot unlock flash for writing"));
}

void stmAbstract::FlashLock()
{
    uint32_t word = par.ReadMemoryRegister(FLASH_CONST.CR);
    word |= FLASH_CONST.CR_BITS.LOCK;
    par.WriteRamRegister(FLASH_CONST.CR,word);
    locked = true;
}

int stmAbstract::GetPage(uint32_t addr)
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

uint32_t stmAbstract::GetBaseAddr(int page)
{
    uint32_t addr = 0;
    for (int i= 0; i < page; i++)
    {
        addr += pages.at(i);
    }

    return addr;
}
