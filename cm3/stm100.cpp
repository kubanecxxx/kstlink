#include "stm100.h"
#include "include.h"
#include "stm32f10x.h"
#include "qstlink.h"
#include <QFile>
#include <unistd.h>

#define REG_RAMPOINTER      0
#define REG_FLASHPOINTER    1
#define REG_STATUS          2
#define REG_DATALENGTH      4
#define REG_PC              15
#define SEGMENT_SIZE        0x400

stm100::stm100(QStLink & father, const pages_t & Pages):
    par(father),
    pages(Pages),
    Size(pages[0]),
    Count(pages.count())
{
    IsLocked();

    QFile file("neco.bin");
    if (!file.open(QFile::ReadOnly))
        ERR("Cannot open loader binary file");

    loader = file.readAll();
    file.close();

    for (int i = 0 ; i < 16 ; i++)
    {
        reg_t temp;
        temp.val = 0;
        temp.Reg = QString("r%1").arg(i);
        registers.push_back(temp);
    }
    reg_t temp;
    temp.val = 0;
    temp.Reg = QString("xpsr");
    registers.push_back(temp);
    temp.Reg = QString("main_sp");
    registers.push_back(temp);
    temp.Reg = QString("process_sp");
    registers.push_back(temp);
    temp.Reg = QString("primask..");
    registers.push_back(temp);
    temp.Reg = QString("fpscr");
    registers.push_back(temp);
}
void stm100::ReadAllRegisters(uint32_t * rawData)
{
    ReadAllRegisters();

    for (int i = 0 ; i < registers.count(); i++)
        rawData[i] = registers[i].val;
}

const stm100::regs_t & stm100::ReadAllRegisters()
{
    uint32_t temp[registers.count()];
    par.ReadAllRegisters(temp,registers.count() * 4);

    for (int i = 0 ; i < registers.count(); i++)
    {
        registers[i].val = temp[i];
    }

    memcpy(&regs_human, temp,  sizeof(cm3_regs_t));

    return registers;
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
    par.WriteRamRegister(&FLASH->CR,FLASH_CR_PG);

    //load loader
    par.WriteRam(SRAM_BASE, loader);
    par.WriteRegister(REG_FLASHPOINTER,start);
    uint32_t flash_sr = (uint64_t) &FLASH->SR;
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

    par.WriteRamRegister(&FLASH->CR,0);
    FlashLock();
}

bool stm100::IsLocked()
{
    uint32_t word = par.ReadMemoryRegister(&FLASH->CR);

    if (IsBitOnMask(word,FLASH_CR_LOCK))
        locked = true;
    else
        locked = false;

    return locked;
}

bool stm100::IsBusy()
{
    uint32_t word = par.ReadMemoryRegister(&FLASH->SR);
    if (IsBitOnMask(word,FLASH_SR_BSY))
        busy = true;
    else
        busy = false;

    return busy;
}

void stm100::EraseRange(uint32_t start, uint32_t stop, bool verify) throw (QString)
{
    uint32_t firstpage = (start - FLASH_BASE) / this->Size;
    uint32_t pagecount = (stop - start) / this->Size + 1;
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

void stm100::ErasePage(int pageNumber) throw (QString)
{
    if (IsBusy())
       throw(QString("stm100 ErasePage - Flash is busy"));

    FlashUnlock();

    uint32_t cr;
    cr = FLASH_CR_PER;
    par.WriteRamRegister(&FLASH->CR,cr);

    uint32_t address = pageNumber * this->Size;
    par.WriteRamRegister(&FLASH->AR, address);

    cr = par.ReadMemoryRegister(&FLASH->CR);
    cr |= FLASH_CR_STRT;
    par.WriteRamRegister(&FLASH->CR,cr);

    while(IsBusy())
        usleep(4000);

    par.WriteRamRegister(&FLASH->CR,0);
    FlashLock();
}

bool stm100::VerifyErased(int PageNum)
{
    QByteArray arr;
    INFO("Verifying flash memory erased");

    uint32_t address;
    uint32_t size;

    if (PageNum == -1)
    {
        address = FLASH_BASE;
        size = this->Size * Count;
    }
    else
    {
        address = PageNum * this->Size + FLASH_BASE;
        size = this->Size;
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
    cr = FLASH_CR_MER;
    par.WriteRamRegister(&FLASH->CR,cr);

    cr = par.ReadMemoryRegister(&FLASH->CR);
    cr |= FLASH_CR_STRT;
    par.WriteRamRegister(&FLASH->CR,cr);

    while (IsBusy())
        usleep(4000);

    cr = par.ReadMemoryRegister(&FLASH->SR);
}

void stm100::FlashUnlock()
{
    uint32_t word = par.ReadMemoryRegister(&FLASH->CR);
    if (IsBitOnMask(word,FLASH_CR_LOCK))
    {
        par.WriteRamRegister(&FLASH->KEYR,KEY1);
        par.WriteRamRegister(&FLASH->KEYR,KEY2);
        locked = false;
    }
}

void stm100::FlashLock()
{
    uint32_t temp = par.ReadMemoryRegister(&FLASH->CR);
    if (IsBitOnMask(temp,FLASH_CR_LOCK))
        return;

    uint32_t word = par.ReadMemoryRegister(&FLASH->CR);
    word |= FLASH_CR_LOCK;
    par.WriteRamRegister(&FLASH->CR,word);
    locked = true;
}
