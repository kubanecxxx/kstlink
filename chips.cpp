#include "chips.h"
#include "armConstants.h"
#include "stm100.h"
#include "stm407.h"

#define RAM(x) (x * 1024)
QMap<int,Chips::chip_st> Chips::ChipList;

Chips::Chips(int ID, QStLink & parent):
    stlink(parent)
{
    //fill map
    chip_st temp;
    stm100::pages_t pages;

    /***************************************
     * VALUE Line F100
     */
    /* i don't know ID
    pages.fill(RAM(1),32);
    temp.name = "STM32F100x4, STM32F100x6";
    temp.chipID = STM32_CHIPID_F1_VL_LOW;
    temp.ramSize = RAM(8);
    temp.FlashPages = pages;
    ChipList.insert(temp.chipID,temp);
    pages.clear();
    */

    pages.fill(RAM(1),128);
    temp.name = "STM32F100x8, STM32F100xB";
    temp.chipID = STM32_CHIPID_F1_VL_MEDIUM;
    temp.ramSize = RAM(8);
    temp.FlashPages = pages;
    temp.loader = 100;
    ChipList.insert(temp.chipID,temp);
    pages.clear();

    pages.fill(RAM(2),512);
    temp.name = "STM32F100xC, STM32F100xD";
    temp.chipID = STM32_CHIPID_F1_VL_HIGH;
    temp.ramSize = RAM(24);
    temp.FlashPages = pages;
    temp.loader = 100;
    ChipList.insert(temp.chipID,temp);
    pages.clear();

    /***************************************
     * F100
     */
    pages.fill(RAM(1),32);
    temp.name = "STM32F101x4, STM32F101x6, STM32F102x4, STM32F102x6, STM32F103x4, STM32F103x6";
    temp.chipID = STM32_CHIPID_F1_LOW;
    temp.ramSize = RAM(8);
    temp.FlashPages = pages;
    temp.loader = 100;
    ChipList.insert(temp.chipID,temp);
    pages.clear();

    pages.fill(RAM(1),128);;
    temp.name = "STM32F101x8, STM32F101xB, STM32F102x8, STM32F102xB, STM32F103x8, STM32F103xB";
    temp.chipID = STM32_CHIPID_F1_MEDIUM;
    temp.ramSize = RAM(16);
    temp.FlashPages = pages;
    temp.loader = 100;
    ChipList.insert(temp.chipID,temp);
    pages.clear();

    pages.fill(1024,128);
    temp.name = "STM32F101xC, STM32F101xD, STM32F102xC, STM32F102xD, STM32F103xC, STM32F103xD";
    temp.chipID = STM32_CHIPID_F1_HIGH;
    temp.ramSize = RAM(32);
    temp.FlashPages = pages;
    temp.loader = 100;
    ChipList.insert(temp.chipID,temp);
    pages.clear();

    /***************************************
     * F4xx
     */
    //pages.fill(1024,128);
    pages.push_back(RAM(16));
    pages.push_back(RAM(16));
    pages.push_back(RAM(16));
    pages.push_back(RAM(16));
    pages.push_back(RAM(64));
    pages.push_back(RAM(128));
    pages.push_back(RAM(128));
    pages.push_back(RAM(128));
    pages.push_back(RAM(128));
    pages.push_back(RAM(128));
    pages.push_back(RAM(128));
    pages.push_back(RAM(128));
    temp.name = "STM32F4xxG";
    temp.chipID = STM32_CHIPID_F4;
    temp.ramSize = RAM(128);
    temp.FlashPages = pages;
    temp.loader = 407;
    ChipList.insert(temp.chipID,temp);
    pages.clear();

    if (ChipList.contains(ID))
        ActualChip = ChipList.value(ID);
    else
        qFatal("Unknown CHIP ID");
}

stm100::pages_t Chips::GetFlashPages()
{
    return ActualChip.FlashPages;
}

uint32_t Chips::GetRamSize()
{
    return ActualChip.ramSize;
}

QString Chips::GetChipName()
{
    return ActualChip.name;
}

uint32_t Chips::GetFlashSize()
{
    uint32_t temp = 0;
    for (int i = 0 ; i < ActualChip.FlashPages.count(); i++)
    {
        temp += ActualChip.FlashPages[i];
    }

    return temp;
}

int  Chips::GetLoader()
{
    return ActualChip.loader;
}

stmAbstract * Chips::GetStm()
{
    stmAbstract * stm;
    if (GetLoader() == 100)
        stm = new stm100(stlink,GetFlashPages());
    else if (GetLoader() == 407)
        stm = new stm407(stlink,GetFlashPages());

    return stm;
}

