#ifndef QLOG_H
#define QLOG_H

#include <QString>

#define BOTHER(x) QLog::InfoMore(x)
#define INFO(x) QLog::Info(x)
#define WARN(x) QLog::Warn(x)
#define ERR(x) QLog::Err(x)

class QLog
{
public:
    inline static void InfoMore(const QString & str) {Log("BOTHER",str,3); }
    inline static void Info(const QString & str) {Log("INFO",str,2); }
    inline static void Warn(const QString & str) {Log("WARNING",str,1); }
    inline static void Err(const QString & str) {Log("ERROR",str,0); }

private:
    static void Log(const QString & comm, const QString & str, unsigned int level);

};

#endif // QLOG_H
