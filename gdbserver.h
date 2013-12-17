#ifndef GDBSERVER_H
#define GDBSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QTcpServer>
#include <inttypes.h>
#include <QFile>
#include "qstlink.h"

class QStLink;
class GdbServer : public QObject
{
    Q_OBJECT
public:
    explicit GdbServer(QObject *parent, const QByteArray & mcu, bool notverify, int PortNumber,QByteArray & file,bool stop);
    
signals:
    
public slots:
    
private slots:
    void newConnection();
    void ReadyRead(void);
    void CoreHalted(quint32 addr);

    void Erasing(int perc);
    void Flashing(int perc);
    void Verify(int perc);

private:
    QStLink * stlink;
    QTcpServer * server;
    const int port;

    typedef QVector<QByteArray> params_t;

    static void MakePacket(QByteArray & data, QByteArray * binary = NULL);
    static bool DecodePacket(QByteArray & data);
    params_t ParseParams(const QByteArray & data);

    void processPacket(QTcpSocket * soc,const QByteArray & rawdata);
    QByteArray processQueryPacket(const QByteArray & rawdata);
    QByteArray processBreakpointPacket(const QByteArray & rawdata);
    void processEscapeChar(QByteArray & arr);

    QByteArray ans;
    QByteArray input;
    QTcpSocket * soc;
    QByteArray FlashProgram;

    const bool NotVerify;
    uint32_t lastAddress;
    QByteArray VeriFile;

    int thread_id;

    QMap<int,QStLink::mode_t> threaed;



};

#endif // GDBSERVER_H
