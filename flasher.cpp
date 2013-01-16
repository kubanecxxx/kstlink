#include "flasher.h"
#include "QDebug"
#include <QString>
#include <qstlink.h>
#include <QCoreApplication>

flasher::flasher(QObject *parent, QFile & BinaryFile, const QByteArray &mcu, bool verifonly) :
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
    if (!verifonly)
    {
        stlink.FlashWrite(FLASH_BASE,data);
        stlink.SysReset();
    }
    bool ok = stlink.FlashVerify(data);

    if (ok)
    {
        stlink.BreakpointRemoveAll();
        stlink.CoreRun();
        qDebug() << "Verification OK";
    }
    else
    {
        qDebug() << "Verification failed";
    }
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
    QString format = QString("Verifying progress: %1").arg(percent);
    qDebug() << format;
}