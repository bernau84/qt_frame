
#include <stdint.h>
#include <stdarg.h>
#include <vector>

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

        unsigned m_countdown;
        unsigned decimationf;     //mozny je jen faktor > 0, 0 funguje jako pauza (vystup se neupdatuje)
        e_type m_type;  //typ pro zpetnou identifikaci

        uint64_t proc;     //index zracovaneho vzorku (inkrement)
        T prev;    //zaloha posledniho vysledeku

    public:

        /*! \brief - defines behaviour of filter,
            n_2_proc == 0 defines valid output in decimation mode
            \todo - may be used to signal valid output after group delay, after filter run-up time/sample
        */
        virtual T process(const T &feed, unsigned *countdown = NULL) = 0;

        /*! \brief - backward identification */
        e_type type(){ return m_type; }

        /*! \brief - return last filtering result (cache for deciamtion purpose) */
        T last(){ return prev; }

        /*! \brief - inicialize shift register
         */
        void reset(const T def = T(0)){

            data.assign(data.size(), def);
            prev = T(0);
            proc = 0;
        }

        /*! \brief -
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
            proc = i;
        }        

        /*! \brief - copy of existing, except delay line */
        a_filter(const a_filter<T> &src):
            data(src.data),
            num(src.num),
            den(src.den),
            decimationf(src.decimationf),
            m_type(src.m_type)
        {
            proc  = 0;
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
            proc = 0;
        }

        /*! \brief - free previsous allocation */
        virtual ~a_filter(){

        }
};

