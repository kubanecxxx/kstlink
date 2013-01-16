#ifndef QSTLINK_H
#define QSTLINK_H

#include <QObject>
#include "qlibusb.h"
#include "include.h"
#include "QByteArray"
#include "stm100.h"
#include <QTimer>

class QStLink : public QObject
{
    Q_OBJECT
public:
    explicit QStLink(QObject *parent, const QByteArray &mcu);

    //stlink version struct
    typedef struct
    {
        int stlink;
        int jtag;
        int swim;
    } version_t;

    typedef struct
    {
        QString Mode;
        QString CoreState;
        version_t stlinkVersion;
        int coreID;
        int chipID;
    } stlink_properties_t;

    //info
    int GetStlinkMode(QString * text = NULL);
    version_t GetStlinkVersion();
    bool IsCoreHalted();
    int GetCoreID();
    inline int GetChipID(){return StProperties.chipID;}
    inline QString GetCoreStatus() {RefreshCoreStatus(); return StProperties.CoreState;}
    inline stlink_properties_t GetState(){return StProperties;}
    inline int GetBreakpointCount() {return breaks.count();}
    inline QString GetMcuName() {return Name;}

    //core commands
    void CoreStop();
    void CoreRun();
    void CoreSingleStep();
    void SysReset();
    void WriteRegister(uint8_t reg_idx, uint32_t data);
    uint32_t ReadRegister(uint8_t reg_idx);

    //memory commands
    void ReadRam(uint32_t address, uint32_t length, QByteArray & buffer);
    void WriteRam(uint32_t address, const QByteArray & buffer) throw (QString);

    void ReadAllRegisters(uint32_t * regs)
    {
        stm->ReadAllRegisters(regs);
    }

    uint32_t ReadMemoryWord(uint32_t address);
    uint32_t ReadMemoryRegister(volatile uint32_t * reg);
    void WriteRamRegister(volatile uint32_t * reg,uint32_t val);
    void WriteRamWord(uint32_t address, uint32_t data);
    void WriteRamByte(uint32_t address, uint8_t data);
    void WriteRamHalfWord(uint32_t address, uint16_t data);

    bool FlashVerify(const QByteArray & data);

    inline void FlashClear(uint32_t address, uint32_t length) throw (QString)
    {
        stm->EraseRange(address,length);
    }
    inline void FlashMassClear() throw (QString)
    {
        stm->EraseMass();
    }
    inline void FlashWrite(uint32_t address, const QByteArray & data) throw (QString)
    {
        stm->WriteFlash(address,data);
    }
    bool BreakpointWrite(uint32_t address);
    bool BreakpointRemove(uint32_t address);
    void BreakpointRemoveAll();

    QByteArray GetMapFile();

signals:
    void Erasing(int percent);
    void Flashing(int percent);
    void Reading(int percent);
    void CoreHalted(uint32_t address);
    void CoreRunning();
    void Verification(bool ok);

private slots:
    void timeout (void);

private:    
    void ReadAllRegisters(void * regs, int size);

    QLibusb * usb;

    QTimer & timer;
    QString Name;

    //modes
    void ExitDFUMode();
    void EnterSWDMode();

    void FillArrayEndian32(QByteArray & array, uint32_t data);
    void FillArrayEndian16(QByteArray & array, uint16_t data);

    uint32_t ReadUint32(const QByteArray & array);
    uint16_t ReadUint16(const QByteArray & array);

    stlink_properties_t StProperties;

    void Command (const QByteArray & txbuf);
    void Command (const QByteArray &txbuf , QByteArray & rxbuf, int rxsize);

    void CommandDebug(QByteArray & txbuf);
    void CommandDebug (QByteArray &txbuf , QByteArray & rxbuf, int rxsize);

    void RefreshCoreStatus();

    void ReadRam32(uint32_t address, uint16_t length,QByteArray &buffer);
    void WriteRam32(uint32_t address,const QByteArray & data);
    void WriteRam8(uint32_t address, const QByteArray & data);

    void ErasingProgress(int percent){emit Erasing(percent);}
    void ProgrammingProcess(int percent) {emit Flashing(percent);}

    stm100 * stm;

    friend class stm100;
private:
    void EnableFPB();
    typedef struct
    {
        bool active;
        uint32_t address;
    } break_t;

    void CodeComparatorCount_();
    void LiteralComparatorCount_();
    int GetFreeBreakpoint() const;

    int lit_count;
    QVector<break_t> breaks;

     bool IsFreeBreakpoint() const;

     QByteArray MapFile;
};

#endif // QSTLINK_H
