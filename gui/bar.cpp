#include "bar.h"
#include "ui_bar.h"
#include <QMouseEvent>
#include <QTimer>

bar::bar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::bar)
{
    ui->setupUi(this);
    setWindowFlags( Qt::WindowStaysOnTopHint | Qt::ToolTip );
    QRect rect = geometry();
    //rect.setX(100);
    //rect.setY(100);
    setGeometry(rect);
    setMouseTracking(true);
}

bar::~bar()
{
    delete ui;
}

void bar::ShowTicks(quint32 ticks)
{
    now = ticks;

    ui->pushButton->setText(QString("%1 ticks").arg(now - last));
}

void bar::ShowPercents(int percent, const QString &task)
{
    show();
    ui->label->setText(QString("%1: %2\%").arg(task).arg(percent));
    ui->graph->setValue(percent);
}

#if 0
void bar::mousePressEvent(QMouseEvent *evt)
{
    if (!evt->button() == Qt::LeftButton)
        return;

    pos = evt->globalPos();
}

void bar::mouseMoveEvent(QMouseEvent *evt)
{
    if (!evt->buttons() == Qt::LeftButton)
        return;

    QRect rect = geometry();
    rect.setRect(-pos.x() + evt->pos().x(),-pos.y() +evt->pos().y(),rect.width(),rect.height());

    setGeometry(rect);
}
#endif

void bar::on_pushButton_clicked()
{
    last = now;
    ShowTicks(last);
}
