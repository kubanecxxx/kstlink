#ifndef INFO_H
#define INFO_H

#include <QWidget>
#include "page.h"


namespace Ui {
class Info;
}

#include "mainwindow.h"

class Info : public Page
{
    Q_OBJECT

public:
    explicit Info(const s_t & t, QWidget *parent = 0);
    ~Info();

private:
    Ui::Info *ui;
    const s_t & s;

public slots:
    void EnableWidget(bool enable);

private slots:
    void timeout(void);
};

#endif // INFO_H
