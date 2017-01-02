#include <QCoreApplication>

#include "easylogging++.h"

#include "rt_audioinput.h"
#include "rt_recorder.h"
#include "rt_generator.h"
#include "rt_wavinput.h"
#include "filter_fir.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    t_f_win<double> wi_gauss(WGAUS, 13, "#B=500#fs=1000#TA=100");
    LOG(INFO) << wi_gauss[0];
    LOG(INFO) << wi_gauss[8];
    LOG(INFO) << wi_gauss[12];
    LOG(INFO) << wi_gauss[2];
    wi_gauss.log();

    t_f_win<double> wi_hann(WHANN, 8, "#B=500#fs=1000#TA=100");
    LOG(INFO) << "[0]" << wi_hann[0] << " [4]" << wi_hann[4];
    wi_hann.log();

    t_filter_fir_direct<double> dfir();

    t_filter_wfir<double> wfir(8, WHAMM, "#B=500#fs=1000");
    for(int i=0; i<16; i++)
        LOG(INFO) << i << " " << *(wfir.proc(1));

    return 0;

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
