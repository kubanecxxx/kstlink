#include "qlog.h"
#include <QDebug>
#include <QDateTime>

extern unsigned int log_level;

void QLog::Log(const QString & comm,const QString &str, unsigned int level)
{
    if (level <= log_level)
    {
        QDateTime time = QDateTime::currentDateTime();

        if (level != 0)
        {
            qDebug() << comm << time.toString("dd.MM.yy hh:mm:ss") << str;
        }
        else
        {
            QString jo;
            jo += comm+ time.toString(" dd.MM.yy hh:mm:ss ") + str;

            qFatal(jo.toStdString().c_str());
        }
    }
}
