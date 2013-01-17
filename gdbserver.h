#ifndef GDBSERVER_H
#define GDBSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QTcpServer>
#include <inttypes.h>
#include <QFile>

class QStLink;
class GdbServer : public QObject
{
    Q_OBJECT
public:
    explicit GdbServer(QObject *parent, const QByteArray & mcu, bool notverify, int PortNumber,QByteArray & file);
    
signals:
    
public slots:
    
private slots:
    void newConnection();
    void ReadyRead(void);
    void CoreHalted(uint32_t addr);

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

    QByteArray ans;
    QByteArray input;
    QTcpSocket * soc;
    QByteArray FlashProgram;

    const bool NotVerify;
    uint32_t lastAddress;
    QByteArray VeriFile;
};

#endif // GDBSERVER_H
