#ifndef QSTLINK_H
#define QSTLINK_H

#include <QObject>
#include "qlibusb.h"
#include "include.h"
#include "QByteArray"
#include "stm100.h"
#include <QTimer>
#include "qstlinkadaptor.h"
#include <QStringList>
#include "stmabstract.h"

class cm3Regs
{
public:
    typedef struct
    {
        quint64 r[13];
        quint64 pc;
        quint64 lr;
        quint64 sp;
        //r15 - program counter
        quint64 xpsr;
        quint64 control_faultmask_basipri_primask;
        quint8 control;
        quint8 faultmask;
        quint8 basepri;
        quint8 primask;
    } cm3_regs_t;

    void fill(const QVector<quint64> & raw);
private:
    cm3_regs_t data;
};

typedef enum {XPSR = 16, SP = 13,LR = 14,PC=15, MSP = 17, PSP = 18 , CFBP = 20} reg_idx_t;
//MSP - handler stack, PSP - thread stack

class cm3DebugRegs;
class QStLink : public QStlinkAdaptor
{
    //trase
    struct trace
    {
       bool traceEnabled;
       bool mcuConfigured;
       quint32 coreFrequency;
       quint32 swoFrequency;
       quint32 bufferSize;

       trace():
           traceEnabled(false),
           mcuConfigured(false),
           coreFrequency(0),
           swoFrequency(2e6),
           bufferSize(1024)
       {
       }
    } trace;

    Q_OBJECT
public:
    explicit QStLink(QObject *parent, const QByteArray &mcu,bool stop = false);

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

    typedef enum {Thread, Handler, Unknown} mode_t;
    QStringList mode_list;

    //info
    int GetStlinkMode(QString * text = NULL);
    const version_t & GetStlinkVersion();
    bool IsCoreHalted();
    quint32 GetCoreID();
    inline int GetChipID(){return StProperties.chipID;}
    inline QString  GetCoreStatus() {RefreshCoreStatus(); return StProperties.CoreState;}
    inline const stlink_properties_t & GetState(){return StProperties;}
    inline int GetBreakpointCount() {return breaks.count();}
    inline QString GetMcuName() {return Name;}

    //core commands
    void CoreStop();
    void CoreRun();
    void CoreSingleStep();
    void SysReset();
    mode_t GetMode();
    QString GetModeString() {return mode_list[static_cast<int>(GetMode())];}

    //register commands
    QVector<quint64> ReadAllRegisters64(mode_t context,bool cached = false);
    QVector<quint32> ReadAllRegisters32(mode_t context, bool cached = false);
    void WriteRegister(mode_t context, uint8_t reg_idx, uint32_t data, bool cached = false);
    uint32_t ReadRegister(mode_t context, uint8_t reg_idx, bool cached = false);


    //memory commands
    void ReadRam(uint32_t address, uint32_t length, QByteArray & buffer);
    void WriteRam(uint32_t address, const QByteArray & buffer) throw (QString);

    uint32_t ReadMemoryWord(uint32_t address);
    uint32_t ReadMemoryRegister(volatile uint32_t * reg);
    void WriteRamRegister(volatile uint32_t * reg,uint32_t val);
    void WriteRamWord(uint32_t address, uint32_t data);
    void WriteRamByte(uint32_t address, uint8_t data);
    void WriteRamHalfWord(uint32_t address, uint16_t data);

    //trace commands
    void traceSetCoreFrequency(quint32 freq) {trace.coreFrequency = freq;}
    void traceEnable() throw(QString);
    void traceDisable() throw(QString);
    void traceConfigureMCU();
    void traceUnconfigureMCU();
    void traceRead(QByteArray & data);

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

    //aux functions
    static void Vector32toByteArray(QByteArray & dest, const QVector<quint32> & input);

private slots:
    void timeout (void);

private:    
    void RefreshRegisters();

    void WriteRegister(uint8_t reg_idx, uint32_t data);
    uint32_t ReadRegister(uint8_t reg_idx);

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

    stmAbstract * stm;

    friend class stmAbstract;
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

     mode_t ThreadMode;

     const int registerCount;
     const int registerSize;
     const int rS;

     //registers and contexts
     QVector<quint64> regsRaw;
     QVector<quint64> regsThread;
     QVector<quint64> regsHandler;
     QMap<mode_t, QVector<quint64>* > contexts;
     cm3Regs cm3regs_raw,cm3regs_thread,cm3regs_handler;
     cm3DebugRegs * debug;


};



#endif // QSTLINK_H
