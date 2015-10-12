#include "gdbserver.h"
#include "qstlink.h"
#include "qlog.h"
#include <QFile>
#include <QTcpSocket>
#include <inttypes.h>
#include <QtEndian>
#include <QDir>
#include "kelnet.h"
#include <iostream>

#define THD_MAIN 1
#define THD_HAN 5

GdbServer::GdbServer(QObject *parent, const QByteArray &mcu, bool notverify, int portnumber, QByteArray & file, bool stop) :
    QObject(parent),
    stlink(new QStLink(parent,mcu,stop)),
    server(new QTcpServer(this)),
    port(portnumber),
    NotVerify(notverify),
    VeriFile(file)
{
    //VeriFile = "/home/kubanec/workspace/ARMCM4-STM32F407-DISCOVERY/build/test.bin";
    new Kelnet(*stlink,parent);


    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    connect(stlink,SIGNAL(CoreHalted(quint32)),this,SLOT(CoreHalted(quint32)));
    bool ok = server->listen(QHostAddress::Any,port);


    printMCUInfo();

    QFile fil(VeriFile);
    if (fil.exists())
    {
        qDebug() << QString("Binary file: " + fil.fileName());
        //VeriFile.append(fil.absolutePath());
    }

    if (ok)
    {
        INFO(QString("Listening on port %1").arg(port));
        qDebug() << QString("Listening on port %1").arg(port);
    }
    else
    {
        ERR("Cannot open listening port");
    }

    connect(stlink,SIGNAL(Erasing(int)),this,SLOT(Erasing(int)));
    connect(stlink,SIGNAL(Flashing(int)),this,SLOT(Flashing(int)));

    threaed.insert(THD_MAIN,QStLink::Thread);
    threaed.insert(THD_HAN,QStLink::Handler);
    threaed.insert(0,QStLink::Unknown);
    QTimer * timer = new QTimer;
    timer->start(1000);
    connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));
    soc = NULL;

    qsThreadInfo = 0;

    std::cout << std::endl << std::flush;
}

void GdbServer::printMCUInfo()
{
    QStringList lst;
    lst << ("Thread Mode");
    lst << "Reserved" << "NMI" << "HardFault" << "MemManage" << "BusFault";
    lst << "Usage Fault" << "Reserved" << "Reserved" << "Reserved" << "Reserved";
    lst << "SVCall" << "Reserved for debug" << "Reserved" << "PendSV" << "SysTick";

    for (int i = 0 ; i < 250 ; i++)
        lst << QString("IRQ%1").arg(i);

    handlers_list = lst;

    uint32_t xpsr = stlink->ReadRegister(XPSR);
    uint32_t psp = stlink->ReadRegister(PSP);
    uint32_t msp = stlink->ReadRegister(MSP);
    uint32_t pc = stlink->ReadRegister(PC);


    qDebug() << QString("Core status: " + stlink->GetCoreStatus());
    qDebug() << QString("xPSR: 0x%1, (%2)").arg(xpsr,0,16 ).arg(lst.at(xpsr & 0xff));
    qDebug() << QString("MSP/PSP 0x%1/0x%2").arg(msp,0,16).arg(psp,0,16);
    qDebug() << QString("PC 0x%1").arg(pc,0,16);
    qDebug() << QString("Core ID: 0x%1").arg(stlink->GetCoreID(),0,16);
    qDebug() << QString("Chip ID: 0x%1").arg(stlink->GetChipID(),0,16);
    qDebug() << QString("Breakpoint count: %1").arg(stlink->GetBreakpointCount());
    qDebug() << QString("Chip name: " + stlink->GetMcuName());

}

void GdbServer::timeout()
{/*
    QByteArray d;
    d = "budak";
    MakePacket(d);
    d.remove(0,2);
    d.prepend('%');


    if (socan)
        socan->write(d);
        */
}

