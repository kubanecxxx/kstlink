#include "qstlink.h"
#include "QDebug"
#include <QtEndian>
#include <QTime>
#include "stlinkCommands.h"
#include "armConstants.h"
#include "QFile"
#include "chips.h"
#include "stm407.h"
#include "cm3debugregs.h"

#define writeDebugWord(STRUCT,MEMBER,data) WriteRamWord(debug->address(STRUCT,MEMBER),data)
#define readDebugWord(STRUCT,MEMBER) (ReadMemoryWord(debug->address(STRUCT,MEMBER)))

QStLink::QStLink(QObject *parent, const QByteArray & mcu, bool stop) :
    QStlinkAdaptor(parent),
    usb(new QLibusb(this)),
    timer(*new QTimer(this)),
    registerCount(21),
    registerSize(4),
    rS(registerSize * registerCount),
    regsRaw(registerCount),
    regsThread(registerCount),
    regsHandler(registerCount)
{
    /*************************
     * init Properties struct;
     ************************/

    GetStlinkVersion();
    EnterSWDMode();
    if (stop)
        CoreStop();

    GetStlinkMode();
    GetCoreID();
    RefreshCoreStatus();

    usleep(1000);
    uint32_t id;
    uint32_t temp =  ReadMemoryWord(REG_CPUID);
    id = temp & 0xFFF;
    // CM4 rev0 fix
    if ((id == 0x411) && (StProperties.coreID == CORE_M4_R0))
        id = STM32_CHIPID_F4;
    StProperties.chipID = id;

    /***********************
     * choose chip
     **********************/
    Chips chip(id,*this);
    stm = chip.GetStm();

    QFile fil(":/xml/map.xml");
    fil.open(QFile::ReadOnly);
    MapFile = fil.readAll();
    QByteArray templa = "<memory type=\"flash\" start=\"0x08000000\" length=\"0x20000\"/>";
    QByteArray repl =  "<memory type=\"flash\" start=\"0x08000000\" length=\"0x";
    repl.append(QString("%1\">").arg(chip.GetFlashSize(),0,16));
    MapFile.replace(templa,repl);

    templa = "<memory type=\"ram\" start=\"0x20000000\" length=\"0x2000\"/>";
    repl =  "<memory type=\"ram\" start=\"0x20000000\" length=\"0x";
    repl.append(QString("%1\"/>").arg(chip.GetRamSize(),0,16));
    MapFile.replace(templa,repl);
    Name = chip.GetChipName();

    debug = new cm3DebugRegs(this);

    /***************************
     * init breakpoints
     ***************************/
    const quint32 FP_CTRL =  debug->address(FPB,CTRL);
    temp = ReadMemoryWord(FP_CTRL);
    temp = (temp >> 4 & 0xF);
    breaks.resize(temp); temp = ReadMemoryWord(FP_CTRL);
    temp = (temp >> 8) & 0xF;
    lit_count = temp;
    BreakpointRemoveAll();

    /***************************
     * init timer for getting
     * core state periodically
     **************************/
    timer.start(100);
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    /*
    if (stop)
        CoreRun();
*/
    //FlashClear(FLASH_BASE, FLASH_BASE + 1024);
#if 0
    /*
     * benchmark
     * speeds of read and write are ~50kbytes/s
     */
    QByteArray tx;
    int i;
    for (i = 0 ; i < 1024*10; i++)
        tx.append(i);
    EXECUTION_TIME(WriteRam(0x20000000,tx); , wr);
    float speed = 1.0 *i / wr_result;

    QByteArray rx;

    EXECUTION_TIME(ReadRam(0x20000000,10000,rx); , read);
    float speed2 = 1.0 *10000 / read_result;

    EXECUTION_TIME(ReadRam(0x08000000,10000,rx); , readFl);
    float speed3 = 1.0 *10000 / readFl_result;

    rx.clear();
#endif

    mode_list << "Thread" << "Handler"<< "Unknown";

    //prepare trace buffer
    for (int i = 0 ; i < 32; i++)
    {
        QBuffer * buf;
        trace_buffer.insert(i, buf =  new QBuffer);
        buf->open(QIODevice::ReadWrite);
    }

   // traceSetCoreFrequency(168e6);
   // traceEnable();
   // traceConfigureMCU();
}

