#ifndef CM3DEBUGREGS_H
#define CM3DEBUGREGS_H

#include <QObject>
#include <inttypes.h>
#include <QFlags>

#define address(y,zz) Base().y + offsetof(cm3DebugRegs::y##_Type,zz)

class QStLink;
class QTimer;


class cm3DebugRegs
{
public:
    explicit cm3DebugRegs(QStLink * stlink);


    typedef struct
    {
        quint32 SCS_BASE;   //internal use only
        quint32 DWT;
        quint32 FPB;
        quint32 ITM;
        quint32 TPI;
        quint32 ETM;
        quint32 NVIC;
        quint32 SCB;
        quint32 CoreDebug;
    } debugBaseAddress_t;

    const debugBaseAddress_t & Base() const {return d;}

    typedef enum {FPB = 0x1, ITM = 0x2, TPI = 0x4, CORE = 0x8} debug_regs_t;
    Q_DECLARE_FLAGS(debug_regs, debug_regs_t)
    Q_FLAGS(debug_regs)

    void printDebugRegisters(debug_regs flags = TPI);
    QString printFPBRegisters();
    QString printITMRegisters();
    QString printTPIRegisters();
    QString printCoreDebugRegs();

private:
    debugBaseAddress_t d;
    static const quint32 romBase;
    static quint32 getBase(quint32 offset);
    QString printTable(void * alt,const void * neu, size_t size,
                              const QVector<quint32> & vct, const QStringList & lst);
    QStLink * stl;



public:
    #include "core_cm3.h"

    typedef struct
    {
        uint32_t CTRL;
        uint32_t REMAP;
        uint32_t COMP0;
        uint32_t COMP1;
        uint32_t COMP2;
        uint32_t COMP3;
        uint32_t COMP4;
        uint32_t COMP5;
        uint32_t COMP6;
        uint32_t COMP7;
        uint32_t RES[1004];
        uint32_t PID4;
        uint32_t PID5;
        uint32_t PID6;
        uint32_t PID7;
        uint32_t PID0;
        uint32_t PID1;
        uint32_t PID2;
        uint32_t PID3;
    } FPB_Type;

private:
    typedef union
    {
        FPB_Type s;
        uint32_t p[10];
    } FPB_u_t;

    TPI_Type tpi_alt;
    ITM_Type itm_alt;
    FPB_u_t fpb_alt;
    CoreDebug_Type cdt_alt;
    void systemPrint(const QString & data, int console);

    static void append (QString & data, const QString & name, quint32 num, bool wrap = true);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(cm3DebugRegs::debug_regs)

#endif // CM3DEBUGREGS_H
