#include "gdbserver.h"
#include "qstlink.h"
#include "qlog.h"
#include <QFile>
#include <QTcpSocket>
#include <inttypes.h>
#include <QtEndian>

GdbServer::GdbServer(QObject *parent) :
    QObject(parent),
    stlink(new QStLink(this)),
    server(new QTcpServer(this)),
    port(4242)
{

    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    bool ok = server->listen(QHostAddress::Any,port);

    if (ok)
    {
        INFO(QString("Listening on port %1").arg(port));
        qDebug() << QString("Listening on port %1").arg(port);
    }
    else
    {
        ERR("Cannot open listening port");
    }

    stlink->SysReset();
 //   stlink->BreakpointWrite(0x80020c2);
    stlink->CoreRun();

    while(1)
    {
        stlink->GetCoreStatus();
    }
}

void GdbServer::MakePacket(QByteArray &checksum)
{
    uint8_t check = 0;

    for (int i = 0 ; i < checksum.count(); i++)
    {
        check += (uint8_t)checksum.at(i);
    }

    GDB_SEND(checksum);
    checksum.prepend('$');
    checksum.prepend('+');
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

    int start = temp.indexOf('$');
    int stop = temp.indexOf('#');

    if (start == -1 || stop == -1)
        return false;

    temp = temp.mid(start + 1, stop-start -1);

    data = temp;
    return true;
}

void GdbServer::ReadyRead()
{
    QTcpSocket * client = qobject_cast<QTcpSocket *>(sender());
    input = client->readAll();

    //special packet +-
    if (input == "+")
        return;

    if (input == "-")
    {
        WARN(input);
        WARN("send" + ans);
        client->write(ans);
        return;
    }

    if (!DecodePacket(input))
        return;

    //process packet
    processPacket(client,input);
}

GdbServer::params_t GdbServer::ParseParams(const QByteArray &data)
{
    params_t temp;
    QByteArray ar(data);
    ar.remove(0,1);

    int i,j;
    while((i = ar.indexOf(',')) > 0 || (j = ar.indexOf('=')) >  0)
    {
        int k;
        if (i != -1)
            k = i;
        else if (j != -1)
            k = j;
        QByteArray t;
        t = ar.left(k);
        ar.remove(0,k + 1);
        temp.push_back(t);
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
        uint32_t len = pars[1].toInt();
        ans.clear();

        stlink->ReadRam(addr,len,ans);
        ans = ans.toHex();
        MakePacket(ans);
    }

    //read register
    else if (data.startsWith("p"))
    {
        params_t pars = ParseParams(data);

        uint32_t idx = pars[0].toInt();

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
        uint8_t reg = pars[0].toInt();
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
        stlink->CoreRun();
    }
    //GDB_SEND(ans.toHex());
    client->write(ans);
}

QByteArray GdbServer::processQueryPacket(const QByteArray &data)
{
    QByteArray ans;

    if (data ==("qSupported:qRelocInsn+"))
    {
        ans.append("PacketSize=3fff");
        ans.append(";qXfer:memory-map:read+");
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
        QFile map("map.xml");
        map.open(QFile::ReadOnly);
        ans = map.readAll();
        ans.prepend('m');
        MakePacket(ans);
    }
    else if (data == "qXfer:memory-map:read::27d,d82")
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
    uint8_t kind = pars[2].toInt();

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
