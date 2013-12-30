#include "cm3debugregs.h"
#include "qstlink.h"
#include <QDebug>

//definitive guide to the arm cortex-m3 page 257
const quint32 cm3DebugRegs::romBase = 0xE00FF000;

cm3DebugRegs::cm3DebugRegs(QStLink *stlink)
{
    quint32 addr = romBase;
    QVector<quint32> table(10);

    stl = stlink;
    Q_ASSERT(stl);

    for (int i = 0; i < table.count(); i++)
    {
        table[i]=stl->ReadMemoryWord(addr);
        addr += 4;
    }

    d.SCS_BASE = getBase(table[0]);
    d.DWT = getBase(table[1]);
    d.FPB = getBase(table[2]);
    d.ITM = getBase((table[3]));
    d.TPI = getBase(table[4]);
    d.ETM = getBase(table[5]);

    d.NVIC = d.SCS_BASE + 0x0100UL;
    d.SCB = d.SCS_BASE + 0x0D00UL;
    d.CoreDebug = d.SCS_BASE + 0xDF0U;
}

void cm3DebugRegs::printDebugRegisters(debug_regs flags)
{
    const int c = 4;
    if (flags.testFlag(FPB))
        systemPrint(printFPBRegisters(),c);
    if (flags.testFlag(ITM))
        systemPrint(printITMRegisters(),c);
    if (flags.testFlag(TPI))
        systemPrint(printTPIRegisters(),c);
    if (flags.testFlag(CORE))
        systemPrint(printCoreDebugRegs(),c);
}

QString cm3DebugRegs::printCoreDebugRegs()
{
    QVector<quint32> b(2);
    CoreDebug_Type cdt;
    memset(&cdt,0,sizeof(cdt));
    b[0] = cdt.DEMCR = stl->ReadMemoryWord(address(CoreDebug,DEMCR));
    b[1] = cdt.DHCSR = stl->ReadMemoryWord(address(CoreDebug,DHCSR));

    QStringList lst;
    lst << "DEMCR" << "DHCSR";

    return printTable(&cdt_alt,&cdt,sizeof(cdt),b,lst);
}

QString cm3DebugRegs::printTable(void *alt,const void *neu, size_t size,
                                 const QVector<quint32> &vct, const QStringList &lst)
{
    QString str;
    if (memcmp(alt,neu,size))
    {
        memcpy(alt,neu,size);

        for (int i = 0; i < lst.count(); i++)
            append(str,lst.at(i),vct.at(i));

        return str;
    }
    return QString();
}

QString cm3DebugRegs::printTPIRegisters()
{
    quint32 a;
    TPI_Type tpi;
    QVector<quint32> b(15);
    memset(&tpi,0,sizeof(tpi));

    b[0] = tpi.SSPSR = stl->ReadMemoryWord(a = address(TPI,SSPSR));
    b[1] = tpi.CSPSR = stl->ReadMemoryWord(a = address(TPI,CSPSR));
    b[2] = tpi.ACPR = stl->ReadMemoryWord(a = address(TPI,ACPR));
    b[3] = tpi.SPPR = stl->ReadMemoryWord(a = address(TPI,SPPR));
    b[4] = tpi.FFSR = stl->ReadMemoryWord(a = address(TPI,FFSR));
    b[5] = tpi.FFCR = stl->ReadMemoryWord(a = address(TPI,FFCR));
    b[6] = tpi.FSCR = stl->ReadMemoryWord(a = address(TPI,FSCR));
    b[7] = tpi.TRIGGER = stl->ReadMemoryWord(a = address(TPI,TRIGGER));
    b[8] = tpi.FIFO0 = stl->ReadMemoryWord(a = address(TPI,FIFO0));
    b[9] = tpi.ITATBCTR2= stl->ReadMemoryWord(a = address(TPI,ITATBCTR2));
    b[10] = tpi.FIFO1 = stl->ReadMemoryWord(a = address(TPI,FIFO1));
    b[11] = tpi.ITATBCTR0 = stl->ReadMemoryWord(a = address(TPI,ITATBCTR0));
    b[12] = tpi.ITCTRL = stl->ReadMemoryWord(a = address(TPI,ITCTRL));
    b[13] = tpi.CLAIMSET = stl->ReadMemoryWord(a = address(TPI,CLAIMSET));
    b[14] = tpi.CLAIMCLR = stl->ReadMemoryWord(a = address(TPI,CLAIMCLR));

    static QStringList lst;
    if (lst.isEmpty())
    {
        lst << "SSPSR" << "CSPSR" << "ACPR" << "SPPR" << "FFSR";
        lst << "FFCR" << "FSCR" << "TRIGGER" << "FIFO0" << "ITATBCTR2";
        lst << "FIFO1" << "ITATBCTR0" << "ITCTRL" << "CLAIMSET" << "CLAIMCLR";
    }

    return printTable(&tpi_alt,&tpi,sizeof(tpi),b,lst);
}

