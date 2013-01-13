#ifndef QARMABSTRACT_H
#define QARMABSTRACT_H

#include <QObject>
#include <inttypes.h>
#include "qstlink.h"
#include "armConstants.h"

/**
 * @brief prescribes interface
 */
class QArmAbstract: public QStLink
{
    Q_OBJECT
public:
    typedef struct
    {
        int Number;
        uint32_t PageSize;
    } flash_page_t;

    typedef struct
    {
        uint32_t chipID;
        QVector<flash_page_t> * FlashPages;
        uint32_t FlashSize;
        uint32_t RamSize;
        QByteArray * Loader;
    } chip_properties_t;

    QArmAbstract(QObject * parent, const chip_properties_t & chip):
        QStLink(parent),
        Chip(chip)
    {
        chipID = ReadMemoryWord(REGS.CPUID) >> 20;

#ifndef QT_DEBUG
        if (chipID != Chip.chipID)
            ERR(QString("Different chip ID, set: 0x%1; Device: 0x%2").arg(Chip.chipID,0,16).arg(chipID,0,16));
#endif
    }

    virtual void BreakpointWrite(uint32_t address) throw (QString) = 0;
    virtual void BreakpointRemove(uint32_t address) throw (QString) = 0;

    void FlashClear(uint32_t address, uint32_t length) throw (QString)
    {

    }

    void FlashMassClear() throw (QString)
    {

    }

    void FlashWrite(uint32_t address, const QByteArray & data) throw (QString)
    {

    }

signals:
    //break reached
    void breakpint(void);

protected:
    uint32_t chipID;
    const chip_properties_t Chip;

};

#endif // QARMABSTRACT_H
