#ifndef RT_GENERATOR_H
#define RT_GENERATOR_H

#include "rt_base_a.h"
#include <QElapsedTimer>

class t_rt_generator;

class t_rt_generator_ex : public i_rt_exchange {

private:
    friend class t_rt_generator;

    QVector<double> a;
    double t;
    double f;

    /*! length of individual array */
    virtual int n(){ return a.size(); }

    /*! indexed acces */
    virtual double f_t(unsigned i){ return (f > 0) ? (t + ((i+1) / f)) : t; }
    virtual double f_a(unsigned i){ return (1.0 * a[i]) / (1 << 15); }
    virtual double f_f(unsigned i){ i = i; return f; }

    /*! pointer to array */
    virtual const double *f_a(){ return a.data(); }
    virtual const double *f_f(){ return NULL; }
    virtual const double *f_t(){ return NULL; }

    t_rt_generator_ex(){}
    virtual ~t_rt_generator_ex(){
        // LOG(TRACE) << "released";
    }
};


class t_rt_generator : public a_rt_base {

    Q_OBJECT

protected:
    double  fs;
    double  ms_period;
    double  ms_proc;

private:
    std::function<double(double)> f_sample;
    QElapsedTimer elapsed;
    QVector<double> x;
    QVector<double>::Iterator xi;
    t_rt_generator_ex *m_data;
    int     id_timer;

    /*! init privates from configuration */
    virtual int reload(int p){
        Q_UNUSED(p);
        return 0;
    }

    /*! here do the work */
    virtual int proc(i_rt_exchange *p){
        Q_UNUSED(p);
        double ms_now = elapsed.elapsed();
        double ms_per = 1000 / fs;
        while((ms_proc + ms_per) < ms_now){ //inkrementujeme dokud sme ve vzorkovacim okne

            *xi++ = f_sample(ms_proc/1000);  //lambda generator
            ms_proc += ms_per;
            if(xi != x.end())
                continue;

            xi = x.begin();   //mame plny buffer
            if((m_data = new t_rt_generator_ex())){  //alokujem novy buffer - stary se uvolni diky SharedPnt

                m_data->a = x;
                m_data->f = fs / 2;  //frekvence vzorku je nyquistovka
                m_data->t = ms_proc - ms_per*x.size();  //znacka 1-ho vzorku
                QSharedPointer<i_rt_exchange> pp(m_data);
                LOG(INFO) << m_data->t << "[s]/" << x.size() << "samples out";
                emit update(pp);
            }
        }

        int cached = xi - x.begin();
        if(cached) LOG(INFO) << (ms_proc - ms_per*cached) << "[s]/" << cached << "samples cached";
        return cached;
    }

    void timerEvent(QTimerEvent *event){
        Q_UNUSED(event);
        proc(NULL);
    }

public slots:

    virtual void on_start(int p){

        Q_UNUSED(p);
        id_timer = startTimer(ms_period);
        x.resize(fs / ms_period);  //reserve nefunguje jak potrebujem
        xi = x.begin();
        elapsed.start();
        ms_proc = 0;
    }

    virtual void on_stop(int p){

        Q_UNUSED(p);
        killTimer(id_timer);
    }

public:
    t_rt_generator(const QString &js_config, std::function<double(double t)> _f_sample, QObject *parent = NULL):
      a_rt_base(js_config, parent),
      f_sample(_f_sample)
    {
        //by default - v ramci on-start z konfigu do budoucna
        ms_period = 50;
        fs = 8000;
        ms_proc = 0;
    }

    virtual ~t_rt_generator(){}
};


#endif // RT_GENERATOR_H
