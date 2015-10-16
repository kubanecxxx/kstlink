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

    ui->editFilename->setText("/home/kuba/workspace/test/build/ch.bin");
    erasing = false;
}

Flash::~Flash()
{
    delete ui;
}

void Flash::EnableWidget(bool enable)
{
    QFile fn(ui->editFilename->text());
    ui->buttonErase->setEnabled(enable);
    ui->buttonFlash->setEnabled(enable && fn.exists());
    enabled = enable;
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
    QFile fn(ui->editFilename->text());
    if (!fn.exists())
        return;


    emit flashWriteRequest(ui->editFilename->text());
    EnableWidget(false);
}

void Flash::on_editFilename_textChanged(const QString &arg1)
{
    QFile fn(arg1);
    ui->buttonFlash->setEnabled(fn.exists() && enabled);

}

void Flash::ErasingActive(bool active)
{
    erasing = active;
}

void Flash::Flashing(int percent)
{
    ui->labelOperation->setText(tr("Flashing"));
    ui->progress->setValue(percent);
}

void Flash::Verifing(int percent)
{
    ui->labelOperation->setText(tr("Verifing"));
    ui->progress->setValue(percent);
}

void Flash::Erasing(int percent)
{
    ui->labelOperation->setText(tr("Erasing"));
    ui->progress->setValue(percent);
}

void Flash::Success(bool ok)
{
    EnableWidget(true);
}


