#ifndef FILTER_AVR_H
#define FILTER_AVR_H

#include "filter_a.h"
#include "f_windowing.h"

template <class T> class t_filter_avr : public a_filter<T> {

public:
    enum e_filter_avr {
        EXPO,
        LIN,
        AUTO,
        DC_REMOVAL_NONLIN
    };

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

private:
    void average(const T &feed)
    {
        if(modef == DC_REMOVAL_NONLIN)
        {
            if(counter == 1)
            {
                prev = feed; //init
                return;
            }
            prev += (prev < feed) ? num[0] : -num[0];
            return;
        }

        //update coef for next sample
        if(((modef == AUTO) && (counter >= num[0])) //po odzaseni prumerovaci doby prechazime na exponencialni prumer
                || (modef == EXPO)){

            prev += (feed - prev) / num[0];
            return;
        }

        if(modef == LIN)
        {
            prev += (feed - prev) / counter;
            return;
        }
    }

public:
    virtual const T *proc(const T &feed, unsigned *count = NULL){

        if(count) *count = ++counter;

        if(decimationf < 0)
        {   //intepolace
            data.clear();
            for(unsigned i = 0; i > decimationf; i--)
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

    /* modifikace prumerovaci doby - bezrozmerne
     * */
    void tune(double _TA){

        num = std::vector<T>(1);
        num[0] = _TA;
    }

    /* modifikace obecne
     * */
    void tune(const t_tf_props &p)
    {
        num[0] = 1000; //1/8 pri 8000Hz
        if((p.end() != p.find(s_wf_TA)) &&
            (p.end() != p.find(s_wf_fs)))
        {
            par = p;
            num[0] = (par[s_wf_TA] / 1000000.0) * par[s_wf_fs];
        }
    }

    t_filter_avr(e_filter_avr _mode, int _TA, int32_t _decimationf = 1) :
        a_filter<T>(NULL, NULL, 0, _decimationf),
        modef(_mode)
    {
        typef = a_filter<T>::AVERAGING;
        tune(_TA); //ma vyznam jen pro expo, nebo auto
        a_filter<T>::reset();  //inicializuje lin, nonlin, a auto
    }


    virtual ~t_filter_avr(){;}
};

#endif // FILTER_AVR_H
