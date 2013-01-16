#ifndef TEMP_H
#define TEMP_H

#include <QObject>
#include <QFile>

class QStLink;
class flasher : public QObject
{
    Q_OBJECT
public:
    explicit flasher(QObject *parent, QFile & BinaryFile, const QByteArray & mcu);
    
signals:
    
public slots:
    void flashing(int percent);
    void erasing(int percent);
    void read(int percent);

private:
    QFile & file;
    QStLink & stlink;
};

#endif // TEMP_H
