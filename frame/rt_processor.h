#ifndef RT_PROCESSOR_H
#define RT_PROCESSOR_H

#include "rt_base_a.h"
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonValue>

class t_rt_processor;

//podobne jako rt_filter, jen nejde o konvolucni pristup
//ale spis korelacni, autokorelacni, vypocty vykonu a jine

//stejny s t_rt_filter_ex, jen friend je to vuci jine tride
class t_rt_processor_ex : public i_rt_exchange {

private:
    friend class t_rt_processor;

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

    t_rt_processor_ex(){}
    virtual ~t_rt_processor_ex(){
        // LOG(TRACE) << "released";
    }
};

class t_rt_processor : public a_rt_base {

    Q_OBJECT

protected:
    int fs;
    int ms_period;
    int fc;
    uint64_t ntotal;
    std::function<double(double t, double a)> f_proc;

private:
    QVector<double>::Iterator xi;
    t_rt_processor_ex *m_data;

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

            double out = f_proc(*a++, p->f_t(ntotal++));
            //out = a++; //bypass for debug

            if(m_data == NULL)
                if((m_data = new t_rt_processor_ex())){  //alokujem novy buffer - stary se uvolni diky SharedPnt

                    m_data->a.resize((fs * ms_period) / 1000);
                    m_data->f = fs; //fc;
                    m_data->t1 = m_data->t0 = (1.0 * ntotal) / fs;
                    m_data->t1 += 1.0 / fs;
                    xi = m_data->a.begin();
                }

            *xi++ = out;
            if(xi != m_data->a.end())
                continue;

            QSharedPointer<i_rt_exchange> pp(m_data);
//            LOG(INFO) << m_data->t0 << "[s]/"
//                      << m_data->a.size()
//                      << "samples processed-out";

            emit update(pp);
            m_data = NULL;
        }

        return 1;
    }

public slots:

    virtual void on_start(int p){

        Q_UNUSED(p);

        if(m_data)
            delete(m_data);
        m_data = NULL;
    }

    virtual void on_stop(int p){

        Q_UNUSED(p);
    }

public:
    t_rt_processor(const QString &js_config, std::function<double(double t, double a)> _f_proc, QObject *parent = NULL):
        a_rt_base(js_config, parent),
        f_proc(_f_proc)
    {
        fs = 0;
        ms_period = 50;
        m_data = NULL;
    }

    virtual ~t_rt_processor(){}
};

class t_rt_harm_correlator : public t_rt_processor
{

    Q_OBJECT

protected:
    using t_rt_processor::fs;

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
    t_rt_harm_correlator(const QString &js_config, QObject *parent = NULL):
      t_rt_processor(js_config, std::bind(&t_rt_harm_correlator::a, this, std::placeholders::_1), parent)
    {
        f_n = {1000}; //{1000, 2000};
        A_n = {0.99}; //{0.3, 0.2};
        fs = 8000;

        if(par.ask("f_n")) f_n = par["f_n"].get().toArray();
        if(par.ask("A_n")) A_n = par["A_n"].get().toArray();
        if(par.ask("fs")) fs = par["fs"].get().toInt();
    }

    virtual ~t_rt_harm_correlator(){}
};

#endif // RT_PROCESSOR_H
