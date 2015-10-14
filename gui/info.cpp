#include "info.h"
#include "ui_info.h"
#include <QTimer>

Info::Info(const s_t & t,QWidget *parent) :
    Page(parent),
    ui(new Ui::Info),
    s(t)
{
    ui->setupUi(this);
    QObject * tray = parent;

    ui->butReset->setProperty("core","SysReset");
    ui->butRun->setProperty("core","CoreRun");
    ui->butStep->setProperty("core","CoreSingleStep");
    ui->butStop->setProperty("core","CoreStop");
    connect(ui->butReset,SIGNAL(clicked()),tray,SLOT(Core()));
    connect(ui->butRun,SIGNAL(clicked()),tray,SLOT(Core()));
    connect(ui->butStep,SIGNAL(clicked()),tray,SLOT(Core()));
    connect(ui->butStop,SIGNAL(clicked()),tray,SLOT(Core()));

    QTimer * tim = new QTimer;
    connect(tim,SIGNAL(timeout()),this,SLOT(timeout()));
    tim->start(100);
}

Info::~Info()
{
    delete ui;
}

void Info::timeout()
{
    ui->labBreaks->setNum(s.breakpointCount);
    ui->labChipID->setText(QString("0x%1").arg(s.chipID,0,16));
    ui->labCoreID->setText(QString("0x%1").arg(s.coreID,0,16));
    ui->labMcuType->setText(s.mcuName);

    ui->labCoreMode->setText(s.coreMode);
    ui->labCoreStatus->setText(s.run);
    ui->labPcAddress->setText(QString("0x%1").arg(s.StopAddress,0,16));
}
