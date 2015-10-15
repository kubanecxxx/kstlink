#include "communication.h"
#include <QDebug>

#ifdef KSTLINK_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#endif

Communication::Communication(QObject *parent) : QObject(parent)
{

}

#ifdef KSTLINK_DBUS
DBus::DBus(QDBusConnection * connection, QObject * parent):
    Communication(parent),
    con(connection)
{
    Q_ASSERT(con);
    service = "org.kubanec.kstlink";
    path = "/qstlink";
    interface = "org.kubanec.kstlink.stlink";
    //interface = "org.qtproject.Qt.QStLink";

    bool ok;
    ok = con->isConnected();
    Q_ASSERT(ok);


    //con = QDBusConnection::sessionBus();

     ok = connect("CoreRunning",SLOT(ch()));

    ok = connect("CoreHalted",SIGNAL(CoreHalted(quint32)));
    connect("CommunicationFailed",SIGNAL(CommunicationFailed()));

    connect("Verification",SIGNAL(Verification(bool)));
    connect("Reading",SIGNAL(Reading(int)));
    connect("Flashing",SIGNAL(Flashing(int)));
    connect("Erasing",SIGNAL(Erasing(int)));
    connect("CoreResetRequested",SIGNAL(ResetRequested()));

    //interface = "org.kubanec.kstlink.stlink";
    Q_ASSERT(ok);
}

bool DBus::connect(const QString &signal, const char *slot)
{
    return QDBusConnection::sessionBus().connect(service,path,interface,signal,this,slot);
}

void DBus::ch()
{
    emit CoreRunning();
}

QDBusMessage DBus::call(const QString &method,const QList<QVariant> & args)
{
    //QDBusInterface iface(service,path,interface,*con);
    //Q_ASSERT(iface.isValid());
    //iface.setTimeout(300);
    //QDBusMessage msg = iface.call(method);

    QDBusMessage msg = QDBusMessage::createMethodCall(service,path,interface,method);
    msg = con->call(msg,QDBus::Block, 300);

    //throw excpeiton

    return msg;
}

quint32 DBus::GetCoreID()
{
    QDBusMessage msg = call("GetCoreID");
    return msg.arguments().at(0).toUInt();
}

int DBus::GetStlinkMode(QString *text)
{
    QDBusMessage msg = call("GetStlinkMode");
    if (text)
        *text = msg.arguments().at(1).toString();

    return msg.arguments().at(0).toInt();
}

bool DBus::IsCoreHalted()
{
    QDBusMessage msg = call("IsCoreHalted");
    return msg.arguments().at(0).toBool();

}

int DBus::GetChipID()
{
    QDBusMessage msg = call("GetChipID");
    return msg.arguments().at(0).toInt();
}

QString DBus::GetCoreStatus()
{
    QDBusMessage msg = call("GetCoreStatus");
    return msg.arguments().at(0).toString();
}

int DBus::GetBreakpointCount()
{
    QDBusMessage msg = call("GetBreakpointCount");
    return msg.arguments().at(0).toInt();
}

QString DBus::GetMcuName()
{
    QDBusMessage msg = call("GetMcuName");
    return msg.arguments().at(0).toString();
}

void DBus::CoreStop()
{
    QDBusMessage msg = call("CoreStop");
}

void DBus::CoreRun()
{
    QDBusMessage msg = call("CoreRun");
}

void DBus::CoreSingleStep()
{
    QDBusMessage msg = call("CoreSingleStep");
}

void DBus::SysReset()
{
    QDBusMessage msg = call("SysReset");
}

QString DBus::GetModeString()
{
    QDBusMessage msg = call("GetModeString");
    return msg.arguments().at(0).toString();
}

quint32 DBus::ReadMemoryWord(quint32 address)
{
    QDBusMessage msg = call("ReadMemoryWord");

}

void DBus::WriteRamHalfWord(quint32 address, quint16 data)
{

}

void DBus::WriteRamByte(quint32 address, quint8 data)
{

}

void DBus::WriteRamWord(quint32 address, quint32 data)
{

}

void DBus::FlashMassClear()
{
    QDBusMessage msg = call("FlashMassClear");

}

void DBus::FlashWrite(uint32_t address, const QByteArray &data)
{

}

quint32 DBus::GetCycleCounter()
{
    QDBusMessage msg = call("GetCycleCounter");
    return msg.arguments().at(0).toUInt();
}

#endif


direct::direct(QStLink * stlink ,QObject *parent):
    Communication(parent),
    link(stlink)
{
    Q_ASSERT_X(stlink, "No stlink pointer","");

    connect(stlink,SIGNAL(CommunicationFailed()), this,SIGNAL(CommunicationFailed()));
    connect(stlink,SIGNAL(CoreHalted()), this,SIGNAL(CoreHalted()));
    connect(stlink,SIGNAL(CoreHalted(quint32)), this, SIGNAL(CoreHalted(quint32)));
    connect(stlink,SIGNAL(CoreResetRequested()),this,SIGNAL(CoreResetRequested()));
    connect(stlink,SIGNAL(CoreRunning()),this,SIGNAL(CoreRunning()));
    connect(stlink,SIGNAL(Erasing(int)),this,SIGNAL(Erasing(int)));
    connect(stlink,SIGNAL(Flashing(int)),this, SIGNAL(Flashing(int)));
    connect(stlink,SIGNAL(Reading(int)),this,SIGNAL(Reading(int)));
    connect(stlink,SIGNAL(Verification(bool)),this,SIGNAL(Verification(bool)));
}