bool QStLink::traceConfigureMCU()
{
    if (!trace.traceEnabled)
        return false;
    Q_ASSERT(trace.coreFrequency);
    quint32 t = readDebugWord(CoreDebug,DHCSR);
    t &= 0x0000FFFF;
    t |= 0xa05f0001; // key + debug_en
    writeDebugWord(CoreDebug,DHCSR,t);
    //tracen - enable TPIU, DWT, ITM, ETM
    writeDebugWord(CoreDebug,DEMCR,CoreDebug_DEMCR_TRCENA_Msk);

    //DWT
    //WriteRamWord(debug->address(DWT,CTRL),0x400003FE);

    //TPIU setup
    writeDebugWord(TPI,FFCR,0x0100);    //disabled formater
    writeDebugWord(TPI,SPPR,0x02);      //NRZ SWO output
    writeDebugWord(TPI,ACPR,(trace.coreFrequency/trace.swoFrequency) -1);
    //stl->WriteRamWord(a = address(TPI,ACPR),1);

    // STM register DBGMCU_CR to enable SWO pin PB3
    t = ReadMemoryWord(DBGMCU_CR);
    t |= 0x20;
    WriteRamWord(DBGMCU_CR,t);

    //ITM regs
    writeDebugWord(ITM,LAR,0xC5ACCE55); //key
    writeDebugWord(ITM,TCR,0x0001000D);
    writeDebugWord(ITM,TER,1);          //enable channel1
    writeDebugWord(ITM,TPR,0xf);        //all channels from user mode
    writeDebugWord(ITM,IMCR,0);
    writeDebugWord(ITM,IWR,1);          //integration

    writeDebugWord(TPI,ITCTRL,0);

    trace.mcuConfigured = true;

    return true;
}

bool QStLink::traceUnconfigureMCU()
{
    // STM register DBGMCU_CR to release PB3 pin
    quint32 t = ReadMemoryWord(DBGMCU_CR);
    t &= ~0x20;
    WriteRamWord(DBGMCU_CR,t);

    //disable whole trace mechanism (DWT,ITM,TPI,ETM)
    t = readDebugWord(CoreDebug,DEMCR);
    t &= ~CoreDebug_DEMCR_TRCENA_Msk;
    writeDebugWord(CoreDebug,DEMCR,CoreDebug_DEMCR_TRCENA_Msk);

    trace.mcuConfigured = false;

    return true;
}

bool QStLink::traceRead(QByteArray &data)
{
    if (!(trace.traceEnabled && trace.mcuConfigured))
        return false;

    QByteArray tr,rt;
    tr.clear();
    tr.append(STLINK_DEBUG_APIV2_GET_TRACE_NB);
    CommandDebug(tr,rt,2);
    size_t z = rt.at(0) | rt.at(1) << 1;

    if (z)
    {
        data = usb->ReadTrace();
        return true;
    }

    return false;
}

bool QStLink::traceEnable()
{
    if (!trace.coreFrequency)
        return false;
    Q_ASSERT(trace.coreFrequency);

    int size = trace.bufferSize;
    int freq = trace.swoFrequency;
    QByteArray tr,r;
    tr.append(STLINK_DEBUG_APIV2_START_TRACE_RX);
    tr.append(size);
    tr.append(size >> 8);
    tr.append(freq);
    tr.append(freq >> 8);
    tr.append(freq >> 16);
    tr.append(freq >> 24);
    CommandDebug(tr,r,2);

    trace.traceEnabled = true;
    return false;
}

bool QStLink::traceDisable()
{
    if (trace.mcuConfigured)
        traceUnconfigureMCU();

    QByteArray tr,r;
    tr.append(STLINK_DEBUG_APIV2_STOP_TRACE_RX);
    CommandDebug(tr,r,2);

    trace.traceEnabled = false;
    return true;
}

