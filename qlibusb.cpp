#include "qlibusb.h"
#include "include.h"
#include "QDebug"
#include "stlinkCommands.h"

QLibusb::QLibusb(QObject *parent) :
    QObject(parent),
    context(0),
    handle(0)

{
    BOTHER("Opening libusb");
    if (libusb_init(&context) != 0)
    {
        ERR("Cannot Open Libusb");
    }
    BOTHER("Libusb Opened");
    BOTHER("Opening ST-Link");

    handle = libusb_open_device_with_vid_pid(context,VID,PID);

    if (handle == NULL)
    {

        //ERR("Cannot open stlink");
        handle = libusb_open_device_with_vid_pid(context, VID, PID_V21);
        if (handle == NULL)
        {
            ERR("Cannot open stlink");
        }
        qDebug() << QString("StLink V2.1 detected (0x0%1:0x%2)").arg(VID,0,16).arg(PID_V21,0,16);
        pid = PID_V21;
        tx_ep = EPOUT_V21;
        trace_ep = EP_TRACE_V21;
        rx_ep = EPIN;
    }
    else
    {
        qDebug() << QString("StLink V2 detected (0x0%1:0x%2)").arg(VID,0,16).arg(PID,0,16);
        pid = PID;

        tx_ep = EPOUT;
        trace_ep = EP_TRACE;
        rx_ep = EPIN;
    }





    BOTHER("Stlink Opened");
}

QLibusb::~QLibusb()
{
    if (handle)
        libusb_close(handle);
    if (context)
        libusb_exit(context);
}


int QLibusb::Write(const QByteArray &data) throw(QString)
{
    QString log = "Writing to stlink " ;
    log += data.toHex();
    INFO(log);
    int tranfserred = 0;
    if (libusb_bulk_transfer(handle,tx_ep,(unsigned char *)data.data(),data.length(),&tranfserred,1000))
    {
        throw (QString("Write to stlink timeout"));
    }

    return tranfserred;
}

QByteArray QLibusb::Read(int count) throw ( QString )
{
    unsigned char  buffer [1000];
    int length = 0;
    int cant = count;

    if (cant == 0)
        count = 500;

    QByteArray ret;
    int tries = 5;
    while (count)
    {
        libusb_bulk_transfer(handle,rx_ep,buffer,count, &length,40);
        ret.append((char *)buffer,length);
        count -= length;

        if (tries-- == 0)
        {
            if (cant != 0)
                throw(QString("Stlink read timeout, transferred: %1/%2").arg(cant-count).arg(cant));
            else
                break;
        }
    }

    QString log = "Read from Stlink: ";
    log += ret.toHex();
    INFO(log);

    return ret;
}

QByteArray QLibusb::ReadTrace()
{
    unsigned char  buffer [1000];
    int length = 0;

    QByteArray ret;
    int tries = 20;
    while (1)
    {
        libusb_bulk_transfer(handle,trace_ep,buffer,1000, &length,10);
        ret.append((char *)buffer,length);

        if (tries-- == 0)
        {
            break;
        }
    }

    return ret;
}
