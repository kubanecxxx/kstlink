#ifndef KELNET_H
#define KELNET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <qstlink.h>

class Kelnet : public QObject
{
    Q_OBJECT
public:
    explicit Kelnet(QStLink & st,QObject *parent = 0);
    
signals:
    
public slots:
private slots:
    void ReadyRead(void);
    void NewConnection(void);
    void Closed(void);

private:
    QStLink & stlink;
    QTcpServer * server;
    QTcpSocket * client;

    typedef QVector<QByteArray> param_t;

    static param_t parsePacket(const QByteArray & data);
};

#endif // KELNET_H