void GdbServer::MakePacket(QByteArray &checksum,QByteArray * binary,bool ok)
{
    uint8_t check = 0;

    for (int i = 0 ; i < checksum.count(); i++)
    {
        check += (uint8_t)checksum.at(i);
    }

    GDB_SEND(checksum);
    checksum.prepend('$');
    if (ok)
        checksum.prepend('+');
    else
        checksum.prepend('-');
    if (binary)
    {
        checksum.append(*binary);
    }
    checksum.append('#');

    QString temp = QString("%1").arg(check,2,16,QChar('0'));
    checksum.append(temp);
}

void GdbServer::newConnection()
{
    QTcpSocket * client;
    qDebug() << "new connection";
    client = server->nextPendingConnection();

    connect(client,SIGNAL(readyRead()),this,SLOT(ReadyRead()));
}

///@todo checksum arq
int GdbServer::DecodePacket(QByteArray &data)
{
    static QByteArray temp;
    temp += data;

    if (data.contains("vFlashWrite"))
    {
        asm("nop");
    }

    int start = temp.indexOf('$');
    int stop = temp.indexOf('#',temp.count()-5);

    //packet is not complete
    if (stop == -1)
    {
        WARN("part of flash data packet");
        return 1;
    }

    if (start == -1 )
    {
        return -1;
    }

    if (stop < data.count() - 10)
    {
        asm("nop");
    }

    temp = temp.mid(start + 1, stop-start -1);
    data = temp;
    temp.clear();

    return 0;
}

void GdbServer::ReadyRead()
{
    QTcpSocket * client = qobject_cast<QTcpSocket *>(sender());
    socan = client;
    input = client->readAll();

    //special packet +- interrupt
    if (input == "+")
        return;

    if (input == "-")
    {
        WARN(input);
        WARN("send" + ans);
        client->write(ans);
        return;
    }

    if (input.at(0) == 3 && input.count() == 1)
    {
        stlink->CoreStop();
        ans = "S05";
        MakePacket(ans);
        client->write(ans);
        INFO("Interrupt");
        return;
    }

    try {
        //process packet
        int ja = DecodePacket(input);
        if (ja == 0)
            processPacket(client,input);
        else if (ja == 1)
        {
            //chvilu pockat
            //client->write("-");
        }
    } catch (QString data)
    {
        WARN(data);
        ans = "E00";
        MakePacket(ans);
        client->write(ans);
    }

}

class sep_t
{
public:
    sep_t()
    {
        separator = 0;
        value = 0;
    }
    char separator;
    int value;

    bool operator<(const sep_t & other) const
    {
        return value < other.value;
    }
};



GdbServer::params_t GdbServer::ParseParams(const QByteArray &data)
{
    params_t temp;
    QByteArray ar(data);
    ar.remove(0,1);

    QByteArray separators = ",=:";
    QVector<sep_t> vec;

    int k = 1;
    while (k)
    {
        for(int j = 0 ; j < separators.count(); j++)
        {
            char sep = separators.at(j);
            if (ar.indexOf(sep) != -1)
            {
                sep_t temp;
                temp.separator = sep;
                temp.value = ar.indexOf(sep);
                vec.push_back(temp);
            }
        }
        if (vec.isEmpty())
            break;

        qSort(vec);
        k = vec[0].value;
        char sep = vec[0].separator;
        vec.clear();

        QByteArray t;
        t = ar.left(k);
        ar.remove(0,k + 1);
        temp.push_back(t);

        if (sep == ':')
        {
            //binary data
            temp.push_back(ar);
            return temp;
        }
    }

    temp.push_back(ar);

    asm("nop");
    return temp;
}

