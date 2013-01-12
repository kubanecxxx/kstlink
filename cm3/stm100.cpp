#include "stm100.h"
#include "qarm_cm3.h"
#include "include.h"
#include "stm32f10x.h"

#define KEY1    0x45670123
#define KEY2    0xCDEF89AB

stm100::stm100(QArm3 & father, int pageSize, int pageCount):
    par(father),
    Size(pageSize),
    Count(pageCount)
{
    IsLocked();
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
