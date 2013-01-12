#include "qarm_cm3.h"
#include "qstlink.h"
#include "armConstants.h"
#include <QFile>
#include "stm100.h"

QArm3::QArm3(QObject *parent, const chip_properties_t & chip) :
    QArmAbstract(parent, chip),
    cm3FPB(*this)
{
    Properties.chipID = chipID;
    Properties.stlink = &StProperties;

    CoreStop();
    ReadAllRegs();
    EnableFPB();

    stm100 stm(*this,1024,128);
    bool ok = stm.VerifyErased(1);

/*
 *  až pojede loadování
 * tak otestovat erase a verify range
 */

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

    WriteRegister(15,0x20000000);

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
    if (address >= SRAM_BASE)
        throw(QString("Cannot setup breakpoint into RAM"));

    bool ok = LoadBreakPoint(address);

    if (!ok)
        throw(QString("No resources to setup another breakpoint"));
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


