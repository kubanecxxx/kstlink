#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widgettreepages.h"
#include "info.h"
#include "flash.h"
#include  <QTimer>
#include "bar.h"
#include "communication.h"
#include <qdebug.h>

MainWindow::MainWindow(Communication * comu, QObject *parent) :
    QMainWindow(NULL),
    ui(new Ui::MainWindow),
    prog(new bar),
    timer(new QTimer(this)),
    com(comu)
{

    Q_ASSERT_X(comu, "No communcation interface", "");
    Q_ASSERT(parent);
    Q_INIT_RESOURCE(resourcesGUI);

    ui->setupUi(this);
    WidgetTreePages * w = new WidgetTreePages(this);
    setCentralWidget(w);

    Page * p;
    p = new Info(s,this);
    w->AddPage(p);

    p = new Flash(this);
    w->AddPage(p);

    bool ok;

    tray = new QSystemTrayIcon;
    tray->show();

    QTimer * timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));
    timer->start(300);


    //interface = "org.kubanec.kstlink.stlink";
    connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,
            SLOT(activated(QSystemTrayIcon::ActivationReason)));

    connect(this->timer,SIGNAL(timeout()),this,SLOT(tooLongNic()));
    this->timer->setSingleShot(true);
    this->timer->start(100);



    //context menu
    QMenu * c_m = new QMenu;
    QAction * a;

    a = c_m->addAction(QIcon(":/tray/config"),tr("Setting"));
    connect(a,SIGNAL(triggered()),this,SLOT(show()));
    c_m->addSeparator();
    a = c_m->addAction(QIcon(":/tray/pause"),tr("Stop core"));
    a->setProperty("core","CoreStop");
    connect(a,SIGNAL(triggered()),this,SLOT(Core()));
    a = c_m->addAction(QIcon(":/tray/play"),tr("Start core"));
    a->setProperty("core","CoreRun");
    connect(a,SIGNAL(triggered()),this,SLOT(Core()));
    a = c_m->addAction(QIcon(":/tray/restart"),tr("Reset core"));
    a->setProperty("core","SysReset");
    connect(a,SIGNAL(triggered()),this,SLOT(Core()));

    c_m->addSeparator();
    //exit
    a = c_m->addAction(QIcon(":/tray/stop"),tr("Exit"));
    connect(a,SIGNAL(triggered()),parent,SLOT(quit()));

    tray->setContextMenu(c_m);
    tray->setIcon(QIcon(":/tray/stop"));


    //connect communication signals
    connect(com,SIGNAL(CoreHalted( quint32)),this,SLOT(CoreHalted(quint32)));
    connect(com,SIGNAL(CommunicationFailed()),this,SLOT(CommunicationFailed()));
    connect(com,SIGNAL(CoreRunning()),this,SLOT(CoreRunning()));
    connect(com,SIGNAL(Verification(bool)),this,SLOT(Verification(bool)));
    connect(com,SIGNAL(Erasing(int)),this,SLOT(Erasing(int)));
    connect(com,SIGNAL(Flashing(int)),this,SLOT(Flashing(int)));
    connect(com,SIGNAL(Reading(int)),this,SLOT(Reading(int)));
    connect(com,SIGNAL(CoreResetRequested()),this,SLOT(ResetRequested()));

   show();

}

typedef void (Communication::*m_t)(void) ;
void MainWindow::Core()
{
    QMap<QString,m_t> map;

    map.insert("SysReset",&Communication::SysReset);
    map.insert("CoreRun",&Communication::CoreRun);
    map.insert("CoreStop", &Communication::CoreStop);
    map.insert("CoreSingleStep",&Communication::CoreSingleStep);
    map.insert("Erase",&Communication::FlashMassClear);



    QVariant v = sender()->property("core");
    Q_ASSERT(v.isValid());
    m_t  m = map.value(v.toString(),NULL);
    Q_ASSERT(m);
    (com->*m)();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::tooLongNic()
{
    tray->hide();
    prog->hide();
}

void MainWindow::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
        prog->setHidden(prog->isVisible());
}

void MainWindow::CoreHalted(quint32 address)
{
    s.StopAddress = address;
    s.run = tr("Halted");
    tray->setIcon(QIcon(":/tray/pause"));
}

void MainWindow::CommunicationFailed()
{
    tray->setIcon(QIcon(":/tray/stop"));
    if (!timer->isActive())
        timer->start(10000);
}

void MainWindow::CoreRunning()
{
    s.run = tr("Running");
    s.StopAddress = 0;
    tray->setIcon(QIcon(":/tray/play"));
}

void MainWindow::Verification(bool ok)
{
    if (ok)
        tray->showMessage("Kstlink", tr("Verification successful"),
                          QSystemTrayIcon::Information,3000);
    else
        tray->showMessage("Kstlink",tr("Verification Failure"),
                          QSystemTrayIcon::Critical,30000);

    prog->hide();
}

void MainWindow::Erasing(int percent)
{
    prog->ShowPercents(percent,tr("Erasing"));
}

void MainWindow::Reading(int percent)
{
    prog->ShowPercents(percent,tr("Reading"));
}

void MainWindow::Flashing(int percent)
{
    prog->ShowPercents(percent,tr("Flashing"));
}



void MainWindow::timeout()
{
   bool ok = refreshState();

   if (ok)
   {
        QString tooltip;
        tooltip = QString("<table>\
                          <caption>Kstlink GUI</caption> \
                          <br/>\
                          <tr><td>Core Status</td><td><b>%1</b></td></tr> \
                          <tr><td>Core mode</td><td><b>%5</b></td></tr> \
                          <tr><td>PC address</td><td><b>0x%2</b></td></tr>\
                          <br/>\
                          <tr><td>Core ID</td><td><b>0x%3</b></td></tr>\
                          <tr><td>Chip ID</td><td><b>0x%4</b></td></tr>\
                          <tr><td>MCU name</td><td><b>%6</b></td></tr> \
                          <tr><td>Breakpoint count</td><td><b>%7</b></td></tr>\
                          </table>").
                arg(s.run).arg(s.StopAddress,0,16).arg(s.coreID,0,16).arg(s.chipID,0,16).
                arg(s.coreMode).arg(s.mcuName).arg(s.breakpointCount);

        tray->setToolTip(tooltip);
        tray->show();
        timer->stop();

        prog->ShowTicks(com->GetCycleCounter());
   }
   else
   {
       tray->setToolTip(QString());
       CommunicationFailed();
   }
}

bool MainWindow::refreshState()
{
    s.chipID = com->GetChipID();
    s.coreID = com->GetCoreID();
    s.mcuName = com->GetMcuName();
    s.coreMode = com->GetModeString();
    s.breakpointCount = com->GetBreakpointCount();

    return true;
}


void MainWindow::ResetRequested()
{
    prog->on_pushButton_clicked();
}
