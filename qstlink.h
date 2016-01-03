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
#include <QBuffer>


typedef enum {XPSR = 16, SP = 13,LR = 14,PC=15, MSP = 17, PSP = 18 , CFBP = 20 , PRIMASK=26,BASEPRI= 27,
             FAULTMASK = 28, CONTROL = 29} reg_idx_t;
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
       bool traceRequestEnabled;

       trace():
           traceEnabled(false),
           mcuConfigured(false),
           coreFrequency(0),
           swoFrequency(2e6),
           bufferSize(1024),
           traceRequestEnabled(false)
       {
       }
    } trace;

    Q_OBJECT
public:
    explicit QStLink(QObject *parent, const QByteArray &mcu,bool stop = false, bool trace_enable = false, long freq = 0);

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
    const version_t GetStlinkVersion();
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
    void AssertResetPin(bool high);
    mode_t GetMode();
    QString GetModeString() {return mode_list[static_cast<int>(GetMode())];}

    //register commands
    QVector<quint32> ReadAllRegisters32();
    uint32_t ReadRegister(uint8_t reg_idx);
    void WriteRegister(uint8_t reg_idx, uint32_t data);
    void UnstackContext(QVector<quint32> & context, uint32_t sp);
    QVector<quint32> MergeContexts(const QVector<quint32> & registers, uint32_t sp);


    //memory commands
    void ReadRam(uint32_t address, uint32_t length, QByteArray & buffer, bool verify = false);
    void WriteRam(uint32_t address, const QByteArray & buffer) throw (QString);

    uint32_t ReadMemoryWord(uint32_t address);
    uint32_t ReadMemoryRegister(volatile uint32_t * reg);
    void WriteRamRegister(volatile uint32_t * reg,uint32_t val);
    void WriteRamWord(uint32_t address, uint32_t data);
    void WriteRamByte(uint32_t address, uint8_t data);
    void WriteRamHalfWord(uint32_t address, uint16_t data);

    //trace commands
    bool traceEnable() ;
    bool traceDisable() ;
    bool traceConfigureMCU();
    bool traceUnconfigureMCU();
    bool traceRead(QByteArray & data);
    void traceFormatData(const QByteArray & data);

    bool FlashVerify(const QByteArray & data, quint32 offset);

    inline void FlashClear(uint32_t address, uint32_t length)
    {
        emit ErasingActive(true);
        stm->EraseRange(address,length);
        emit ErasingActive(false);
    }
    inline void FlashMassClear() throw (QString)
    {
        emit ErasingActive(true);
        stm->EraseMass();
        emit ErasingActive(false);
    }
    inline void FlashWrite(quint32 address, const QByteArray & data, bool verify )
    {
        emit FlashingActive(true);
        stm->WriteFlash(address,data);

        if (verify)
            FlashVerify(data,address - FLASH_BASE);
        emit FlashingActive(false);
    }

    bool BreakpointWrite(uint32_t address);
    bool BreakpointRemove(uint32_t address);
    void BreakpointRemoveAll();

    QByteArray GetMapFile();

    //aux functions
    static void Vector32toByteArray(QByteArray & dest, const QVector<quint32> & input);
    quint32 GetCycleCounter();
    static uint32_t ReadUint32(const QByteArray & array);
    static uint16_t ReadUint16(const QByteArray & array);


private slots:
    void timeout (void);

private:    
    QByteArray RefreshRegisters();
    QLibusb * usb;

    QTimer & timer;
    QString Name;

    //modes
    void ExitDFUMode();
    void EnterSWDMode();

    void FillArrayEndian32(QByteArray & array, uint32_t data);
    void FillArrayEndian16(QByteArray & array, uint16_t data);


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
    void ErasingActiveF(bool val) {emit ErasingActive(val);}

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
     cm3DebugRegs * debug;

     QMap<quint8,QByteArray *> trace_buffer;



};



#endif // QSTLINK_H
