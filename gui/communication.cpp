#include "communication.h"

#ifdef KSLTINK_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#define connectS(method,slot) connection->sessionBus().connect(service,path,interface,method,this,slot);
#endif

Communication::Communication(QObject *parent) : QObject(parent)
{

}


#ifdef KSTLINK_DBUS
dbus:dbus()
{
    service = "org.kubanec.kstlink";
    path = "/qstlink";
    //interface = "org.kubanec.kstlink.stlink";
    interface = "org.qtproject.Qt.QStLink";

    if (con)
        ok = connectS("CoreHalted",SLOT(CoreHalted(quint32)));
        connectS("CommunicationFailed",SLOT(CommunicationFailed()));
        connectS("CoreRunning",SLOT(CoreRunning()));
        connectS("Verification",SLOT(Verification(bool)));
        connectS("Reading",SLOT(Reading(int)));
        connectS("Flashing",SLOT(Flashing(int)));
        connectS("Erasing",SLOT(Erasing(int)));
        connectS("CoreResetRequested",SLOT(ResetRequested()));
        qDebug() << ok;
}



QDBusMessage Communication::DbusCallMethod(const QString &method)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service,path,interface,method);
    //QDBusConnection con = QDBusConnection::connectToBus(QDBusConnection::SessionBus,"dbus");
    msg = this->con->call(msg,QDBus::Block,300);

    return msg;
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
