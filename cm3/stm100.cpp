#include "stm100.h"
#include "qarm_cm3.h"
#include "include.h"
#include "stm32f10x.h"
#include "qstlink.h"
#include <QFile>
#include <unistd.h>

#define KEY1    0x45670123
#define KEY2    0xCDEF89AB

stm100::stm100(QStLink & father, int pageSize, int pageCount):
    par(father),
    Size(pageSize),
    Count(pageCount)
{
    IsLocked();

    QFile file("neco.bin");
    if (!file.open(QFile::ReadOnly))
        ERR("Cannot open loader binary file");

    loader = file.readAll();
    file.close();
}

#define REG_RAMPOINTER      0
#define REG_FLASHPOINTER    1
#define REG_STATUS          2
#define REG_DATALENGTH      4
#define REG_PC              15
#define SEGMENT_SIZE        0x400

bool stm100::WriteFlash(uint32_t start, const QByteArray &data)
{
    par.CoreStop();
    if (IsBusy())
        return false;

    QByteArray cpy(data);
    while (cpy.count() % 2)
        cpy.append('\0');

    bool erased =EraseRange(start,start+data.count(),false);
    if (!erased)
        return false;

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
                WARN("Flash writing timeout");
                return false;
            }
        }
    }

    par.WriteRamRegister(&FLASH->CR,0);
    FlashLock();

    return true;
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

bool stm100::EraseRange(uint32_t start, uint32_t stop, bool verify)
{
    uint32_t firstpage = (start - FLASH_BASE) / this->Size;
    uint32_t pagecount = (stop - start) / this->Size + 1;

    while(pagecount--)
    {
        if (!ErasePage(firstpage))
            return false;

        if (verify)
            if (!VerifyErased(firstpage))
                return false;

        firstpage++;
    }

    return true;
}

bool stm100::ErasePage(int pageNumber)
{
    FlashUnlock();
    if (IsBusy())
    {
        FlashLock();
        return false;
    }

    uint32_t cr;
    cr = FLASH_CR_PER;
    par.WriteRamRegister(&FLASH->CR,cr);

    uint32_t address = pageNumber * this->Size;
    par.WriteRamRegister(&FLASH->AR, address);

    cr = par.ReadMemoryRegister(&FLASH->CR);
    cr |= FLASH_CR_STRT;
    par.WriteRamRegister(&FLASH->CR,cr);

    while(IsBusy());

    par.WriteRamRegister(&FLASH->CR,0);
    FlashLock();

    return true;
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

bool stm100::EraseMass()
{
    FlashUnlock();
    if (IsBusy())
    {
        FlashLock();
        return false;
    }

    uint32_t cr;
    cr = FLASH_CR_MER;
    par.WriteRamRegister(&FLASH->CR,cr);

    cr = par.ReadMemoryRegister(&FLASH->CR);
    cr |= FLASH_CR_STRT;
    par.WriteRamRegister(&FLASH->CR,cr);

    while (IsBusy());

    cr = par.ReadMemoryRegister(&FLASH->SR);

    return true;
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
