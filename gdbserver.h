#ifndef GDBSERVER_H
#define GDBSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QTcpServer>
#include <inttypes.h>

class QStLink;
class GdbServer : public QObject
{
    Q_OBJECT
public:
    explicit GdbServer(QObject *parent);
    
signals:
    
public slots:
    
private slots:
    void newConnection();
    void ReadyRead(void);

private:
    QStLink * stlink;
    QTcpServer * server;
    int port;

    typedef QVector<QByteArray> params_t;

    static void MakePacket(QByteArray & data);
    static bool DecodePacket(QByteArray & data);
    params_t ParseParams(const QByteArray & data);

    void processPacket(QTcpSocket * soc,const QByteArray & rawdata);
    QByteArray processQueryPacket(const QByteArray & rawdata);
    QByteArray processBreakpointPacket(const QByteArray & rawdata);

    QByteArray ans;
    QByteArray input;
};

#endif // GDBSERVER_H