void GdbServer::processPacket(QTcpSocket *client,const QByteArray &data)
{
    GDB_REC(data);
    ans.clear();

    if (data.startsWith('q'))
    {
        ans = processQueryPacket(data);
    }
    else if (data.startsWith("Z") || data.startsWith("z"))
    {
        ans = processBreakpointPacket(data);
    } else

    //init section
    if (data.startsWith("Hg"))
    {
        thread_id = data.mid(2).toInt(NULL,16);
        ans.append("OK");
        MakePacket(ans);
    }
    else if (data.startsWith("H"))
    {
        ans.append("OK");
        MakePacket(ans);
    }
    //if target halts
    else if (data == ("?"))
    {
        //if halts
        stlink->CoreStop();
        ans.append("S05");
        MakePacket(ans);
    }
    else if (data == "vCont?")
    {
        ans.clear();
        MakePacket(ans);
    }
    //get general registers r0 - r15 (sp,lr,pc)
    else if (data == "g")
    {
        QByteArray arr;
        QVector<quint32> regs;

        int threads = 1;
        if (stlink->GetMode() == QStLink::Handler)
            threads = 5;
        int context = threads;
        regs = stlink->ReadAllRegisters32();

        QStLink::Vector32toByteArray(arr,regs);
        ans = arr.toHex();
        MakePacket(ans);
    }
    //memory read
    else if (data.startsWith("m"))
    {
        params_t pars = ParseParams(data);
        uint32_t addr = (pars[0]).toLong(NULL,16);
        if (addr == 0)
        {
            asm("nop");
        }
        uint32_t len = pars[1].toInt(NULL,16);
        ans.clear();

        stlink->ReadRam(addr,len,ans);
        ans = ans.toHex();
        MakePacket(ans);
    }
    //memory write
    //data.startsWith("M") ||
    else if ( data.startsWith("X"))
    {
        params_t pars = ParseParams(data);
        uint32_t addr = (pars[0]).toLong(NULL,16);
        uint32_t len = pars[1].toInt(NULL,16);
        if (len > 0)
        {
            processEscapeChar(pars[2]);
            QByteArray buf = pars[2];
            stlink->WriteRam(addr,buf);
        }
        ans.clear();


        ans = "OK";
        MakePacket(ans);
    }
    //read register
    else if (data.startsWith("p"))
    {
        params_t pars = ParseParams(data);

        uint32_t idx = pars[0].toInt(NULL,16);
        uint32_t reg = stlink->ReadRegister(idx);

        ans.resize(4);
        qToLittleEndian(reg,(uchar *)ans.data());
        ans = ans.toHex();
        MakePacket(ans);
    }
    //write register
    else if (data.startsWith("P"))
    {
        params_t pars = ParseParams(data);
        uint8_t reg = pars[0].toInt(NULL,16);
        QByteArray temp = QByteArray::fromHex(pars[1]);
        uint32_t val = qFromLittleEndian<uint32_t>((uchar*)temp.constData());
        stlink->WriteRegister(reg,val);

        ans = "OK";
        MakePacket(ans);
    }
    //stepping
    else if(data.startsWith("s"))
    {
        if (data.count() > 1)
        {
            //step addr
            params_t par = ParseParams(data);
        }
        else
        {
            //single step
            stlink->CoreSingleStep();
        }
        ans = "S05";
        MakePacket(ans);
    }
    //thread info
    else if(data.startsWith("T"))
    {
        params_t par = ParseParams(data);
        uint32_t id = par[0].toInt(NULL,16);

        if (id == THD_MAIN)
            ans = "OK";
        if (id == THD_HAN)
        {
            if (stlink->GetMode() == QStLink::Handler)
                ans = "OK";
            else
                ans = "E00";
        }

        MakePacket(ans);
    }
    //continue
    else if (data.startsWith("c"))
    {
        params_t pars = ParseParams(data);
        QByteArray temp= pars[0];

        if (temp.count())
        {
            //address where to start
        }
        ans = "+";
        soc = client;
        stlink->CoreRun();
    }
    //baud speed
    else if (data.startsWith("b"))
    {
        ans = "+";
    }
    //kill program
    else if (data == "k" || data == "D")
    {
        ans = "+";
        stlink->BreakpointRemoveAll();
        stlink->CoreRun();
    }
    else if (data.startsWith("vFlashErase"))
    {
        FlashProgram.clear();
        ans = "OK";
        MakePacket(ans);
    }
    else if (data.startsWith("vFlashWrite"))
    {
        uint32_t addr = (data.mid(12,data.indexOf(":",12)-12)).toInt(NULL,16) - FLASH_BASE;

        if (addr)
        {
            int64_t temp = addr - lastAddress;
            while(temp > 0)
            {
                lastAddress++;
                temp = addr - lastAddress;
                FlashProgram.append('\0');
            }
        }

        QByteArray temp  = data.mid(20);
        processEscapeChar(temp);

        FlashProgram.append(temp);
#if 0
        for (int i = 0 ; i < FlashProgram.count(); i++)
        {
            QFile file("ch.bin");
            file.open(QFile::ReadOnly);
            QByteArray temp = file.readAll();
            if (FlashProgram.at(i) != temp.at(i))
            {
                asm("nop");
            }
        }
#endif
        lastAddress = FlashProgram.count();
        ans  = "OK";
        MakePacket(ans);
    }
    else if (data.startsWith("vFlashDone"))
    {
        QFile file(VeriFile);
        if (file.open(QFile::ReadOnly))
        {
            QByteArray fil = file.readAll();
            QByteArray ringFile;
            QByteArray ringGdb;
            for (int i = 0; i < fil.count(); i++)
            {
                ringFile.append(fil.at(i));
                ringGdb.append(FlashProgram.at(i));

                if (ringFile.count() > 10)
                    ringFile.remove(0,1);

                if (ringGdb.count() > 10)
                    ringGdb.remove(0,1);

            }
        }

        std::cout << "Program size " << FlashProgram.count() << " bytes" << std::flush;
        stlink->FlashWrite(FLASH_BASE,FlashProgram);
        if (!NotVerify)
        {
            connect(stlink,SIGNAL(Reading(int)),this,SLOT(Verify(int)));
            if (stlink->FlashVerify(FlashProgram))
            {
                std::cout << std::endl << "Verification OK" << std::endl;
                std::cout.flush();
            }
            else
            {
                WARN("Verification failed");
            }
            disconnect(stlink,SIGNAL(Reading(int)),this,SLOT(Verify(int)));
        }
        if (ans.count() == 0)
            ans  = "OK";
        MakePacket(ans);
        asm("nop");
    }
    client->write(ans);
}

