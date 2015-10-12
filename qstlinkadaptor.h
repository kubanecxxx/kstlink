#ifndef MAINAPPLICATIONADAPTOR_H
#define MAINAPPLICATIONADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <stdint.h>

class QStlinkAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kubanec.kstlink.stlink")

public:
    QStlinkAdaptor(QObject * parent);

private:


Q_SIGNALS:
    void Erasing(int percent);
    void Flashing(int percent);
    void Reading(int percent);
    void CoreHalted(quint32 address);
    void CoreHalted();
    void CoreRunning();
    void Verification(bool ok);
    void CommunicationFailed();
    void CoreResetRequested();


public slots:
    virtual quint32 GetCoreID(void) = 0;
    virtual int GetStlinkMode(QString * text = NULL) = 0;
    virtual bool IsCoreHalted() = 0;
    virtual int GetChipID() = 0;
    virtual QString GetCoreStatus()= 0;
    virtual int GetBreakpointCount() = 0;
    virtual QString GetMcuName() = 0;

    //core commands
    virtual void CoreStop() = 0;
    virtual void CoreRun() = 0;
    virtual void CoreSingleStep() = 0;
    virtual void SysReset() = 0;
    //virtual void WriteRegister(quint8 reg_idx, quint32 data) = 0;
    //virtual quint32 ReadRegister(quint8 reg_idx) = 0;
    virtual QString GetModeString() = 0;

    //ram memory commands
    virtual quint32 ReadMemoryWord(quint32 address) = 0;
    virtual void WriteRamWord(quint32 address, quint32 data) = 0;
    virtual void WriteRamByte(quint32 address, quint8 data) = 0;
    virtual void WriteRamHalfWord(quint32 address, quint16 data) = 0;

    //flash memory commands
    virtual void FlashMassClear() throw (QString) = 0;
    //virtual void FlashWrite(uint32_t address, const QByteArray & data) throw (QString)

    //debug commands
    virtual quint32 GetCycleCounter() = 0;

};

#endif // MAINAPPLICATIONADAPTOR_H
