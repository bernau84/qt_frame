#include <QCoreApplication>
#include "rt_audioinput.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    t_rt_audioinput aud_src("");
    aud_src.on_start(0);

    return a.exec();
}
