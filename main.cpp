#include <QCoreApplication>
#include "rt_audioinput.h"
#include "rt_recorder.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    t_rt_audioinput aud_src("");
    t_rt_recorder aud_rec("");

    aud_rec.connect(&aud_src);

    aud_rec.on_start(0);
    aud_src.on_start(0);

    return a.exec();
}
