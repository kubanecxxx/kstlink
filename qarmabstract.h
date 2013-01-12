#ifndef QARMABSTRACT_H
#define QARMABSTRACT_H

#include <QObject>
#include <inttypes.h>
#include "qstlink.h"

/**
 * @brief prescribes interface
 */
class QArmAbstract: public QStLink
{
    Q_OBJECT
public:   
    QArmAbstract(QObject * parent):
        QStLink(parent)
    {
    }

    virtual void BreakpointWrite(uint32_t address) throw (QString) = 0;
    virtual void BreakpointRemove(uint32_t address) throw (QString) = 0;

    virtual void FlashClear(uint32_t address, uint32_t length) throw (QString) = 0;
    virtual void FlashMassClear() throw (QString) = 0;
    virtual void FlashWrite(uint32_t address, const QByteArray & data) throw (QString) = 0;



signals:
    //break reached
    void breakpint(void);

};

#endif // QARMABSTRACT_H
