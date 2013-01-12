#ifndef QARM_H
#define QARM_H

#include <QObject>
#include "qarmabstract.h"
#include "cm3fpb.h"

class QArm3 : public QArmAbstract, public cm3FPB
{
    Q_OBJECT
public:
    explicit QArm3(QObject *parent, const chip_properties_t & chip);

    //arm stm32 register set
    typedef struct
    {
        uint32_t r[16];
        //r15 - program counter
        //uint32_t s[32];
        uint32_t xpsr;
        uint32_t main_sp;
        uint32_t process_sp;
        uint8_t control;
        uint8_t faultmask;
        uint8_t basepri;
        uint8_t primask;
        uint32_t fpscr;
    } cm3_regs_t;

    typedef struct
    {
        cm3_regs_t coreRegs;
        int CodeComparators;
        int LitComparators;
        int chipID;
        QStLink::stlink_properties_t * stlink;
    } properties_t;

    void WriteRam(uint32_t address, const QByteArray &buffer) throw (QString);
    cm3_regs_t ReadAllRegs();

    void BreakpointWrite(uint32_t address) throw (QString);
    void BreakpointRemove(uint32_t address) throw (QString);

    void FlashClear(uint32_t address, uint32_t length) throw (QString);
    void FlashMassClear() throw (QString);
    void FlashWrite(uint32_t address, const QByteArray & data) throw (QString);
signals:

public slots:

private:
    properties_t Properties;
   // static const Debug_t CONSTANTS;
    friend class cm3FPB;
};

#endif // QARM_H
