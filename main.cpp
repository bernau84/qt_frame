#include <QCoreApplication>

#include "easylogging++.h"
#include "f_windowing.h"
#include "filter_fir.h"

#include "rt_audioinput.h"
#include "rt_audiooutput.h"
#include "rt_recorder.h"
#include "rt_generator.h"
#include "rt_wavinput.h"
#include "rt_processor.h"

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

    double linavr[] = {1.0/8, 1.0/8, 1.0/8, 1.0/8, 1.0/8, 1.0/8, 1.0/8, 1.0/8};
    t_filter_fir<double> dfir(linavr, sizeof(linavr)/sizeof(double));

    for(int i=0; i<16; i++)  //impuslni odezva
        LOG(INFO) << "I[" << i << "] " << *(dfir.proc(0 == i));

    for(int i=0; i<16; i++)  //prechodova charakteristika
        LOG(INFO) << "H[" << i << "] " << *(dfir.proc(1));

    t_filter_wfir<double> wfir(8, WHANN, "#B=500#fs=1000");
    for(int i=0; i<16; i++)
        LOG(INFO) << i << " " << *(wfir.proc(1));

    t_rt_audioinput aud_src("");
    t_rt_generator sig_src("", [](double t) -> double { return 0.8*sin(2*M_PI*2000*t); });
    t_rt_wavinput wav_src("");

    a_rt_base *src_sel = &sig_src;

    t_rt_filter fir_pro("", dfir);
    t_rt_recorder pro_rec("{\"path\":{\"__def\":\"filteredx.wav\"}}");
    t_rt_recorder src_rec("{\"path\":{\"__def\":\"sourcex.wav\"}}");
    t_rt_audiooutput aud_out("");

    //aud_rec.connect(&sig_gen);
//    src_rec.connect(src_sel);
//    fir_pro.connect(src_sel);
//    pro_rec.connect(&fir_pro);
    aud_out.connect(src_sel);

    aud_out.on_start(0);
    src_sel->on_start(0);
//    src_rec.on_start(0);
//    fir_pro.on_start(0);
//    pro_rec.on_start(0);


    /*
    aud_src.on_start(0);
    sig_gen.on_start(0);
    */

    return a.exec();
}
