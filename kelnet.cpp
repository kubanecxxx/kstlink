#include "kelnet.h"

Kelnet::Kelnet(QStLink & st,QObject *parent) :
    QObject(parent),
    stlink(st),
    server(new QTcpServer(this)),
    client(NULL)
{
    server->listen(QHostAddress::Any,4243);
    connect(server,SIGNAL(newConnection()),this,SLOT(NewConnection()));
}

void Kelnet::NewConnection()
{
    if (client == NULL)
    {
        client = server->nextPendingConnection();
        connect(client,SIGNAL(readyRead()),this,SLOT(ReadyRead()));
        connect(client,SIGNAL(disconnected()),this,SLOT(Closed()));
        qDebug() << "Kmaster connected";
    }
    else
    {
        server->nextPendingConnection();
    }
}

void Kelnet::Closed()
{
    client = NULL;
    qDebug() << "Kmaster disconnected";
}

void Kelnet::ReadyRead()
{
    QByteArray data = client->readAll();
    QByteArray ans;

    if (data.startsWith("s"))
    {
        ans.append("s");
        ans.append(stlink.GetCoreStatus());
    }
    else if (data.startsWith("r"))
    {
        param_t par = parsePacket(data);
        uint32_t addr = par[0].toInt(NULL,16);
        uint32_t len = par[1].toInt(NULL,16);

        QByteArray temp;
        stlink.ReadRam(addr,len,temp);
        QString str = QString("%1").arg(addr,8,16,QChar('0'));
        ans.append("r");
        ans.append(str);
        ans.append(",");
        ans.append(temp);
    }
    else if (data.startsWith("W"))
    {
        param_t par = parsePacket(data);
        uint32_t addr = par[0].toInt(NULL,16);
        //uint32_t len = par[1].toInt(NULL,16);
        QByteArray data = par[2];

        stlink.WriteRam(addr,data);
        ans.append("W");
    }

    client->write(ans);
    client->flush();
}

Kelnet::param_t Kelnet::parsePacket(const QByteArray &data)
{
    param_t pars;
    QByteArray temp (data);

    temp.remove(0,1);

    while (temp.indexOf(",") != -1)
    {
        QByteArray param;
        param = temp.mid(0,temp.indexOf(","));
        pars.push_back(param);
        temp.remove(0,temp.indexOf(",") + 1);
    }

    pars.push_back(temp);

    return pars;
}
