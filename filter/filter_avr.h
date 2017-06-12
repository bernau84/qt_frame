#ifndef FILTER_AVR_H
#define FILTER_AVR_H

#include "filter_a.h"
#include "f_windowing.h"

template <class T> class t_filter_expo_avr : public a_filter<T> {

protected:
    using a_filter<T>::data;
    using a_filter<T>::num;
    using a_filter<T>::den;
    using a_filter<T>::counter;
    using a_filter<T>::prev;
    using a_filter<T>::decimationf;
    using a_filter<T>::typef;
    using a_filter<T>::par;

    e_filter_avr modef;
    double TA;

public:
    enum {
        EXPO,
        LIN,
        AUTO,
        DC_REMOVAL_NONLIN
    }  e_filter_avr;

private:
    void average(const T &feed)
    {
        prev = prev*num[1] + feed*num[0]; //analogie s fir N = 1 akorat adaptivni (LIN varianty)

        if(modef == AUTO)
        {
            if(TA >= num[0]){ //prechazime na exponencialni prumer

                num[1] = (TA - 1) / TA;
                num[0] = 1 / TA;
                return;
            }
        }

        if(modef == LIN)
        {
            num[1] = (1.0 * counter) / (counter + 1);
            num[0] = 1.0 / counter;
            return;
        }

        if(DC_REMOVAL_NONLIN)
        {
            num[0] = 0;
            num[1] = (feed > prev) ? (1 + 1/prev) : (1 - 1/prev);  //+1 nebo -1 v pristim kroku
            return;
        }
    }

public:
    virtual const T *proc(const T &feed, unsigned *count = NULL){

        if(count) *count = ++counter;

        if(decimationf < 0)
        {   //intepolace
            data.clear();
            for(int i = 0; i > decimationf; i--)
            {  //uplne stupidne a asi neee dobre - vzdyt TA mam napocitanou na nejakou fs
                //urcite ale funguje pro TA = 1 (interpolace skokem)
                average(feed);
                data.push_back(prev);
            }
            return data.data();
        }

        if(0 == (counter % decimationf))
        {
            average(feed);
            return &prev;
        }

        return NULL;  //pokud vypocet neprobihal (kvuli decimaci) vracime null
    }

    void reset(const T def = T(0))
    {
        a_filter::reset(def);
        if(modef == DC_REMOVAL_NONLIN)
        {
            num.clear();
            num.push_back(1);  //num[0]
            num.push_back(0);  //num[1]
        }
        if((modef == LIN) || (modef == AUTO))
        {
            num.clear();
            num.push_back(-1);  //num[0]
            num.push_back(1);  //num[1]
        }
    }

    /* modifikace prumerovaci doby - bezrozmerne
     * */
    void tune(double _TA){

        TA = _TA;
        if(modef == EXPO)
        {
            num.clear();
            num.push_back(1 / TA);  //num[0]
            num.push_back((TA - 1) / TA);  //num[1]
        }
    }

    /* modifikace obecne
     * */
    void tune(const t_tf_props &p)
    {
        TA = 1000; //1/8 pri 8000Hz
        if((p.end() != p.find(s_wf_TA)) &&
            (p.end() != p.find(s_wf_fs)))
        {
            par = p;
            TA = (p[s_wf_TA] / 1000000.0) * p[s_wf_fs];
        }
        tune(TA);
    }

    t_filter_avr(e_filter_avr _mode, double _TA, int32_t _decimationf = 1) :
        t_filter_fir<T>(NULL, NULL, 0, _decimationf),
        modef(_mode)
    {
        typef = a_filter<T>::AVERAGING;
        tune(_TA); //ma vyznam jen pro expo, nebo auto
        reset();  //inicializuje lin, nonlin, a auto
    }


    virtual ~t_filter_avr(){;}
};

#endif // FILTER_AVR_H
