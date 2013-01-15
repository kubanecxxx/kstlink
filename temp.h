#ifndef TEMP_H
#define TEMP_H

#include <QObject>

class temp : public QObject
{
    Q_OBJECT
public:
    explicit temp(QObject *parent = 0);
    
signals:
    
public slots:
    void flashing(int percent);
    void erasing(int percent);
    void read(int percent);
    
};

#endif // TEMP_H
