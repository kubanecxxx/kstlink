#include <QCoreApplication>
#include <include.h>
#include "qstlink.h"
#include "temp.h"
#include <QFile>
#include "QDebug"
#include "gdbserver.h"

unsigned int log_level = 3;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

/*
    QArm3::Debug_t deb;

    deb.FLASH_BASE = STM32_FLASH_BASE;
    deb.RAM_BASE = STM32_SRAM_BASE;
    deb.REG_CPUID = CM3_REG_CPUID;
    deb.REG_FP_CTRL = CM3_REG_FP_CTRL;
    deb.REG_COMP_BASE = CM3_REG_FP_COMP0;
    deb.COMP_REG_COUNT = 8;
    deb.DHCSR = _DHCSR;
    deb.DCRDR = _DCRDR;
    deb.DCRSR = _DCRSR;
    deb.DBGKEY = _DBGKEY;
*/
#if 1
    try
    {
        new GdbServer(&a);
    } catch (QString data)
    {
        WARN(data);
    }
#else

    QStLink * link;
    try {
        link = new QStLink(&a);
    }
    catch(QString data)
    {
        ERR(data);
    }

    temp * jo = new temp(&a);

    QObject::connect(link,SIGNAL(Erasing(int)),jo,SLOT(erasing(int)));
    QObject::connect(link,SIGNAL(Flashing(int)),jo,SLOT(flashing(int)));
    QObject::connect(link,SIGNAL(Reading(int)),jo,SLOT(read(int)));



    QFile file("termostat.bin");
    bool co = file.open(QFile::ReadOnly);

    QByteArray array = file.readAll();

    link->FlashWrite(FLASH_BASE,array);
    bool ok = link->FlashVerify(array);

    if (ok)
        qDebug() << "Success";
    else
        qDebug() << "Failed";

    link->SysReset();
    link->CoreRun();
#endif
    return a.exec();
}


