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
    explicit GdbServer(QObject *parent, QStLink * stlink, bool notverify, int PortNumber,QByteArray & file);
    QStLink * GetStlink();
    
signals:
    
public slots:
    
private slots:
    void newConnection();
    void ReadyRead(void);
    void CoreHalted(quint32 addr);

    void Erasing(int perc);
    void Flashing(int perc);
    void Verify(int perc);
    void timeout(void);

private:
    QStLink * stlink;
    QTcpServer * server;
    QTcpSocket * socan;
    const int port;

    typedef QVector<QByteArray> params_t;

    static void MakePacket(QByteArray & data, QByteArray * binary = NULL, bool ok = true);
    static int DecodePacket(QByteArray & data);
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

    void progressBar(int percent, const QString & operation);
    QString prevOpearation;

    int qsThreadInfo;

    int threads();
    QStringList handlers_list;

    void printMCUInfo(void);
    QString getHandler();

    bool switched;
    int segment;
    quint32 offset;

};

#endif // GDBSERVER_H
