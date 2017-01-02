#ifndef FILTER_AVR_H
#define FILTER_AVR_H

#include "filter_a.h"
#include "f_windowing.h"

template <class T> class t_filter_avr : public a_filter<T> {

protected:
    using a_filter<T>::data;
    using a_filter<T>::num;
    using a_filter<T>::den;
    using a_filter<T>::counter;
    using a_filter<T>::prev;
    using a_filter<T>::decimationf;
    using a_filter<T>::typef;
    using a_filter<T>::par;

public:
    enum {
        EXPO_LIN,
        LIN_AVER,
        AUTO,
        DC_NONLIN
    } e_filter_avr;

    e_filter_avr mode;

    virtual T *proc(const T &feed, unsigned *count = NULL){

        int N = (num.size() < data.size()) ? num.size() : data.size();  //teor. muze byt delka ruzna

        data[counter++ % data.size()] = feed;
        if(count) *count = counter;

        if(0 == (counter % decimationf)){

            prev = 0;
            for(int i=0; i<N; i++){

                T v_i = data[(counter + i) % data.size()];
                prev += num[num.size() - i] * v_i;   //nejstarsi data nasobime nejvyssimi koeficienty
            }

            return &prev;
        }

        return NULL;  //pokud vypocet neprobihal (kvuli decimaci) vracime null
    }

    /* modifikace prumerovaci doby
     * */
    void tune(double TA){

    }

    t_filter_avr(t_tf_props _par, e_filter_avr _mode, int32_t _decimationf = 1) :
        decimationf(_decimationf)
    {
        switch(mode){

        }

        typef = a_filter<T>::AVERAGING;
    }

    virtual ~t_filter_avr(){;}
};
#endif // FILTER_AVR_H
