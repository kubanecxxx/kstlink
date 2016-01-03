#ifndef KTRAY_H
#define KTRAY_H

#include <QSystemTrayIcon>

class KTray : public QSystemTrayIcon
{
public:
    KTray(QObject * parent);

signals:

public slots:
};

#endif // KTRAY_H
