#ifndef RT_FILTER_H
#define RT_FILTER_H

#include "filter_avr.h"
#include "filter_fir.h"
#include "filter_shifter.h"
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

public:
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

    a_filter<double> *_filter_factory(const QString &s_props)
    {
        t_tf_props props = f_str2tf(s_props.toLatin1().constData());

        //defaults
        a_filter<double>::e_type ftype = a_filter<double>::FIR_DIRECT1;
        e_win fwin = WHANN;
        int ford = 64;
        int fdecim = 1;
        t_filter_avr<double>::e_filter_avr fmode;
        int fTA = 0;

        //custom par
        if(props.find(s_wf_RES) != props.end()) fdecim = props[s_wf_RES];
        if(props.find(s_wf_FILTER) != props.end()) ftype = (a_filter<double>::e_type)props[s_wf_FILTER];
        if(props.find(s_wf_WINDOW) != props.end()) fwin = (e_win)props[s_wf_WINDOW];
        if(props.find(s_wf_AVR) != props.end()) fmode = (t_filter_avr<double>::e_filter_avr)props[s_wf_AVR];
        if(props.find(s_wf_TA) != props.end()) fTA = props[s_wf_TA];
        if(props.find(s_wf_N) != props.end()) ford = props[s_wf_N];

        //fabrique
        switch(ftype)
        {
            case a_filter<double>::FIR_DIRECT1:
                return new t_filter_wfir<double>(ford, fwin, props, fdecim);
            case a_filter<double>::AVERAGING:
                return new t_filter_avr<double>(fmode, fTA, fdecim);
            case a_filter<double>::SHIFTER:
                return new t_filter_shift<double>(ford, props, fdecim);
            default: break;
        }

        return NULL;
    }

    /*! init privates from configuration */
    virtual int reload(int p){

        Q_UNUSED(p);
        QString sprop = par["properties"].get().toString();
        if(m_filter) m_filter->tune(sprop.toLatin1().constData());
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
            const double *out = m_filter->proc(*a++, &ntotal);
            //const double *out = a++; //bypass for debug
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
                LOG(INFO) << m_data->t0 << "[s]/"
                          << m_data->a.size()
                          << "samples filtered-out";

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
        if(m_filter) fc = (*m_filter)[s_wf_f_ce]; //neni vyplneno, bude tam 0 jao by slo o low pass
        m_data = NULL;
    }

    virtual ~t_rt_filter(){}
};

#endif // RT_FILTER_H
