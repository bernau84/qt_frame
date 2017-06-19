#ifndef RT_PROCESSOR_H
#define RT_PROCESSOR_H

#include "filter_avr.h"
#include "filter_fir.h"
#include "rt_base_a.h"
#include <QElapsedTimer>

class t_rt_filter;

class t_rt_filter_ex : public i_rt_exchange {

private:
    friend class t_rt_filter;

    QVector<double> a;
    double t0;
    double t1;
    double f;

    /*! length of individual array */
    virtual int n(){ return a.size(); }

    /*! indexed acces */
    virtual double f_t(unsigned i){ return (t0 + (i+1)*(t1 - t0)); }
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
    int fs;
    int ms_period;
    int fc;

private:
    a_filter<double> *m_filter;
    QVector<double>::Iterator xi;
    t_rt_filter_ex *m_data;

    a_filter<double> *_filter_factory(const QString &props)
    {
        t_tf_props props = f_str2tf(props.toLatin1().constData());

        a_filter::e_type ftype = a_filter::FIR_DIRECT1;
        e_win fwin = WHANN;
        int ford = 64;
        int fdecim = 1;

        if(props.find(s_wf_FILTER) != props.end()) ftype = props[s_wf_FILTER];
        if(props.find(s_wf_WINDOW) != props.end()) fwin = props[s_wf_WINDOW];

        switch(ftype)
        {
            case a_filter::FIR_DIRECT1:
                return new t_filter_wfir(fwin, ford, props, fdecim);
            break;
            case a_filter::AVERAGING:
                /*! \todo */
            break;
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

        int i_fs = round(1.0 / (p->f_t(1) - p->f_t(0)));
        if(fs != i_fs){
            on_start(0);  //zmena vzorkovacky - reset filtru
            fs = i_fs;
        }

        if(p->n() == 0)
            return 0;

        const double *a = p->f_a();
        QVector<double> d;
        if(NULL == a){

            d.resize(p->n());  //floatovy buffer si musime vyrobit
            for(int i=0; i < p->n(); i++) d[i] = p->f_a(i);
            a = d.constData();
        }

        for(int i=0; i<p->n(); i++){

            unsigned ntotal = 0;
            double *out = m_filter->proc(*a++, &ntotal);
            if(out){  // != 0 refresh

                if(m_data == NULL)
                    if((m_data = new t_rt_filter_ex())){  //alokujem novy buffer - stary se uvolni diky SharedPnt

                        m_data->a.resize((fs * ms_period) / 1000);
                        m_data->f = fs; //fc;
                        m_data->t1 = m_data->t0 = (1.0 * ntotal) / fs;
                        m_data->t1 += 1.0 / fs;
                        xi = m_data->a.begin();
                    }

                *xi++ = *out;
                if(xi != m_data->a.end())
                    continue;

                QSharedPointer<i_rt_exchange> pp(m_data);
                LOG(INFO) << m_data->t0/1000.0 << "[s]/"
                          << m_data->a.size() << "samples filtered-out";
                //          << "dbg" << m_data->a;

                emit update(pp);
                m_data = NULL;
            }
        }

        return 1;
    }

public slots:

    virtual void on_start(int p){

        Q_UNUSED(p);
        m_filter->reset();

        if(m_data)
            delete(m_data);
        m_data = NULL;
    }

    virtual void on_stop(int p){

        Q_UNUSED(p);
    }

    virtual void on_change(){


    }

public:
    t_rt_filter(const QString &js_config, a_filter<double> &_f_filter, QObject *parent = NULL):
      a_rt_base(js_config, parent),
      m_filter(&_f_filter)
    {
        fs = 0;
        ms_period = 50;
        fc = (*m_filter)[s_wf_f_ce]; //neni vyplneno, bude tam 0 jao by slo o low pass
        m_data = NULL;
    }

    t_rt_filter(const QString &js_config, QObject *parent = NULL):
      a_rt_base(js_config, parent),
      m_filter(_filter_factory(par["properties"].get().toString()))
    {
        fs = 0;
        ms_period = 50;
        fc = m_filter[s_wf_f_ce]; //neni vyplneno, bude tam 0 jao by slo o low pass
        m_data = NULL;
    }

    virtual ~t_rt_filter(){}
};

#endif // RT_PROCESSOR_H
