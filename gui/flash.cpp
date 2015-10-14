#include "flash.h"
#include "ui_flash.h"

Flash::Flash(QWidget *parent) :
    Page(parent),
    ui(new Ui::Flash)
{
    Q_ASSERT (parent);
    ui->setupUi(this);

    ui->buttonErase->setProperty("core","Erase");
    connect(ui->buttonErase, SIGNAL(clicked()), parent,SLOT(Core()));
}

Flash::~Flash()
{
    delete ui;
}


