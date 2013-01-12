#include "qarm_cm3.h"
#include "qstlink.h"
#include "armConstants.h"
#include <QFile>

QArm3::QArm3(QObject *parent) :
    QArmAbstract(parent),
    cm3FPB(*this)
{
    CoreStop();
    ReadAllRegs();

    EnableFPB();



    /*
     * test - play some program into ram and step it
     */
#if 0
    LoadBreakPoint(0x08000006);
    QFile file("neco.bin");
    bool ok = file.open(QFile::ReadOnly);

    QByteArray array = file.readAll();
    WriteRam(0x20000000,array);
    QByteArray ver;
    ReadRam(0x20000000,20,ver);

    WriteRegister(15,0x08000000);

   ReadAllRegs();
   CoreRun();
   for(;;)
   {
   ReadAllRegs();
   GetCoreStatus();
   CoreStop();
   }
    for (;;)
    {
    CoreSingleStep();
   ReadAllRegs();
    }
#endif
    asm("nop");
}

QArm3::cm3_regs_t QArm3::ReadAllRegs()
{
    cm3_regs_t ret;
    ReadAllRegisters(&ret,sizeof(ret));
    Properties.coreRegs = ret;

    return ret;
}

void QArm3::WriteRam(uint32_t address, const QByteArray &buffer) throw (QString)
{
    if (address < SRAM_BASE)
        throw(QString("WriteRam: address is out of range"));

    QStLink::WriteRam(address,buffer);
}

void QArm3::BreakpointWrite(uint32_t address) throw (QString)
{
    //emit breakpint();
}

void QArm3::BreakpointRemove(uint32_t address) throw (QString)
{

}

void QArm3::FlashClear(uint32_t address, uint32_t length) throw (QString)
{

}

void QArm3::FlashMassClear() throw (QString)
{

}

void QArm3::FlashWrite(uint32_t address, const QByteArray & data) throw (QString)
{

}


