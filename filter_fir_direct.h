#include "filter_a.h"
#include "f_windowing.h"

template <class T> class t_filter_fir_direct : public a_filter<T> {

	using a_filter<T>::data;
	using a_filter<T>::num;
	using a_filter<T>::den;
	using a_filter<T>::proc;
	using a_filter<T>::decimationf;
    using a_filter<T>::m_type;

    public:
        virtual T process(const T &feed, unsigned *countdown = &m_countdown){

            prev = 0;
            int N = (num.size() < data.size()) ? num.size() : data.size();  //teor muze byt delka ruzna

            data[proc % data.size()] = feed;

            if(0 == (countdown = (proc++ % decimationf))){

                for(unsigned i=0; i<N; i++){

                    T v_i = data[(proc + i) % data.size()];
                    prev += num[num.size() - i] * v_i;   //nejstarsi data nasobime nejvyssimi koeficienty
                }
            }

            return prev;  //pokud vypocet neprobihal (kvuli decimaci, vracime posledni vysledek)
        }

        /* frekvencni posuv fir charakteristiky
         * nasobeni v case vede na konvoluci ve spektrech
         */
        static void fshift(double f, std::vector<T> &in){

            for(int i=0; i<in.size(); i++)
                in[i] *= cos(2*M_PI*f*i);
        }

        /* modifikace filtru posunem
         * */
        void tune(double f){

            fshift(f, num);
        }

        /* becna modifikace za behu
         * */
        void tune(const a_filter<T> &coe, int32_t _decimationf = 1){

            num = coe;   //neresetujem - muze se lisit i delka; nic se nedeje, v pameti je jen delay line
            decimationf = _decimationf;
        }

        t_filter_fir_direct(const a_filter<T> &src)
                       :a_filter<T>(src){

            m_type = a_filter<T>::FIR_DIRECT1;
        }

        t_filter_fir_direct(const T *_num, int32_t _N, int32_t _decimationf = 1)
                       :a_filter<T>(_num, NULL, _N, _decimationf){

            m_type = a_filter<T>::FIR_DIRECT1;
        }

        virtual ~t_filter_fir_direct(){;}
};


template <class T> class t_filter_wfir : public t_filter_fir_direct {

    using a_filter<T>::data;
    using a_filter<T>::num;
    using a_filter<T>::den;
    using a_filter<T>::proc;
    using a_filter<T>::decimationf;
    using a_filter<T>::m_type;

private:
    e_win  m_win;
    int m_N;

public:
        /* vygeneruje inpulsni odezvu idelaniho low pass fir filtru delky N a mezni frekvence fm
         * pomoci okenkovani fce sinc jakozto idelani LP odezvy
         * nenormalizuje
         *
         * staticka proto aby se dala pouzit i jinde (pro vypocet sync())
         */
        static std::vector<T> fdesign(double fm, int N, e_win w = RECT){

            if(0 == (1&N)) N += 1; //zjednoduseni - N musi byt liche
            std::vector<T> out(N);

            out[N/2] = B; //napocitame idealni filtr
            for(int i=0; i<N/2; i++){

                T a = 2*M_PI*fm*(N/2 - i);
                out[i] = out[N - i - 1] = sin(a) / (M_PI * a);
            }

            //aplikace okynka
            for(int i=0; i<N; i++) out[i] *= f_win(i, N, w);  //from f_windovin

            return out;
        }

        /* zmena za behu 
         * fs [Hz] sampling freq 
         * G [dB] gain (== normalization)
         * B [Hz] (0, fs) sirka pasma
         * fc [Hz] (0, f_nyq) centralni frekvence
         **/
        void redesign(t_tf_props &par){

            double fm = 1.0; //default - udelame half band
            double A = 1.0;

            int B = par[s_wf_B];
            int G = par[s_wf_G];        
            int fs = par[s_wf_fs];
            int fc = par[s_wf_F_central];

            if(B && fs) fm = (1.0 * B/2) / fs;  //mezni fr je polovinou B u ideal LP
            if(G) A = pow(10, G/20.0);

            num = fdesign(fm, N, w);
            if(fc > 1e-10) fshift(fc, num);
            for(int i=0; i<N; i++) out[i] *= A;  //gain          
        }

        void redesign(const char *config = "#B=0.5#fs=1"){

            t_tf_props par = f_str2tf(config);
            redesign(par);
        }

        t_filter_wfir(const a_filter<T> &src)
                       :a_filter<T>(src)
        {
            m_type = a_filter<T>::FIR_DIRECT1;
        }

        /* v par musi byt je celkova sirka pasma B <0, 1> kde 0.5 je idealni half pass
         * pozor B se zadava jako idealni LP takze musi byt 2*fm (mezni frekvence)
         * fc je centralni frekvence pro realizaci bp, resp hp
         **/
        t_filter_wfir(int32_t N, e_win w, t_tf_props &par, int32_t _decimationf = 1)
                       :t_filter_fir_direct<T>(NULL, N, _decimationf),
                        m_win(w),
                        m_N(N)
        {
            redesign(par);
            m_type = a_filter<T>::FIR_DIRECT1;
        }

        t_filter_wfir(int32_t N, e_win w, const char *config = "#B=0.5#fs=1", int32_t _decimationf = 1)
                       :t_filter_fir_direct<T>(NULL, N, _decimationf),
                        m_win(w),
                        m_N(N)
        {
            redesign(config);
            m_type = a_filter<T>::FIR_DIRECT1;
        }

        virtual ~t_filter_wfir(){;}
};
