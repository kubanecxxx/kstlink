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

    //stlink version struct
    typedef struct
    {
        int stlink;
        int jtag;
        int swim;
    } version_t;

    //arm stm32 register set
    typedef struct
    {
        uint32_t r[16];
        //uint32_t s[32];
        uint32_t xpsr;
        uint32_t main_sp;
        uint32_t process_sp;
        uint32_t rw;
        uint32_t rw2;
        /*
        uint8_t control;
        uint8_t faultmask;
        uint8_t basepri;
        uint8_t primask;
        uint32_t fpscr;
        */
    } core_regs_t;

    //info
    int GetStlinkMode(QString * text = NULL);
    version_t GetStlinkVersion();
    bool IsCoreHalted();
    int GetCoreID();
    inline QString GetCoreStatus() {RefreshCoreStatus(); return CoreState;}

    //modes
    void ExitDFUMode();
    void EnterSWDMode();

    //core commands
    void CoreStop();
    void CoreRun();
    void CoreSingleStep();
    void SysReset();
    void WriteRegister(uint8_t reg_idx, uint32_t data);
    uint32_t ReadRegister(uint8_t reg_idx);
    void ReadAllRegisters(core_regs_t * regs);

    //memory commands
    void ReadRam(uint32_t address, uint32_t length, QByteArray & buffer) throw (QString);
    void WriteRam(uint32_t address, const QByteArray & buffer) throw (QString);

    //možná hodit do vyšši vrstvy, uvidime jak to pude
    void ReadFlash(uint32_t address, uint32_t length, QByteArray & buffer);
    void WriteFlash(uint32_t address, QByteArray & buffer);
    
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
