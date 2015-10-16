#ifndef MAINWINDOW_H1
#define MAINWINDOW_H1

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QLabel>


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

signals:
    void EnableWidget(bool enable);


private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * tray;
    void connected(bool connected);

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
    void ErasingActive(bool active);
    void FlashingActive(bool active);

    void activated(QSystemTrayIcon::ActivationReason);
    void tooLongNic(void);

    void Core(void);

    bool refreshState(void);
    void  flashWriteRequest(const QString & filename);

private:

    bar * prog;
    QTimer * timer;
    s_t s;
    Communication * com;
    QLabel * message;
    bool old_state;
    bool erasing;
    bool flashing;
    bool era_prev;
    bool connected_state;

    void messageControl();

};

#endif // MAINWINDOW_H
