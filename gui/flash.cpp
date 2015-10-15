#include "flash.h"
#include "ui_flash.h"
#include <QFileDialog>
#include <QDebug>

Flash::Flash(QWidget *parent) :
    Page(parent),
    ui(new Ui::Flash)
{
    Q_ASSERT (parent);
    ui->setupUi(this);

    ui->buttonErase->setProperty("core","Erase");
    connect(ui->buttonErase, SIGNAL(clicked()), this,SIGNAL(flashEraseRequest()));

    ui->editFilename->setText("/home/kubanec/workspaces/workspace/test/build/ch.bin");
}

Flash::~Flash()
{
    delete ui;
}



void Flash::on_buttonFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"Read file",lastDir,"*.bin");

    if (filename.isEmpty())
        return;

    QDir dir (filename);
    lastDir = dir.absolutePath();

    ui->editFilename->setText(filename);
}


void Flash::on_buttonFlash_clicked()
{
    if (ui->editFilename->text().isEmpty())
        return;

    emit flashWriteRequest(ui->editFilename->text());
}

void Flash::on_editFilename_textChanged(const QString &arg1)
{
    QFile fn(arg1);
    ui->buttonFlash->setEnabled(fn.exists());

}

void Flash::Flashing(int percent)
{

}

void Flash::Verifing(int percent)
{

}

void Flash::Erasing(int percent)
{

}

void Flash::Success(bool ok)
{

}

void Flash::FlasingStarted()
{

}
