#include <QCoreApplication>
#include <qstlink.h>

unsigned int log_level = 3;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QStLink * stlink = new QStLink(&a);

    return a.exec();
}