void QStLink::traceFormatData(const QByteArray & data)
{
    for (int i = 0 ; i < data.count() ; i+= 2)
    {
        quint8 stream = data.at(i) - 1;
        quint8 character = data.at(i + 1);

        bool ok = trace_buffer.value(stream)->putChar(character);
        asm("nop");
    }
}

QVector<quint32> QStLink::MergeContexts(const QVector<quint32> &registers, uint32_t sp)
{
    QVector<quint32> ret(registers);
    QVector<quint32> ram;

    UnstackContext(ram, sp);

    QVector<int> indexes;
    indexes << 0 << 1 << 2 << 3 << 12 << 14 << 15;
    //indexes << 16;
    int i = 0;

    foreach(int idx, indexes)
    {
        ret[idx] = ram.at(i);
        i++;
    }

    return ret;
}

void QStLink::UnstackContext(QVector<quint32> &context, uint32_t sp)
{
    QByteArray buf;
    ReadRam32(sp,8*4,buf);

    context.clear();


    //R0, R1 ,R2, R3, R12, LR, PC, xPSR
    for (int i = 0 ; i < 8 ; i++)
    {
        QByteArray num = buf.mid(i*4,4);
        quint32 n = ReadUint32(num);
        context.push_back(n);
    }
}

// raw data
QByteArray QStLink::RefreshRegisters()
{
    BOTHER("Core read all registers");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_READALLREGS);
    CommandDebug(tx,rx,rS);

    return rx;
}


QVector<quint32> QStLink::ReadAllRegisters32()
{
    QByteArray rx = RefreshRegisters();

    QVector<quint32> t;
    for (int i = 0 ; i < qMin(rx.count() / 4 , 16 ); i+= 1)
    {
        quint32 reg;
        QByteArray num = rx.mid(i*4,4);
        memcpy(&reg,num.constData(), 4);
        t.append(reg);
    }

    return t;
}

QStLink::mode_t QStLink::GetMode()
{
    uint32_t xpsr = ReadRegister(XPSR);
    xpsr &= 0xff;

    if (xpsr == 0)
        ThreadMode = Thread;
    else
        ThreadMode = Handler;

    return ThreadMode;
}

//timeout for read core status
void QStLink::timeout()
{
    uint32_t t =  ReadRegister(PC);
    if (t == 0xf)
    {
        emit CommunicationFailed();
        ERR("Communication Failed");
    }

    bool temp = IsCoreHalted();

    if (temp)
    {
        uint32_t addr  = ReadRegister(PC);
        emit CoreHalted(addr);
        emit CoreHalted();
    }
    else
    {
        emit CoreRunning();
    }

    QByteArray ar;

    if (traceRead(ar))
    {
        traceFormatData(ar);
        trace_buffer.value(0)->seek(0);
        QByteArray buf = trace_buffer.value(0)->readAll();
        qDebug() << buf;
    }

    asm("nop");
}

quint32 QStLink::GetCycleCounter()
{
    quint32 i = ReadMemoryWord(debug->address(DWT,CYCCNT));
    return i;
}

bool QStLink::BreakpointWrite(uint32_t address)
{
    if (!IsFreeBreakpoint())
        return false;

    int b = GetFreeBreakpoint();

    breaks[b].active = true;
    breaks[b].address = address;
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_SETFP);
    tx.append(b);
    FillArrayEndian32(tx,address);

    int i;
    if (address % 4 == 0)
        i = 0;
    else
        i = 1;

    tx.append(i);
    CommandDebug(tx,rx,2);

    return true;
}

//clear all breakpoints
void QStLink::BreakpointRemoveAll()
{
    for (int i = 0 ; i< breaks.count(); i++)
    {
        QByteArray tx,rx;
        tx.append(STLINK_DEBUG_CLEARFP);
        tx.append(i);
        CommandDebug(tx,rx,2);

        breaks[i].active = false;
        breaks[i].address = 0;
    }
}

bool QStLink::BreakpointRemove(uint32_t address)
{
    for  (int i = 0; i < breaks.count(); i++)
    {
        if (breaks[i].address == address && breaks[i].active)
        {
            QByteArray tx,rx;
            tx.append(STLINK_DEBUG_CLEARFP);
            tx.append(i);
            CommandDebug(tx,rx,2);
            breaks[i].active = false;

            return true;
        }
    }

    return false;
}

