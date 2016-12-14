#include "filter_a.h"

template <class T> class t_filter_fir_direct : public a_filter<T> {

	using a_filter<T>::data;
	using a_filter<T>::num;
	using a_filter<T>::den;
	using a_filter<T>::proc;
	using a_filter<T>::decimationf;

    public:
        T process(const T &new_smpl, unsigned *proc_2_output = &m_countdown){

            if(0 == (proc_2_output = (proc % decimationf))){

                prev = new_smpl * num[0];
                for(unsigned i=1; i<data.size(); i++){

                    T v_i = data[(proc + i - 1) % data.size()];
                    prev += num[num.size() - i] * v_i;   //nejstarsi data nasobime nejvyssimi koeficienty
                }

                //FREQ_RT_FILT_TRACE(1, "fir-proc-tick on %llu, res %f", this->proc, this->prev);
            }

            data[proc++ % data.size()] = new_smpl;
            return prev;  //pokud vypocet neprobihal (kvuli decimaci, vracime posledni vysledek)
        }

        t_filter_fir_direct(const t_pFilter<T> &src)
                       :a_filter<T>(src){

            struction = a_filter<T>::FIR_DIRECT1;
        }

        t_filter_fir_direct(const T *_num, int32_t _N, int32_t _decimationf = 1)
                       :a_filter<T>(_num, NULL, _N, _decimationf){

            struction = a_filter<T>::FIR_DIRECT1;
        }

        virtual ~t_filter_fir_direct(){;}
};

