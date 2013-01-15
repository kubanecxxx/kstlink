#include "temp.h"
#include "QDebug"
#include <QString>

temp::temp(QObject *parent) :
    QObject(parent)
{
}

void temp::flashing(int percent)
{
    QString format = QString("Flashing progress: %1").arg(percent);
    qDebug() << format;
}

void temp::erasing(int percent)
{
    QString format = QString("Erasing progress: %1").arg(percent);
    qDebug() << format;
}

void temp::read(int percent)
{
    QString format = QString("Reading progress: %1").arg(percent);
    qDebug() << format;
}
