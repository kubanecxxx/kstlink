#include <QCoreApplication>
#include <include.h>
#include "qstlink.h"
#include "flasher.h"
#include <QFile>
#include "QDebug"
#include "gdbserver.h"

unsigned int log_level = 3;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

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
    int port = 4242;
    for (int i = 0 ; i< input_pars.count(); i++)
    {
        if (input_pars[i].startsWith("-m"))
        {
            mcu = input_pars[i].mid(2);
        }
        else if (input_pars[i] == "-flashonly")
        {
            flashonly = true;
        }
        else if (input_pars[i] == "-notverify")
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
    }

    if(mcu.isEmpty())
        qFatal("You must specify mcu with -mprefix");

    if (flashonly)
    {
        //run only flasher and exit
        if (file.isEmpty())
            qFatal("If you want to flash you must specify input binary file with -i prefix");

        QFile fil(file);
        if (!fil.exists())
            qFatal("Input file doesn't exist");

        try
        {
            new flasher(&a,fil,mcu);
        } catch (QString data)
        {
            ERR(data);
        }
    }
    else
    {
        //run gdbserver
        try
        {
            new GdbServer(&a,mcu,notverify,port);
        } catch (QString data)
        {
            WARN(data);
        }
        return a.exec();
    }
}


