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
    void reset(){

        cache_i = 0;

        in.clear();  // !clears fail and eof bits
        in.seekg(0, in.beg);
        in.read((char *)&header, sizeof(header));
    }

    double sample(double t){

        if(cache_i == 0){

             int written = t_wav_file_reader::read(cache.data(), cache.size());
             if(written != (int)cache.size())
                for_each(cache.begin()+written, cache.end(), [t](double &x){ x = 0.1*sin(2*M_PI*800*t); }); //exitus beep
        }

        double ret = cache[cache_i];
        cache_i = (cache_i + 1) % cache.size();
        return ret;
    }

    t_wav_read_cached(const char *path, bool auto_rewind = true, unsigned cache_sz = 256):
        t_wav_file_reader(path, auto_rewind),  //fs irelevantni - rozhoduje cetnost vycitani
        cache(cache_sz)
    {
        cache_i = 0;
    }

    virtual ~t_wav_read_cached(){}
};

class t_rt_wavinput : public t_rt_generator {

private:
    t_wav_read_cached file;

public slots:

    virtual void on_start(int p){

        file.reset();
        t_rt_generator::on_start(p);
    }

public:
    t_rt_wavinput(const QString &js_config, const char *path = "in.wav", QObject *parent = NULL):
      t_rt_generator(js_config, std::bind(&t_wav_read_cached::sample, &file, std::placeholders::_1), parent),
//      t_rt_generator(js_config, [](double t){ return t; }, parent),
      file(path)
    {
        t_wav_file_reader::t_wav_header h;
        file.info(h);
        fs = h.sample_frequency;
    }

    virtual ~t_rt_wavinput(){}
};

#endif // RT_WAVINPUT_H
