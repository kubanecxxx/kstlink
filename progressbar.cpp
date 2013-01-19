#include "progressbar.h"
#include "ui_progressbar.h"

ProgressBar::ProgressBar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressBar)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::SubWindow);

}

ProgressBar::~ProgressBar()
{
    delete ui;
}

void ProgressBar::SetPercent(int perc)
{
    ui->progressBar->setValue(perc);
}

void ProgressBar::SetAction(const QString &action)
{
    setWindowTitle(action);
    ui->labAction->setText(action);
}
