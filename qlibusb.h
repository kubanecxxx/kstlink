#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QObject>
#include <libusb.h>


class QLibusb : public QObject
{
    Q_OBJECT
public:
    explicit QLibusb(QObject *parent = 0);
    ~QLibusb(void);

    //api
    int Write(const QByteArray & data) throw ( QString );
    QByteArray Read(int count) throw ( QString );
    QByteArray ReadTrace();


signals:

    
public slots:

private:
    libusb_context * context;
    libusb_device_handle * handle;

    int pid;
    int tx_ep;
    int trace_ep;
    int rx_ep;



};

#endif // QLIBUSB_H
