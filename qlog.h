#ifndef QLOG_H
#define QLOG_H

#include <QString>
#include <QTime>

#define BOTHER(x) QLog::InfoMore(x)
#define INFO(x) QLog::Info(x)
#define WARN(x) QLog::Warn(x)
#define ERR(x) QLog::Err(x)
#define GDB_REC(x) QLog::GdbRec(x)
#define GDB_SEND(x) QLog::GdbSend(x)

class QLog
{
public:
    inline static void InfoMore(const QString & str) {Log("BOTHER",str,5); }
    inline static void Info(const QString & str) {Log("INFO",str,4); }
    inline static void Warn(const QString & str) {Log("WARNING",str,1); }
    inline static void Err(const QString & str) {Log("ERROR",str,0); }
    inline static void GdbRec(const QString & str) {Log("RECV", str,3); }
    inline static void GdbSend(const QString & str) {Log("SEND",str,3); }

private:
    static void Log(const QString & comm, const QString & str, unsigned int level);

};

#define EXECUTION_TIME(x,h) \
    QTime h##_time1 = QTime::currentTime(); \
    {x} \
    QTime h##_time2 = QTime::currentTime(); \
    int h##_ms1 = (h##_time1).minute() * 60000 + (h##_time1).second() * 1000 + h##_time1.msec(); \
    int h##_ms2 = (h##_time2).minute() * 60000 + (h##_time2).second() * 1000 + h##_time2.msec(); \
    int h##_result = h##_ms2 - h##_ms1;


#endif // QLOG_H
