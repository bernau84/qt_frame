#ifndef _RT_FILTERA_H_
#define _RT_FILTERA_H_

#include <stdint.h>
#include <stdarg.h>
#include <vector>
#include "f_props.h"

/*! \class template of t_pFilter
 * \brief - pure virtual ancestor of SISO digital filter
 * deep copy of filter coeficient is made
 * filter impementation depends on successor as well as order of coeficinets
 */
template <class T> class a_filter {

    public:

        enum e_type {

            IIR_DIRECT1 = 1,
            IIR_DIRECT2 = 2,
            IIR_LATTICE = 3,
            IIR_BIQUADR = 4,
            FIR_DIRECT1 = 5,
            FIR_DELAYLN = 6,
            FIR_LATTICE = 7,
            FFT_FILTER = 8,
            AVERAGING = 9
        };

    protected:
        std::vector<T> data;
        std::vector<T> num;
        std::vector<T> den;

        unsigned decimationf;     //mozny je jen faktor > 0, 0 funguje jako pauza (vystup se neupdatuje)
        e_type typef;  //typ pro zpetnou identifikaci

        uint64_t counter;     //index zracovaneho vzorku (inkrement)
        T prev;    //zaloha posledniho vysledeku

        t_tf_props par;  //info parameters (B, fc, TA, ..) see f_props.h

    public:

        /*! \brief - indeirect way mofify filter properties via property
         */
        virtual void tune(const t_tf_props &p){

            (void)p; //do nothing, let
        }

        /*! \brief - indirect vai mofify filter properties via string of property
         */
        void tune(const char *config/* = "#B=500#fs=1000"*/){

            t_tf_props par = f_str2tf(config);
            tune(par);
        }

        /*! \brief - defines behaviour of filter,
            n_2_proc == 0 defines valid output in decimation mode
            \todo - may be used to signal valid output after group delay, after filter run-up time/sample
        */
        virtual const T *proc(const T &feed, unsigned *count = NULL) = 0;

        /*! \brief - backward identification */
        e_type type(){ return typef; }

        /*! \brief - return last filtering result (cache for deciamtion purpose) */
        T last(){ return prev; }

        /*! \brief - filter has some info properties */
        bool has_props(){ return !par.empty(); }

        /*! \brief - info properties getter */
        int operator[](const char *name){ return (par.find(name) != par.end()) ?  par[name] : 0; }

        /*! \brief - inicialize shift register
         */
        void reset(const T def = T(0)){

            data.assign(data.size(), def);
            prev = T(0);
            counter = 0;
        }

        /*! \brief - direct common way of mofify filter properties
         */
        void tune(std::vector<T> &_num, std::vector<T> &_den){

            num = _num;
            den = _den;
        }

        /*! \brief - set initial value (to minimize gd for example)
         * preload sample inxed as well (to realize harmonic bank filter)
         */
        void reset(std::vector<T> &def, T iniv = T(0)){

            unsigned i=0;
            for(; i<data.size(); i++)
                if(i<def.size()) data[i] = def[i];
                    else data[i] = T(0.0);

            prev = iniv;
            counter = i;
        }        

        /*! \brief - copy of existing, except delay line */
        a_filter(const a_filter<T> &src):
            data(src.data),
            num(src.num),
            den(src.den),
            decimationf(src.decimationf),
            typef(src.typef)
        {
            counter  = 0;
        }

        /*! \brief - new flter definition (including decimation factor) */
        a_filter(const T *_num, const T *_den, int32_t N, int32_t _decimationf = 1):
            data(0),
            num(0),
            den(0),
            decimationf(_decimationf)
        {
            data = std::vector<T>(N);            
            if(_num) num = std::vector<T>(_num, &_num[N]);
            if(_den) den = std::vector<T>(_den, &_den[N]);
           
            prev = T(0);
            counter = 0;
        }

        /*! \brief - free previsous allocation */
        virtual ~a_filter(){

        }
};

#endif //_RT_FILTERA_H_
