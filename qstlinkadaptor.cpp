#include "qstlinkadaptor.h"


QStlinkAdaptor::QStlinkAdaptor(QObject * parent)
#ifdef KSTLINK_DBUS
    : QDBusAbstractAdaptor(parent)
    #else
    : QObject(parent)
#endif
{
#ifdef KSTLINK_DBUS
    setAutoRelaySignals(true);
#endif
}

