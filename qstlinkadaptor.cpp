#include "qstlinkadaptor.h"

QStlinkAdaptor::QStlinkAdaptor(QObject * parent)
: QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}
