#ifndef RT_GENERATOR_H
#define RT_GENERATOR_H

#include "rt_base_a.h"
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonValue>

class t_rt_generator;

class t_rt_generator_ex : public i_rt_exchange {

private:
    friend class t_rt_generator;

    QVector<double> a;
    double t0;
    double t1;
    double f;

    /*! length of individual array */
    virtual int n(){ return a.size(); }

    /*! indexed acces */
    virtual double f_t(unsigned i){ return (t0 + (i+1)*(t1 - t0)); }
    virtual double f_a(unsigned i){ return a[i]; }
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
    int  fs;
    int  ms_period;
    double  ms_proc;
    std::function<double(double)> f_sample;

private:
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
        double ms_per = 1000.0 / fs;
        while((ms_proc + ms_per) < ms_now){ //inkrementujeme dokud sme ve vzorkovacim okne

            *xi++ = f_sample(ms_proc/1000);  //lambda generator
            ms_proc += ms_per;
            if(xi != x.end())
                continue;

            xi = x.begin();   //mame plny buffer
            if((m_data = new t_rt_generator_ex())){  //alokujem novy buffer - stary se uvolni diky SharedPnt

                m_data->a = x;
                m_data->f = fs / 2;  //frekvence vzorku je nyquistovka
                m_data->t1 = m_data->t0 = (ms_proc - ms_per*x.size())/1000.0;  //znacka 1-ho vzorku
                m_data->t1 += ms_per/1000;

                QSharedPointer<i_rt_exchange> pp(m_data);
//                LOG(INFO) << m_data->t0 << "[s]/"
//                          << x.size() << "samples out";

                emit update(pp);
            }
        }

        int cached = xi - x.begin();
//        if(cached) LOG(INFO) << (ms_proc - ms_per*cached)/1000.0
//                             << "[s]/" << cached << "samples cached";
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
        if(id_timer)
        {
            killTimer(id_timer);
            id_timer = 0;
        }
        else
        {
            LOG(ERROR) << "timer not running!";
        }
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

class t_rt_sweep_generator : public t_rt_generator {

    Q_OBJECT

protected:
    using t_rt_generator::fs;

private:
    int f_0;
    int f_1;
    int T;

    double a(double t)
    {
        t = fmod(t, T);
        double fi = t*f_0 + t*t/2*(f_1 - f_0)/T; //faze pro lin preladeni
        return 0.5 * cos(2*M_PI*fi);
    }

public:
    t_rt_sweep_generator(const QString &js_config, QObject *parent = NULL):
      t_rt_generator(js_config, std::bind(&t_rt_sweep_generator::a, this, std::placeholders::_1), parent)
    {
        f_0 = 100;
        f_1 = 3800;
        fs = 8000;
        T = 10000;

        if(par.ask("f_0")) f_0 = par["f_0"].get().toInt();
        if(par.ask("f_1")) f_1 = par["f_0"].get().toInt();
        if(par.ask("T")) T = par["T"].get().toInt();
        if(par.ask("fs")) fs = par["fs"].get().toInt();
    }

    virtual ~t_rt_sweep_generator(){}
};

class   t_rt_multisin_generator : public t_rt_generator
{

    Q_OBJECT

protected:
    using t_rt_generator::fs;

private:
    QJsonArray f_n;
    QJsonArray A_n;

    double a(double t)
    {
        double a_t = 0;
        double fi = 2*M_PI*t;

        for(int i=0; i<f_n.size(); i++)
        {
            //90% plneni predpoklad a vsechny amp stejne
            double A = (i < A_n.size()) ? A_n[i].toDouble() : 0.9/f_n.size();
            a_t += A * cos(fi * f_n[i].toInt());
        }

        return a_t;
    }

public:
    t_rt_multisin_generator(const QString &js_config, QObject *parent = NULL):
      t_rt_generator(js_config, std::bind(&t_rt_multisin_generator::a, this, std::placeholders::_1), parent)
    {
        f_n = {100, 250, 400, 1000, 2000, 3000}; //{1000, 2000};
        A_n = {0.3, 0.2, 0.1, 0.08, 0.05, 0.02}; //{0.3, 0.2};
        fs = 8000;

        if(par.ask("f_n")) f_n = par["f_n"].get().toArray();
        if(par.ask("A_n")) A_n = par["A_n"].get().toArray();
        if(par.ask("fs")) fs = par["fs"].get().toInt();
    }

    virtual ~t_rt_multisin_generator(){}
};

#endif // RT_GENERATOR_H
