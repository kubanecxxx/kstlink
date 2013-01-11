#include "qstlink.h"
#include "QDebug"
#include <QtEndian>
#include <QTime>

QStLink::QStLink(QObject *parent) :
    QObject(parent),
    coreID(0)
{
    usb = new QLibusb(this);

    QByteArray neco =usb->Read(0);
    EnterSWDMode();
    GetStlinkMode();
    GetCoreID();
    RefreshCoreStatus();

    /*
     * otestovat funkce na registry
     * reset
     * single step
     *
     */

    CoreStop();
    WriteRegister(5,100);
    uint32_t val = ReadRegister(5);
    core_regs_t regs = ReadAllRegisters();

    /*
     * benchmark
     * speeds of read and write are ~50kbit/s
     */
#if 1
    QByteArray tx;
    int i;
    for (i = 0 ; i < 10000; i++)
        tx.append(i);
    EXECUTION_TIME(WriteRam(0x20000000,tx); , wr);
    float speed = 1.0 *i / wr_result;

    QByteArray rx;

    EXECUTION_TIME(ReadRam(0x20000000,10000,rx); , read);
    float speed2 = 1.0 *10000 / read_result;

    int co = rx.count();

    rx.clear();
#endif
}

#define SEGMENT_SIZE 512

void QStLink::ReadRam(uint32_t address, uint32_t length, QByteArray & buffer) throw(QString)
{
    BOTHER("Read ram");

    int kolik  = 0;
    while(length % 4)
    {
        length++;
        kolik++;
    }

    int temp = length;
    int size;
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
    }

    buffer.resize(buffer.count() - kolik);
}

/**
 * @brief QStLink::WriteRam
 * api, writes data, any size
 * @param address
 * @param buffer
 */
void QStLink::WriteRam(uint32_t address, const QByteArray & buffer) throw (QString)
{
    QByteArray cpy(buffer);

    BOTHER("Write ram");

    if (address < STM32_SRAM_BASE)
        throw(QString("QStLink::WriteRam: address is out of range"));

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

    tx.append(STLINK_DEBUG_READMEM_32BIT);
    FillArrayEndian32(tx,address);
    FillArrayEndian16(tx,length);

    CommandDebug(tx,buffer,length);
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
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_WRITEREG);
    tx.append(reg_idx);
    FillArrayEndian32(tx,data);
    CommandDebug(tx,rx,2);
}

uint32_t QStLink::ReadRegister(uint8_t reg_idx)
{
    BOTHER("Core read register");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_READREG);
    tx.append(reg_idx);
    CommandDebug(tx,rx,4);

    uint32_t val;
    memcpy(&val,rx.constData(),4);

    return val;
}

QStLink::core_regs_t QStLink::ReadAllRegisters()
{
    BOTHER("Core read all registers");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_READALLREGS);
    CommandDebug(tx,rx,84);
    //CommandDebug(tx,rx,0);

    core_regs_t temp;
    memcpy(&temp, rx.constData(),sizeof(core_regs_t));

    GetCoreStatus();

    return temp;
}

void QStLink::SysReset()
{
    BOTHER("System reset");
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_RESETSYS);
    CommandDebug(tx,rx,2);

    RefreshCoreStatus();
}

void QStLink::RefreshCoreStatus()
{
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_GETSTATUS);
    CommandDebug(tx,rx,2);

    unsigned char s = rx.at(0);

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
}

int QStLink::GetCoreID()
{
    QByteArray tx,rx;
    tx.append(STLINK_DEBUG_READCOREID);
    CommandDebug(tx,rx,4);

    memcpy(&coreID,rx.constData(),4);

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
    return (CoreState == "Halted");
}

int QStLink::GetStlinkMode(QString * text)
{
    QByteArray tx;
    QByteArray rx;

    tx.append(STLINK_GET_CURRENT_MODE);
    tx.append('\0');
    Command(tx,rx,2);

    int mode = rx.at(0);

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

    return mode;
}

QStLink::version_t QStLink::GetStlinkVersion()
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

    version = ver;

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
        Q_ASSERT(0);
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
        Q_ASSERT(0);
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
        Q_ASSERT(0);
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
