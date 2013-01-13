#include "qarm_cm3.h"
#include "qstlink.h"
#include "armConstants.h"
#include <QFile>
#include "stm100.h"
#include "stm32f10x.h"

QArm3::QArm3(QObject *parent, const chip_properties_t & chip) :
    QArmAbstract(parent, chip),
    cm3FPB(*this)
{
    Properties.chipID = chipID;
    Properties.stlink = &StProperties;

    EnableFPB();
    ReadAllRegisters();

    /*
     * mužu ho dat přimo do abstractu,
     * na stm budou taky abstraktni třida s jednim rozhranim
     * a budou se vytvářet do ní...
     */


/*
 *  až pojede loadování
 * tak otestovat erase a verify range
 */

    /*
     * test - play some program into ram and step it
     */
#if 1
    //LoadBreakPoint(0x08000006);
/*
    QFile program("termostat.bin");
    bool ok = program.open(QFile::ReadOnly);
    QByteArray array = program.readAll();
    stm100 stm(*this ,1024,128);
    EXECUTION_TIME(stm.WriteFlash(FLASH_BASE,array); , t);

    QByteArray read;
    EXECUTION_TIME(ReadRam(FLASH_BASE,array.count(),read); , read);

    int errs  = 0;
    for (int i = 0 ; i < read.count(); i++)
    {
        if (read.at(i) != array.at(i))
            errs++;
    }
*/
    CoreStop();
    ReadAllRegisters();
    //SysReset();
    WriteRegister(15,0x08000140);
    ReadAllRegisters();
    CoreRun();
#endif
    asm("nop");
}

QArm3::cm3_regs_t QArm3::ReadAllRegisters()
{
    cm3_regs_t ret;
    QStLink::ReadAllRegisters(&ret,sizeof(ret));
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


