#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "qstlink.h"
#include <QObject>
#ifdef KSTLINK_DBUS
#include <QDBusMessage>
#include <QDBusConnection>
#endif

#include <QList>
#include <QVariant>

class Communication : public QObject
{
    Q_OBJECT
public:
    explicit Communication(  QObject * parent);

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
        virtual void FlashMassClear() = 0;
        virtual void FlashWrite(uint32_t address, const QByteArray & data) = 0;

        //debug commands
        virtual quint32 GetCycleCounter() = 0;
};



#ifdef KSTLINK_DBUS
class DBus: public Communication
{
    Q_OBJECT
public:
    explicit DBus(QDBusConnection * con, QObject * parent);

private slots:
        void ch();

public slots:
     quint32 GetCoreID(void) ;
     int GetStlinkMode(QString * text = NULL) ;
     bool IsCoreHalted() ;
     int GetChipID() ;
     QString GetCoreStatus();
     int GetBreakpointCount() ;
     QString GetMcuName() ;
    //core commands
     void CoreStop();
     void CoreRun();
     void CoreSingleStep() ;
     void SysReset();
    //virtual void WriteRegister(quint8 reg_idx, quint32 data) = 0;
    //virtual quint32 ReadRegister(quint8 reg_idx) = 0;
     QString GetModeString() ;

    //ram memory commands
     quint32 ReadMemoryWord(quint32 address) ;
     void WriteRamWord(quint32 address, quint32 data) ;
     void WriteRamByte(quint32 address, quint8 data) ;
     void WriteRamHalfWord(quint32 address, quint16 data) ;

    //flash memory commands
    void FlashMassClear();
    void FlashWrite(uint32_t address, const QByteArray & data);

    //debug commands
     quint32 GetCycleCounter() ;

private:
    QDBusMessage call(const QString & method, const QList<QVariant> & args = QList<QVariant>());
    bool connect(const QString & signal, const char * slot);
    QString service ;
    QString path;
    QString interface;

    QDBusConnection * con;
};
#endif

 class direct: public Communication
{
    Q_OBJECT
    public:
        direct(QStLink * stlink ,QObject * parent);

    private:
        QStLink * link;

    public slots:
        quint32 GetCoreID(void) {return link->GetCoreID();}
        int GetStlinkMode(QString * text = NULL) {return link->GetStlinkMode(text);}
        bool IsCoreHalted() {return link->IsCoreHalted();}
        int GetChipID() {return link->GetChipID();}
        QString GetCoreStatus() {return link->GetCoreStatus();}
        int GetBreakpointCount() {return link->GetBreakpointCount();}
        QString GetMcuName() {return link->GetMcuName();}

        //core commands
        void CoreStop() {link->CoreStop();}
        void CoreRun() {link->CoreRun();}
        void CoreSingleStep() {link->CoreSingleStep();}
        void SysReset() {link->SysReset();}
        //virtual void WriteRegister(quint8 reg_idx, quint32 data) = 0;
        //virtual quint32 ReadRegister(quint8 reg_idx) = 0;
        QString GetModeString() {return link->GetModeString();}

        //ram memory commands
        quint32 ReadMemoryWord(quint32 address) {return link->ReadMemoryWord(address);}
        void WriteRamWord(quint32 address, quint32 data) {link->WriteRamWord(address,data);}
        void WriteRamByte(quint32 address, quint8 data) {link->WriteRamByte(address,data);}
        void WriteRamHalfWord(quint32 address, quint16 data) {link->WriteRamHalfWord(address,data);}

        //flash memory commands
        void FlashMassClear() {link->FlashMassClear();}
        virtual void FlashWrite(uint32_t address, const QByteArray & data)  {link->FlashWrite(address,data);}
        //virtual void FlashWrite(uint32_t address, const QByteArray & data) throw (QString)

        //debug commands
        quint32 GetCycleCounter() {return link->GetCycleCounter();}


 };

#endif // COMMUNICATION_H
