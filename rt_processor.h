#ifndef RT_PROCESSOR_H
#define RT_PROCESSOR_H


#include "filter_a.h"
#include "rt_base_a.h"
#include <QElapsedTimer>

class t_rt_filter;

class t_rt_filter_ex : public i_rt_exchange {

private:
    friend class t_rt_filter;

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

    t_rt_filter_ex(){}
    virtual ~t_rt_filter_ex(){
        // LOG(TRACE) << "released";
    }
};


class t_rt_filter : public a_rt_base {

    Q_OBJECT

protected:
    double  fs;
    double  ms_period;
    double  ms_proc;

private:
    a_filter<double> &m_filter;
    QElapsedTimer elapsed;
    QVector<double> x;
    QVector<double>::Iterator xi;
    t_rt_filter_ex *m_data;

    void _filter(double *a, int n){

        for(int i=0; i<n; i++)
            if(0 == m_filter.process(*a++)){

                //write to output
                if(m_data == NULL){

                    m_data = new t_rt_filter_ex()
                }
            }
    }

    /*! init privates from configuration */
    virtual int reload(int p){
        Q_UNUSED(p);
        return 0;
    }

    /*! here do the work */
    virtual int proc(i_rt_exchange *p){

        if(!p)
            return 0;

        int i_fs = p->f_f(0) * 2; //z nyguista na fs + zaokrouhleni na cele Hz
        if(fs != i_fs)
            on_stop(0);  //zmena vzorkovacky - musime zalozit novy soubor

        if(p->n() == 0)
            return 0;

        double *a = p->f_a();
        if(NULL == a){

            QVector<double> d(p->n());  //floatovy buffer si musime vyrobit
            for(int i=0; i < p->n(); i++) d[i] = p->f_a(i);
        }



        return file->write(d.constData(), p->n());
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
        m_filter.reset();
    }

public:
    t_rt_filter(const QString &js_config, std::function<double(double t)> _f_sample, QObject *parent = NULL):
      a_rt_base(js_config, parent),
      f_sample(_f_sample)
    {
        //by default - v ramci on-start z konfigu do budoucna
        ms_period = 50;
        fs = 8000;
        ms_proc = 0;
    }

    virtual ~t_rt_filter(){}
};

#endif // RT_PROCESSOR_H
