#ifndef RT_WAVINPUT_H
#define RT_WAVINPUT_H

#include <QVector>
#include "wav_read_file.h"
#include "rt_generator.h"

class t_wav_read_cached : public t_wav_file_reader {

private:
    std::vector<double> cache;
    uint32_t cache_i;

public:
    double sample(double t){

        if(cache_i == 0){

             int written = t_wav_file_reader::read(cache.data(), cache.size());
             if(written != cache.size())
                for_each(cache.begin()+written, cache.end(), [](double &x){ x = 0.1*sin(2*M_PI*128*t); });
        }

        double ret = cache[cache_i];
        cache_i = (cache_i + 1) % cache.size();
        return ret;
    }

    t_wav_read_cached(const char *path, bool auto_rewind = true, unsigned cache_sz = 256):
        t_wav_file_reader(path, auto_rewind),
        cache(cache_sz)
    {
        cache_i = 0;
    }

};

class t_rt_wavinput : public a_rt_base {

private:
    t_wav_read_cached file;
    t_rt_generator player;

public:
    t_rt_wavinput(const QString &js_config, QObject *parent = NULL):
      file(path, )
      player(js_config, , parent),
      f_sample(_f_sample)
    {
        //by default - v ramci on-start z konfigu do budoucna
        ms_period = 50;
        fs = 8000;
        ms_proc = 0;
    }

    virtual ~t_rt_generator(){}
};

#endif // RT_WAVINPUT_H
