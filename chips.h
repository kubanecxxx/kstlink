#ifndef CHIPS_H
#define CHIPS_H

#include <QMap>
#include <QString>
#include "stm100.h"

class Chips
{
public:
    Chips(int ID);

    typedef struct
    {
        QString name;
        int chipID;
        uint32_t ramSize;
        stm100::pages_t FlashPages;
        int loader;
    } chip_st;

    stm100::pages_t GetFlashPages();
    uint32_t GetRamSize();
    QString GetChipName();
    int GetLoader();
    uint32_t GetFlashSize();

private:
    static QMap<int,chip_st> ChipList;
    static int count;
    chip_st ActualChip;

};

#endif // CHIPS_H