int QStLink::GetFreeBreakpoint() const
{
    for (int i = 0 ; i < breaks.count(); i++)
    {
        if (breaks[i].active == false)
            return i;
    }

    return -1;
}

bool QStLink::IsFreeBreakpoint() const
{
    if (GetFreeBreakpoint() == -1)
        return false;
    else
        return true;
}

QByteArray QStLink::GetMapFile()
{
    return MapFile;
}

/** ******************************************************
 * @brief QStLink::ReadRam
 * api reads ram any size
 * @param address start address
 * @param length data length in bytes
 * @param buffer output data buffer
 *********************************************************/
void QStLink::ReadRam(uint32_t address, uint32_t length, QByteArray & buffer)
{
    BOTHER("Read ram");

    int kolik  = 0;
    //fill to multiply 4
    while(length % 4)
    {
        length++;
        kolik++;
    }

    int temp = length;
    int size;
    //divide into smaller segments
    while(temp > 0)
    {
        if (temp > SEGMENT_SIZE)
        {
            size = SEGMENT_SIZE;
            temp -= SEGMENT_SIZE;
        }
        else
        {
            size = temp;
            temp = 0;
        }

        QByteArray array;
        ReadRam32(address,size,array);
        buffer.append(array);

        address += SEGMENT_SIZE;

        if (buffer.count() > 1024)
        {
            emit Reading(100*buffer.count() / length);
        }
    }

    //delete added bytes
    buffer.resize(buffer.count() - kolik);
}

/** ******************************************************
 * @brief QStLink::WriteRam
 * api, writes data, any size
 * @param address start address
 * @param buffer input data
 *********************************************************/
void QStLink::WriteRam(uint32_t address, const QByteArray & buffer) throw (QString)
{
    if (address < SRAM_BASE)
        throw(QString("WriteRam out of range"));

    QByteArray cpy(buffer);

    BOTHER("Write ram");

    while (cpy.count())
    {
        /*
         * dividing big data into smaller segments and write
         * to mcu by segments (teoretical maximum is 6k but
         * it doesn't work for me so smaller segment)
         */
        QByteArray segment = cpy.left(SEGMENT_SIZE);
        cpy.remove(0,SEGMENT_SIZE);
        uint32_t prev_address = address;

        /*
         * write segment
         */
        int len = segment.count();
        int words = len / 4;

        int len4 = words * 4;

        QByteArray tx;
        tx = segment.left(len4);
        if (tx.count())
            WriteRam32(address,tx);

        tx = segment.right(len - len4);
        address += len4;
        if (tx.count())
            WriteRam8(address,tx);

        address = prev_address + SEGMENT_SIZE;
    }
}

bool QStLink::FlashVerify(const QByteArray &data)
{
    QByteArray rx;
    ReadRam(FLASH_BASE,data.count(), rx);

    for (int i = 0 ; i < rx.count(); i++)
    {
        if (rx.at(i) != data.at(i))
        {
            emit Verification(false);
            return false;
        }
    }

    emit Verification(true);
    return true;
}

void QStLink::WriteRam32(uint32_t address,const QByteArray &data)
{
    int size = data.count();
    QByteArray tx;

    tx.append(STLINK_DEBUG_WRITEMEM_32BIT);
    FillArrayEndian32(tx,address);
    FillArrayEndian16(tx,size);

    CommandDebug(tx);
    Command(data);
}

void QStLink::WriteRam8(uint32_t address,const QByteArray &data)
{
    int size = data.count();
    QByteArray tx;

    tx.append(STLINK_DEBUG_WRITEMEM_8BIT);
    FillArrayEndian32(tx,address);
    FillArrayEndian16(tx,size);

    CommandDebug(tx);
    Command(data);
}

void QStLink::ReadRam32(uint32_t address, uint16_t length, QByteArray &buffer)
{
    QByteArray tx;

    //address is automatically aligned to multiply of 4
    //by stlink
    int temp = 0;
    while(address % 4)
    {
        address--;
        length++;
        temp++;
    }

    tx.append(STLINK_DEBUG_READMEM_32BIT);
    FillArrayEndian32(tx,address);
    FillArrayEndian16(tx,length);

    CommandDebug(tx,buffer,length);
    buffer.remove(0,temp);
}

