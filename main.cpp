#include <QCoreApplication>
#include "rt_audioinput.h"
#include "rt_recorder.h"
#include "rt_generator.h"
#include "rt_wavinput.h"
#include "f_windowing.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    t_f_win<double> wi_gauss(WGAUS, 8, "#B=0.5#fs=1#TA=0.1");
    LOG(INFO) << wi_gauss[0];
    LOG(INFO) << wi_gauss[1];
    LOG(INFO) << wi_gauss[2];
    LOG(INFO) << wi_gauss[3];
    LOG(INFO) << wi_gauss[4];
    LOG(INFO) << wi_gauss[5];
    LOG(INFO) << wi_gauss[6];
    LOG(INFO) << wi_gauss[7];
    wi_gauss.log();

    t_f_win<double> wi_hann(WHANN, 8, "#B=0.5#fs=1#TA=0.1");
    LOG(INFO) << wi_hann[0] << wi_hann[4];
    wi_hann.log();

    t_rt_audioinput aud_src("");
    t_rt_recorder aud_rec("");
    t_rt_generator sig_gen("", [](double t) -> double { return sin(2*M_PI*440*t); });
    t_rt_wavinput wav_src("");

    aud_rec.connect(&wav_src);
    //aud_rec.connect(&sig_gen);

    wav_src.on_start(0);
    aud_rec.on_start(0);
    /*
    aud_src.on_start(0);
    sig_gen.on_start(0);
    */

    return a.exec();
}
