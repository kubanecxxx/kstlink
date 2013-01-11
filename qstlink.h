#ifndef QSTLINK_H
#define QSTLINK_H

#include <QObject>
#include "qlibusb.h"
#include "include.h"
#include "QByteArray"

class QStLink : public QObject
{
    Q_OBJECT
public:
    explicit QStLink(QObject *parent = 0);
    
    typedef struct
    {
        int stlink;
        int jtag;
        int swim;
    } version_t;


    //api
    int GetStlinkMode(QString * text = NULL);
    version_t GetStlinkVersion();
    bool IsCoreHalted();
    void ExitDFUMode();
    void EnterSWDMode();
    int GetCoreID();
    void CoreStop();
    void CoreRun();
    inline QString GetCoreStatus() {RefreshCoreStatus(); return CoreState;}

    void ReadRam(uint32_t address, uint32_t length, QByteArray & buffer);
    void WriteRam(uint32_t address, const QByteArray & buffer) throw (QString);

    void ReadFlash(uint32_t address, uint32_t length, QByteArray & buffer);
    void WriteFlash(uint32_t address, QByteArray & buffer);

    /*
     * writetoram(adresa,co)
     * readram(adresa,kolik)
     * readflash(adresa,kolik) - asi stejně jak readram
     * zápis do flaš přes loader - umřu
     */


signals:
    
public slots:
    
private:
    QString Mode;
    QString CoreState;
    version_t version;
    int coreID;

    void Command (const QByteArray & txbuf);
    void Command (const QByteArray &txbuf , QByteArray & rxbuf, int rxsize);

    void CommandDebug(QByteArray & txbuf);
    void CommandDebug (QByteArray &txbuf , QByteArray & rxbuf, int rxsize);
    QLibusb * usb;

    void RefreshCoreStatus();
    void FillArrayEndian32(QByteArray & array, uint32_t data);
    void FillArrayEndian16(QByteArray & array, uint16_t data);

    void ReadRam32(uint32_t address, uint16_t length,QByteArray &buffer);
    void WriteRam32(uint32_t address,const QByteArray & data);
    void WriteRam8(uint32_t address, const QByteArray & data);
};

#endif // QSTLINK_H