void QStLink::CoreStop()
{
    BOTHER("Core stop");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_FORCEDEBUG);
    CommandDebug(tx,rx,2);

    RefreshCoreStatus();
}

void QStLink::CoreRun()
{
    BOTHER("Core run");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_RUNCORE);
    CommandDebug(tx,rx,2);

    RefreshCoreStatus();
}

void QStLink::CoreSingleStep()
{
    BOTHER("Core single step");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_STEPCORE);
    CommandDebug(tx,rx,2);

    RefreshCoreStatus();
}

void QStLink::WriteRegister(uint8_t reg_idx, uint32_t data)
{
    BOTHER("Core write register");

    if (reg_idx > CFBP)
        return;

    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_WRITEREG);
    tx.append(reg_idx);
    FillArrayEndian32(tx,data);
    CommandDebug(tx,rx,2);
}


uint32_t QStLink::ReadRegister(uint8_t reg_idx)
{
    uint32_t reg;
    //control, faultmask,basepri,primask
    if (reg_idx > 25)
    {
        reg_idx -= 26;
        reg_idx *= 8;
        quint32 temp = ReadRegister(CFBP);
        reg = (temp >> reg_idx) & 0xff;
        return reg;
    }

    BOTHER("Core read register");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_READREG);
    tx.append(reg_idx);
    CommandDebug(tx,rx,4);

    uint32_t val;
    memcpy(&val,rx.constData(),4);

    return val;
}

void QStLink::SysReset()
{
    BOTHER("System reset");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_RESETSYS);
    CommandDebug(tx,rx,2);

    RefreshCoreStatus();
    emit CoreResetRequested();
}

void QStLink::RefreshCoreStatus()
{
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_GETSTATUS);
    CommandDebug(tx,rx,2);

    unsigned char s = rx.at(0);
    QString CoreState;

    switch(s)
    {
    case STLINK_CORE_HALTED:
        CoreState = "Halted";
        break;
    case STLINK_CORE_RUNNING:
        CoreState = "Running";
        break;
    default:
        CoreState = "Unknown";
        break;
    }

    INFO("Core status " + CoreState);

    StProperties.CoreState = CoreState;
}

quint32 QStLink::GetCoreID()
{
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_READCOREID);
    CommandDebug(tx,rx,4);

    uint32_t coreID;
    memcpy(&coreID,rx.constData(),4);

    StProperties.coreID = coreID;

    INFO(QString("Core ID: 0x%1").arg(coreID,0,16));

    return coreID;
}

void QStLink::ExitDFUMode()
{
    QByteArray tx;
    tx.append(STLINK_DFU_COMMAND);
    tx.append(STLINK_DFU_EXIT);
    Command(tx);

    INFO("ST-Link dfu exit");
}

void QStLink::EnterSWDMode()
{
    ExitDFUMode();
    QByteArray tx;
    tx.append(STLINK_DEBUG_ENTER);
    tx.append(STLINK_DEBUG_ENTER_SWD);

    CommandDebug(tx);
    BOTHER("ST-Link enter SWD");
}

bool QStLink::IsCoreHalted()
{
    RefreshCoreStatus();
    return (StProperties.CoreState == "Halted");
}

int QStLink::GetStlinkMode(QString * text)
{
    QByteArray tx;
    QByteArray rx;

    tx.append(STLINK_GET_CURRENT_MODE);
    tx.append('\0');
    Command(tx,rx,2);

    int mode = rx.at(0);

    QString Mode;
    switch (mode)
    {
    case STLINK_DEV_DFU_MODE:
        Mode = "DFU";
        break;
    case STLINK_DEV_MASS_MODE:
        Mode = "MASS";
        break;
    case STLINK_DEV_DEBUG_MODE:
        Mode = "Debug";
        break;
    default:
        Mode = "Unknown";
        break;
    }

    if (text)
        *text = Mode;

    INFO("STlink mode " + Mode);

    StProperties.Mode = Mode;

    return mode;
}

