#include "flasher.h"
#include "QDebug"
#include <QString>
#include <qstlink.h>
#include <QCoreApplication>

flasher::flasher(QObject *parent, QFile & BinaryFile, const QByteArray &mcu) :
    QObject(parent),
    file(BinaryFile),
    stlink(*new QStLink(this,mcu))
{
    if (!file.open(QFile::ReadOnly))
        qFatal("Cannot open input file");

    connect(&stlink,SIGNAL(Erasing(int)),this,SLOT(erasing(int)));
    connect(&stlink,SIGNAL(Flashing(int)),this,SLOT(flashing(int)));
    connect(&stlink,SIGNAL(Reading(int)),this,SLOT(read(int)));

    QByteArray data = file.readAll();
    stlink.FlashWrite(FLASH_BASE,data);
    stlink.SysReset();
    stlink.FlashVerify(data);
}

void flasher::flashing(int percent)
{
    QString format = QString("Flashing progress: %1").arg(percent);
    qDebug() << format;
}

void flasher::erasing(int percent)
{
    QString format = QString("Erasing progress: %1").arg(percent);
    qDebug() << format;
}

void flasher::read(int percent)
{
    QString format = QString("Reading progress: %1").arg(percent);
    qDebug() << format;

    if (percent == 100)
    {
        qDebug() << "Mcu flashed succesfully";
        stlink.CoreRun();
        QCoreApplication * app = qobject_cast<QCoreApplication *>(parent());
        app->exit(0);
    }
}
