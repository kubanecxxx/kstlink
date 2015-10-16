#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widgettreepages.h"
#include "info.h"
#include "flash.h"
#include  <QTimer>
#include "bar.h"
#include "communication.h"
#include <qdebug.h>
#include <QFile>

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

    Info * info;
    info = new Info(s,this);
    w->AddPage(info);
    connect(this,SIGNAL(EnableWidget(bool)),info,SLOT(EnableWidget(bool)));

    Flash * flash = new Flash(this);
    w->AddPage(flash);

    connect(flash,SIGNAL(flashEraseRequest()),com,SLOT(FlashMassClear()));
    connect(flash,SIGNAL(flashWriteRequest(const QString & )),this,SLOT(flashWriteRequest(const QString &)));
    connect(com,SIGNAL(Flashing(int)),flash,SLOT(Flashing(int)));
    connect(com,SIGNAL(Verifing(int)),flash,SLOT(Verifing(int)));
    connect(com,SIGNAL(Erasing(int)),flash,SLOT(Erasing(int)));
    connect(com,SIGNAL(Verification(bool)),flash,SLOT(Success(bool)));
    connect(this,SIGNAL(EnableWidget(bool)),flash,SLOT(EnableWidget(bool)));
    connect(com,SIGNAL(ErasingActive(bool)),flash,SLOT(ErasingActive(bool)));



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
    connect(this,SIGNAL(EnableWidget(bool)),a,SLOT(setEnabled(bool)));
    connect(a,SIGNAL(triggered()),this,SLOT(Core()));
    a = c_m->addAction(QIcon(":/tray/play"),tr("Start core"));
    a->setProperty("core","CoreRun");
    connect(this,SIGNAL(EnableWidget(bool)),a,SLOT(setEnabled(bool)));
    connect(a,SIGNAL(triggered()),this,SLOT(Core()));
    a = c_m->addAction(QIcon(":/tray/restart"),tr("Reset core"));
    a->setProperty("core","SysReset");
    connect(this,SIGNAL(EnableWidget(bool)),a,SLOT(setEnabled(bool)));
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
    connect(com,SIGNAL(Verifing(int)),this,SLOT(Reading(int)));
    connect(com,SIGNAL(CoreResetRequested()),this,SLOT(ResetRequested()));
    connect(com,SIGNAL(ErasingActive(bool)),this,SLOT(ErasingActive(bool)));
    connect(com,SIGNAL(FlashingActive(bool)),this,SLOT(FlashingActive(bool)));

    erasing = false;
    flashing = false;
    connected_state = false;

    message = new QLabel();
    statusBar()->addPermanentWidget(message);

    show();

}

void MainWindow::FlashingActive(bool active)
{
    flashing = active;
    messageControl();
}

void MainWindow::ErasingActive(bool active)
{
    erasing = active;
    messageControl();
}

void MainWindow::messageControl()
{
    QString msg;
    if (connected_state)
        msg = tr ("Connected");
    else
        msg = tr("Disconnected");


    if (erasing)
    {
        msg = tr("Erasing");
    }

    if (flashing )
    {
        msg = tr("Flashing");
    }

    if (!erasing && era_prev)
    {
        statusBar()->showMessage(tr("Erased"),3000);
    }

    era_prev = erasing;
    message->setText(msg);
}

void MainWindow::flashWriteRequest(const QString &filename)
{
    QFile fn(filename);
    fn.open(QIODevice::ReadOnly);
    QByteArray d = fn.readAll();
    com->FlashWrite(FLASH_BASE, d);
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
    //tray->hide();
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
    QString msg;
    QSystemTrayIcon::MessageIcon icon;
    if (ok)
    {
        icon = QSystemTrayIcon::Information;
         msg = tr("Verification successful");
    }
    else
    {
        icon = QSystemTrayIcon::Critical;
        msg = tr ("Verification Failed");
    }

    tray->showMessage("Kstlink", msg,icon,3000);
    statusBar()->showMessage(msg,5000);
    prog->hide();
}

void MainWindow::Erasing(int percent)
{
    prog->ShowPercents(percent,tr("Erasing"));
}

void MainWindow::Reading(int percent)
{
    prog->ShowPercents(percent,tr("Verifing"));
}

void MainWindow::Flashing(int percent)
{
    prog->ShowPercents(percent,tr("Flashing"));
}



void MainWindow::timeout()
{
   bool ok = refreshState();

   if (old_state != ok)
   {
        connected(ok);
        old_state = ok;
   }


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

        try
        {
            prog->ShowTicks(com->GetCycleCounter());
        } catch (const char* e)
        {
            //dbus error
        }
   }
   else
   {
       tray->setToolTip(QString());
       CommunicationFailed();
   }
}

void MainWindow::connected(bool connected)
{
    emit EnableWidget(connected);

    connected_state = connected;
    messageControl();

}

bool MainWindow::refreshState()
{
    try
    {
        s.chipID = com->GetChipID();
        s.coreID = com->GetCoreID();
        s.mcuName = com->GetMcuName();
        s.coreMode = com->GetModeString();
        s.breakpointCount = com->GetBreakpointCount();
    } catch (const char * e)
    {
        //dbus error
        s.chipID = 0;
        s.coreID = 0;
        s.mcuName = tr("None");
        s.coreMode = tr("Invalid");
        s.breakpointCount = 0;
        s.StopAddress = 0;
        s.run = tr("Invalid");
        return false;

    }

    return true;
}


void MainWindow::ResetRequested()
{
    prog->on_pushButton_clicked();
}