QByteArray GdbServer::processQueryPacket(const QByteArray &data)
{
    QByteArray ans;

    if (data.contains( "qSupported:"))
    {
        ans.append("PacketSize=3fff");
        ans.append(";qXfer:memory-map:read+");
        stlink->BreakpointRemoveAll();
        MakePacket(ans);
    }
    else if (data == ("qC") || data == "qSymbol::")
    {
        ans.append("OK");
        MakePacket(ans);
    }
    else if (data == "qAttached" ||  data == "qOffsets")
    {
        MakePacket(ans);
    }
    else if (data == "qTStatus")
    {
        MakePacket(ans);
    }
    else if (data == ("qXfer:memory-map:read::0,fff"))
    {
        ans = stlink->GetMapFile();
        ans.prepend('m');
        MakePacket(ans);
    }
    else if (data.startsWith("qXfer:memory-map:read::27"))
    {
        ans = "l";
        MakePacket(ans);
    }
    else if (data == "qfThreadInfo")
    {
       ans = "m1";
        MakePacket(ans);
    }
    else if (data == "qsThreadInfo")
    {

        ans = "l";
        MakePacket(ans);
    }
    else if (data.startsWith("qThreadExtraInfo"))
    {
        params_t pars = ParseParams(data);
        int id = pars[1].toInt(NULL,16);

        ans = getHandler().toLocal8Bit();

        ans = ans.toHex();
        MakePacket(ans);
    }
    else if (data.startsWith("qRcmd"))
    {
        bool fail = false;
        params_t pars = ParseParams(data);
        QByteArray arr = QByteArray::fromHex(pars[1]);
        if (arr == "reset")
        {
            stlink->SysReset();
            stlink->BreakpointRemoveAll();
        }
        else if (arr == "Reset")
        {
            stlink->SysReset();
        }
        else if (arr == "LOAD")
        {
            QFile moje(VeriFile);

            if (moje.open(QFile::ReadOnly))
                stlink->FlashWrite(FLASH_BASE,moje.readAll());
        }
        else if (arr.startsWith("verify"))
        {
            QFile * file;
            if (arr == "verify")
            {
                file = new QFile(VeriFile);
                //file.setFileName(VeriFile);
            }
            else
            {
                arr.remove(0,7);
                //file.setFileName(arr);
                file = new QFile(arr);
            }

            int temp = 0;

            if (file->open(QFile::ReadOnly))
            {
                bool ok = stlink->FlashVerify(file->readAll());
                if (ok)
                    temp =  0;
                else
                    temp = 1;
                file->close();
            }
            else
            {
                temp =  2;
            }

            QString text;
            switch (temp)
            {
            case 0: text = "Verification OK"; break;
            case 1: text = "Verification failed"; break;
            case 2: text = "No file found "; break;
            }

            qDebug() << text;
            delete file;
        }
        else if (arr == "erase")
        {
            stlink->FlashMassClear();
            qDebug() << "Erased";
        }
        else if (arr == "PSP")
        {
            quint32 psp = stlink->ReadRegister(PSP);
            QVector<quint32> vec;
            stlink->UnstackContext(vec,psp);
            QList<quint32> lst;
            lst = vec.toList();
            qDebug( ) << "PSP: " << QString("0x%1").arg(psp,0,16);
            while(lst.count())
            {
                qDebug() << QString("0x%1").arg(lst.takeLast(),0,16);
            }
        }
        else
        {
            qDebug() << "unknown remote command" << arr;
            ans = "";
            fail = true;
        }

        if (!fail)
            ans = "OK";
        MakePacket(ans);
    }

    return ans;
}