QString cm3DebugRegs::printFPBRegisters()
{
    quint32 a =  Base().FPB;
    cm3DebugRegs::FPB_u_t fbp;
    memset(&fbp,0,sizeof(fbp));
    for (int i = 0 ;i < 10; i++)
    {
        fbp.p[i] = stl->ReadMemoryWord(a);
        a+= 4;
    }

    if (memcmp(&fbp,&fpb_alt,sizeof(fbp)))
    {
        memcpy(&fpb_alt,&fbp,sizeof(fbp));

        QString str;
        QStringList lst;
        lst << "CTRL" << "REMAP";

        for (int i = 0 ; i < 8 ; i++)
        {
            QString jo = "COMP";
            jo.append(QString("%1").arg(i));
            lst << jo;
        }
        for (int i = 0 ; i < lst.count(); i++)
        {
            append(str,lst.at(i),fbp.p[i]);
        }

        return str;
    }
    return QString();
}

QString cm3DebugRegs::printITMRegisters()
{
    quint32 a;
    ITM_Type itm;
    memset(&itm,0,sizeof(itm));

    itm.TER = stl->ReadMemoryWord(a = address(ITM,TER));
    itm.TCR = stl->ReadMemoryWord(a = address(ITM,TCR));
    itm.TPR = stl->ReadMemoryWord(a = address(ITM,TPR));
    itm.IMCR = stl->ReadMemoryWord(a = address(ITM,IMCR));
    itm.IWR = stl->ReadMemoryWord(a = address(ITM,IWR));
    itm.IRR = stl->ReadMemoryWord(a = address(ITM,IRR));
    itm.LAR = stl->ReadMemoryWord(a = address(ITM,LAR));
    itm.LSR = stl->ReadMemoryWord(a = address(ITM,LSR));

    a =  Base().ITM ;
    for (int i = 0 ;i < 32; i++)
    {
        itm.PORT[i].u32 = stl->ReadMemoryWord(a);
        a+= 4;
    }

    if (memcmp(&itm,&itm_alt,sizeof(itm)))
    {
        memcpy(&itm_alt,&itm,sizeof(itm));

        QString str;
        QStringList lst;

        for (int i = 0 ; i < 32 ; i++)
        {
            QString jo = "PORT";
            jo.append(QString("%1").arg(i,2,10,QChar('0')));
            lst << jo;
        }

        for (int i = 0 ; i < lst.count(); i++)
        {
            bool wrap = false;
            if ((i % 4) == 3)
                wrap = true;
            append(str,lst.at(i),itm.PORT[i].u32,wrap);
        }

        append(str,"TER",itm.TER);
        append(str,"TCR",itm.TCR);
        append(str,"TPR",itm.TPR);
        append(str,"IMCR",itm.IMCR);
        append(str,"IWR",itm.IWR);
        append(str,"IRR",itm.IRR);
        append(str,"LAR",itm.LAR);
        append(str,"LSR",itm.LSR);


        return str;
    }

    return QString();
}

void cm3DebugRegs::append(QString &data, const QString &name, quint32 num, bool wrap)
{
    if (wrap)
        data.append(QString("%1 0x%2\n").arg(name).arg(num,8,16,QChar('0')));
    else
        data.append(QString("%1 0x%2 ").arg(name).arg(num,8,16,QChar('0')));
}

void cm3DebugRegs::systemPrint(const QString &data, int console)
{
    if (data.isEmpty())
        return;

    if (console == -1)
    {
        qDebug() << data;
        return;
    }

    QString d(data);
    d.prepend("echo \"");
    d.append(QString("\" > /dev/pts/%1").arg(console));

    system(d.toAscii().constData());
}

quint32 cm3DebugRegs::getBase(quint32 offset)
{
    return (offset + romBase) & 0xfffffff0;
}

