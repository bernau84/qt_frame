#include <QCoreApplication>
#include "rt_audioinput.h"
#include "rt_recorder.h"
#include "rt_generator.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    t_rt_audioinput aud_src("");
    t_rt_recorder aud_rec("");
    t_rt_generator sig_gen("", [](double t) -> double { return sin(2*M_PI*800*t); });

    //aud_rec.connect(&aud_src);
    aud_rec.connect(&sig_gen);

    aud_rec.on_start(0);
    aud_src.on_start(0);
    sig_gen.on_start(0);

    return a.exec();
}