QByteArray GdbServer::processBreakpointPacket(const QByteArray &data)
{
    QByteArray ans;
    params_t pars = ParseParams(data);
    char z = data.at(0);

    uint8_t type = pars[0].toInt();
    uint32_t addr =(pars[1]).toInt(NULL,16);
    //uint8_t kind = pars[2].toInt();

    if (type != 1)
    {
        //not supported
        MakePacket(ans);
        return ans;
    }

    bool ok;
    //insert breakpoint
    if (z == 'Z')
    {
        ok = stlink->BreakpointWrite(addr);
    }
    //delete breakpoint
    else if (z == 'z')
    {
        ok = stlink->BreakpointRemove(addr);
    }

    if (ok)
        ans = "OK";
    else
        ans = "E00";
    MakePacket(ans);

    return ans;
}

void GdbServer::CoreHalted(uint32_t addr)
{
    (void) addr;
    //last command was continue
    if (input == "c")
    {
        ans = "S05";
        INFO("Breakpoint");
        GDB_SEND(ans);
        MakePacket(ans);
        soc->write(ans);
    }


}



void GdbServer::progressBar(int percent, const QString & operation)
{

    if (prevOpearation != operation)
    {
        prevOpearation = operation;
        std::cout << std::endl;
        std::cout << operation.toStdString() <<  std::endl;
    }

    float progress = percent / 100.0;
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << qMin(100,percent) << " %\xd";
    std::cout.flush();


}

void GdbServer::Erasing(int perc)
{
    progressBar(perc, "Erasing");
}

void GdbServer::Flashing(int perc)
{
    progressBar(perc,"Flashing");
}

void GdbServer::Verify(int perc)
{
    progressBar(perc,"Verifying");
}

void GdbServer::processEscapeChar(QByteArray & temp)
{
    for (int i = 0 ; i < temp.count() ; i++)
    {
        if (temp.at(i) == '}')
        {
            temp.remove(i,1);
            temp[i] = temp[i] ^ 0x20 ;
        }
    }
}

int GdbServer::threads()
{
    int thd = 1;
    if (stlink->GetMode() == QStLink::Handler)
        thd = 2;

    return thd;
}

QString GdbServer::getHandler()
{
    return handlers_list.at(stlink->ReadRegister(XPSR) & 0xFF);
}
