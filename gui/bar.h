#ifndef BAR_H
#define BAR_H

#include <QWidget>

namespace Ui {
class bar;
}

class bar : public QWidget
{
    Q_OBJECT

public:
    explicit bar(QWidget *parent = 0);
    ~bar();

private:
    Ui::bar *ui;
    //void mouseMoveEvent(QMouseEvent * evt);
    //void mousePressEvent(QMouseEvent * evt);
    QPoint pos;
    quint32 last;
    quint32 now;
    quint32 frequency;

public slots:
    void ShowPercents(int percent, const QString & task);
    void ShowTicks(quint32 ticks);
    void on_pushButton_clicked();
    void timeout(void);
};

#endif // BAR_H
