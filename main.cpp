#include <QCoreApplication>
#include <include.h>
#include "qstlink.h"
#include "flasher.h"
#include <QFile>
#include "QDebug"
#include "gdbserver.h"
#include <QApplication>

#include "mainwindow.h"
#include "communication.h"

#ifdef KSTLINK_DBUS
#include <QDBusConnection>
#endif

unsigned int log_level = 2;
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);


    //parse input params
    QVector<QByteArray> input_pars;
    for (int i = 0 ; i < argc; i++)
    {
        input_pars.push_back(argv[i]);
    }
    input_pars.remove(0);

    QByteArray mcu;
    QByteArray file;
    bool flashonly = false;
    bool notverify = false;
    bool verifyonly = false;

    bool masserase = false;
    bool stop = false;
    bool run = false;
    int port = 4242;
    bool gui = true;
    bool gdb = true;
    bool dbus = true;

    for (int i = 0 ; i< input_pars.count(); i++)
    {
        if (input_pars[i].startsWith("-m"))
        {
            mcu = input_pars[i].mid(2);
        }
        else if (input_pars[i] == "--flashonly")
        {
            flashonly = true;
        }
        else if (input_pars[i] == "--notverify")
        {
            notverify = true;
        }
        else if (input_pars[i].startsWith("-i"))
        {
            file = input_pars[i].mid(2);
        }
        else if (input_pars[i].startsWith("-p"))
        {
            QByteArray temp = input_pars[i].mid(2);
            port = temp.toInt();
        }
        else if (input_pars[i] == "--verifyonly")
        {
            verifyonly = true;
        }
        else if (input_pars[i] == "--erase")
        {
            masserase = true;
        }
        else if (input_pars[i] == "--stopcore")
        {
            stop = true;
        }
        else if (input_pars[i] == "--run")
        {
            run = true;
        }
        else if (input_pars[i] == "--nogui")
        {
            gui = false;
        }
        else if (input_pars[i] == "--nogdb")
        {
            gdb = false;
        }
        else
        {
            qDebug() << "unrecognized parameter:" << input_pars[i];
        }
    }

    dbus = !(gui && gdb);
    qDebug() << "Build date:" << __DATE__  << __TIME__;


    if (flashonly && verifyonly)
    {
        qDebug() << "verifyonly ignored";
        verifyonly = false;
    }
/*
    if(mcu.isEmpty())
        qFatal("You must specify mcu with -mprefix");

    else */


    QCoreApplication * app = NULL;



    //decide if gui or not
    if (gui)
    {
        app = new QApplication(argc,argv);
    }
    else
    {
        app = new QCoreApplication(argc,argv);
    }
    //QApplication b;

    if (masserase)
    {
        QByteArray ar;
        QStLink st(app,ar);
        st.FlashMassClear();
        qDebug() << "Erased";
    } else
    if (flashonly || verifyonly || run )
    {
        //run only flasher and exit
        if (file.isEmpty())
            qFatal("If you want to flash you must specify input binary file with -i prefix");

        QFile fil(file);
        if (!fil.exists())
            qFatal("Input file doesn't exist");

        try
        {
            new flasher(app,fil,mcu, verifyonly,run);
        } catch (QString data)
        {
            ERR(data);
        }
    }
    else
    {
        //run gdb server + gui without dbus
        //run gdb server only
        //run gui only

        QStLink * stlink = NULL;
        //run gdbserver
        if (gdb)
        {
        #ifdef KSTLINK_DBUS
            if (dbus)
            {
                bool ok = QDBusConnection::sessionBus().registerService("org.kubanec.kstlink");
                QDBusConnection::sessionBus().registerObject("/qstlink",app);

                if (ok)
                {
                    qDebug() << "Connected to DBUS session";
                }
                else
                {
                    qFatal("Failed connecting to DBUS session");
                }

            }
        #endif

            //single app - gdb only once
            try
            {
                stlink = new QStLink(app,mcu,stop),
                new GdbServer(app,stlink,notverify,port,file);
            } catch (QString data)
            {
                WARN(data);
            }
        }

        //run gui part
        if (gui)
        {
            //single app not necessary - guis could be more
            Communication * c;
#ifdef KSTLINK_DBUS
            if (dbus)
            {
                QDBusConnection * con;
                QDBusConnection con3 = QDBusConnection::connectToBus(QDBusConnection::SessionBus,"dbus");
                con = &con3;


                bool ok = true;
                        ok = con->registerService("org.kubanec.kstlinkGui");
                QDBusError  err = con->lastError();
                if (!ok)
                {
                    qFatal("Cannot connect to DBUS session");
                }
                c = new DBus(con,app);
                //c = new direct(stlink,app);
            }
            else
            {
#endif
                c = new direct(stlink,app);
#ifdef KSTLINK_DBUS
            }
#endif
            QApplication * a = qobject_cast<QApplication *> (app);
            a->setQuitOnLastWindowClosed(false);

            new MainWindow(c,app);

        }

        return app->exec();
    }
}


