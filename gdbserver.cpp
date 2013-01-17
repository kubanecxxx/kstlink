#include "gdbserver.h"
#include "qstlink.h"
#include "qlog.h"
#include <QFile>
#include <QTcpSocket>
#include <inttypes.h>
#include <QtEndian>
#include <QDir>

GdbServer::GdbServer(QObject *parent, const QByteArray &mcu, bool notverify, int portnumber, QByteArray & file) :
    QObject(parent),
    stlink(new QStLink(this,mcu)),
    server(new QTcpServer(this)),
    port(portnumber),
    NotVerify(notverify),
    VeriFile(file)
{

    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    connect(stlink,SIGNAL(CoreHalted(uint32_t)),this,SLOT(CoreHalted(uint32_t)));
    bool ok = server->listen(QHostAddress::Any,port);

    qDebug() << QString("Core status: " + stlink->GetCoreStatus());
    qDebug() << QString("Core ID: 0x%1").arg(stlink->GetCoreID(),0,16);
    qDebug() << QString("Chip ID: 0x%1").arg(stlink->GetChipID(),0,16);
    qDebug() << QString("Breakpoint count: %1").arg(stlink->GetBreakpointCount());
    qDebug() << QString("Chip name: " + stlink->GetMcuName());

    QDir fil(VeriFile);
    if (fil.exists())
    {
        VeriFile.clear();
        qDebug() << QString("Binary file: " + fil.absolutePath());
        VeriFile.append(fil.absolutePath());
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
}

void GdbServer::MakePacket(QByteArray &checksum,QByteArray * binary)
{
    uint8_t check = 0;

    for (int i = 0 ; i < checksum.count(); i++)
    {
        check += (uint8_t)checksum.at(i);
    }

    GDB_SEND(checksum);
    checksum.prepend('$');
    checksum.prepend('+');
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
bool GdbServer::DecodePacket(QByteArray &data)
{
    QByteArray temp(data);

    if (data.contains("vFlashWrite"))
    {
        asm("nop");
    }

    int start = temp.indexOf('$');
    int stop = temp.indexOf('#');

    if (start == -1 || stop == -1)
        return false;

    temp = temp.mid(start + 1, stop-start -1);

    data = temp;

    for (int i = data.indexOf('}') ; i < data.count(); i = data.indexOf('}',i + 1))
    {
        if (i == -1)
            break;
        if ((data.at(i + 1) == ('#' ^ 0x20)) || (data.at(i + 1) == ('$' ^ 0x20))
                || (data.at(i + 1) == ('}' ^ 0x20))  )
        {
            char temp = data.at(i+1);
            QByteArray arr = "}";
            arr.append(temp);
            QByteArray dva;
            dva.append((char)(temp ^ 0x20));
            data.replace(arr,dva);
        }
    }

    return true;
}

void GdbServer::ReadyRead()
{
    QTcpSocket * client = qobject_cast<QTcpSocket *>(sender());
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

    if (!DecodePacket(input))
        return;

    try {
        //process packet
        processPacket(client,input);
    } catch (QString data)
    {
        WARN(data);
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
    if (data.contains("Hg"))
    {
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
    //get general registers r0 - r15
    else if (data == "g")
    {
        QByteArray arr;
        arr.resize(84);
        stlink->ReadAllRegisters((uint32_t *)arr.data());
        arr.resize(64);

        ans = arr.toHex();
        MakePacket(ans);
    }
    //memory read
    else if (data.startsWith("m"))
    {
        params_t pars = ParseParams(data);
        uint32_t addr = (pars[0]).toInt(NULL,16);
        uint32_t len = pars[1].toInt(NULL,16);
        ans.clear();

        stlink->ReadRam(addr,len,ans);
        ans = ans.toHex();
        MakePacket(ans);
    }
    //memory write
    else if (data.startsWith("M") || data.startsWith("X"))
    {
        params_t pars = ParseParams(data);
        uint32_t addr = (pars[0]).toInt(NULL,16);
        uint32_t len = pars[1].toInt(NULL,16);
        if (len > 0)
        {
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

        if (idx == 18)
            idx = 20;
        else if (idx == 19)
            idx = 16;
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
    //kill program
    else if (data == "k" || data == "D")
    {
        ans = "+";
        stlink->BreakpointRemoveAll();
        stlink->CoreRun();
    }
    else if (data.startsWith("vFlashErase"))
    {
        //params_t pars = ParseParams(data);
        //uint32_t addr = data.mid(12,8).toInt(NULL,16);
        //uint32_t len = data.mid(21,8).toInt(NULL,16);

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
            while(temp)
            {
                lastAddress++;
                temp = addr - lastAddress;
                FlashProgram.append('\0');
            }
        }

        FlashProgram.append(data.mid(20));
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
        stlink->FlashWrite(FLASH_BASE,FlashProgram);
        if (!NotVerify)
        {
            connect(stlink,SIGNAL(Reading(int)),this,SLOT(Verify(int)));
            qDebug() << "Verifying";
            if (stlink->FlashVerify(FlashProgram))
                qDebug() << "Verified OK";
            else
                qWarning("Verification Failed");
            disconnect(stlink,SIGNAL(Reading(int)),this,SLOT(Verify(int)));
        }
        ans  = "OK";
        MakePacket(ans);
        asm("nop");
    }
    //GDB_SEND(ans.toHex());
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
    else if (data == "qAttached" || data == "qTStatus" || data == "qOffsets")
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
        ans = "l";
        MakePacket(ans);
    }
    else if (data.startsWith("qRcmd"))
    {
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
        else if (arr.startsWith("verify"))
        {
            QFile file;
            if (arr == "verify")
            {
                file.setFileName(VeriFile);
            }
            else
            {
                arr.remove(0,7);
                file.setFileName(arr);
            }

            if (file.open(QFile::ReadOnly))
            {
                bool ok = stlink->FlashVerify(file.readAll());
                if (ok)
                    qDebug() << "Verif ok";
                else
                    qDebug() << "Verif failed";
                file.close();
            }
            else
            {
                qDebug() << "Bad file";
            }
        }
        else if (arr == "erase")
        {
            stlink->FlashMassClear();
            qDebug() << "Erased";
        }

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
    //last command was run
    if (input == "c")
    {
        ans = "S05";
        INFO("Breakpoint");
        GDB_SEND(ans);
        MakePacket(ans);
        soc->write(ans);
    }
}

void GdbServer::Erasing(int perc)
{
    qDebug() << QString("Erasing progress: %1\%").arg(perc);
}

void GdbServer::Flashing(int perc)
{
    qDebug() << QString("Flashing progress: %1\%").arg(perc);
}

void GdbServer::Verify(int perc)
{
    qDebug() << QString("Verifying progress: %1\%").arg(perc);
}