const QStLink::version_t QStLink::GetStlinkVersion()
{
    QByteArray tx;
    QByteArray rx;
    version_t ver;

    tx.append(STLINK_GET_VERSION);
    Command(tx,rx,6);

    int b0 = rx.at(0);
    int b1 = rx.at(1);

    ver.stlink = (b0 & 0xf0) >> 4;
    ver.jtag = ((b0 & 0x0f) << 2) | ((b1 & 0xc0) >> 6);
    ver.swim = b1 & 0x3f;

    StProperties.stlinkVersion = ver;

    INFO("STlink version " + rx.toHex() );

    return ver;
}

void QStLink::CommandDebug(QByteArray &txbuf)
{

    txbuf.prepend(STLINK_DEBUG_COMMAND);
/*
    while(buf.count() < 16)
        buf.append('\0');
*/
    Command(txbuf);
}

void QStLink::CommandDebug(QByteArray &txbuf, QByteArray &rxbuf, int rxsize)
{
    CommandDebug(txbuf);

    try
    {
        rxbuf = usb->Read(rxsize);
    }
    catch ( QString  neco)
    {
        //cannot happen - bad input values from programmer
        ERR(neco);
    }
}

void QStLink::Command(const QByteArray &txbuf)
{
    int transf;
    try
    {
    transf = usb->Write(txbuf);
    }
    catch ( QString  neco)
    {
        ERR(neco);
    }

    if (txbuf.count() != transf)
    {
        //@todo problem
    }
}

void QStLink::Command(const QByteArray &txbuf, QByteArray &rxbuf, int rxsize)
{
    Command(txbuf);
    try
    {
        rxbuf = usb->Read(rxsize);
    }
    catch ( QString  neco)
    {
        //cannot happen - bad input values from programmer
        ERR(neco);
    }
}

void QStLink::FillArrayEndian16(QByteArray &array, uint16_t data)
{
    array.append(data & 0xFF);
    array.append(data >> 8);
}

void QStLink::FillArrayEndian32(QByteArray &array, uint32_t data)
{
    array.append(data & 0xFF);
    array.append((data >> 8) & 0xFF );
    array.append((data >> 16)&  0xFF );
    array.append((data >> 24)&  0xFF );
}

uint32_t QStLink::ReadUint32(const QByteArray &array)
{
    uint32_t ret;

    memcpy(&ret,array.constData(),4);
    return ret;
}

uint16_t QStLink::ReadUint16(const QByteArray &array)
{
    uint16_t ret;

    memcpy(&ret,array.constData(),2);
    return ret;
}

void QStLink::WriteRamRegister(volatile uint32_t *reg,uint32_t val)
{
    uint32_t temp = (uint64_t) reg;
    WriteRamWord(temp,val);
}

uint32_t QStLink::ReadMemoryRegister(volatile uint32_t * reg)
{
    uint32_t temp = (uint64_t) reg;

    return ReadMemoryWord(temp);
}

uint32_t QStLink::ReadMemoryWord(uint32_t address)
{
    QByteArray buf;
    ReadRam32(address,4,buf);

    return ReadUint32(buf);
}

void QStLink::WriteRamWord(uint32_t address, uint32_t data)
{
    QByteArray buf;
    FillArrayEndian32(buf,data);

    WriteRam32(address,buf);
}

void QStLink::WriteRamByte(uint32_t address, uint8_t data)
{
    QByteArray buf;
    buf.append(data);

    WriteRam8(address,buf);
}

void QStLink::WriteRamHalfWord(uint32_t address, uint16_t data)
{
    QByteArray buf;
    FillArrayEndian16(buf,data);
    WriteRam8(address,buf);
}

void QStLink::Vector32toByteArray(QByteArray &dest, const QVector<quint32> &input)
{
    dest.resize(sizeof(quint32) * input.count());
    for (int i = 0 ; i < input.count(); i++)
    {
        qToLittleEndian<quint32>(input.at(i),(uchar*)dest.data()+sizeof(quint32)*i);
    }
}
