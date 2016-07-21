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
    QStringList input_pars;
    for (int i = 1 ; i < argc; i++)
    {
        input_pars.push_back(argv[i]);
    }

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
    bool trace = false;
    long freq = 168e6;

    foreach (QString param , input_pars)
    {
        QStringList values = param.split("=");
        QString par = values.at(0);
        if (par.startsWith("-m"))
        {
            mcu = values.at(1).toAscii();
        }
        else if (par == "--flashonly")
        {
            flashonly = true;
        }
        else if (par == "--notverify")
        {
            notverify = true;
        }
        else if (par.startsWith("-i"))
        {
            file = values.at(1).toAscii();
        }
        else if (par.startsWith("-p"))
        {
            port = values.at(1).toInt();
        }
        else if (par == "--verifyonly")
        {
            verifyonly = true;
        }
        else if (par == "--erase")
        {
            masserase = true;
        }
        else if (par == "--stopcore")
        {
            stop = true;
        }
        else if (par == "--run")
        {
            run = true;
        }
        else if (par == "--nogui")
        {
            gui = false;
        }
        else if (par == "--nogdb")
        {
            gdb = false;
        }
        else if(par == "--trace")
        {
            trace =true;
            bool ok;
            freq = values.at(1).toLong(&ok);
            Q_ASSERT(ok);

        }
        else
        {
            qDebug() << "unrecognized parameter:" << par;
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
                stlink = new QStLink(app,mcu,stop, trace,freq),
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
                if (!ok)
                {
                    qFatal("Cannot connect to DBUS session");
                }
                c = new DBus(con,app);
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


