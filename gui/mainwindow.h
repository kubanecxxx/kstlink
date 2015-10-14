#ifndef MAINWINDOW_H1
#define MAINWINDOW_H1

#include <QMainWindow>
#include <QSystemTrayIcon>


class bar;
class QMenu;

namespace Ui {
class MainWindow;
}

typedef struct
{
    QString run;
    quint32 StopAddress;
    int chipID;
    quint32 coreID;
    QString mcuName;
    QString coreMode;
    int breakpointCount;
} s_t;

class Communication;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Communication * communication, QObject *parent = 0);
    ~MainWindow();



private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * tray;

private slots:
    void CoreHalted(quint32);
    void CommunicationFailed();
    void CoreRunning();
    void Verification(bool ok);
    void Erasing(int);
    void Flashing(int);
    void Reading(int);
    void timeout();
    void ResetRequested();

    void activated(QSystemTrayIcon::ActivationReason);
    void tooLongNic(void);

    void Core(void);

    bool refreshState(void);
private:

    bar * prog;
    QTimer * timer;
    s_t s;
    Communication * com;

};

#endif // MAINWINDOW_H
