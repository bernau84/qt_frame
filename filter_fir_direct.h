#include "filter_a.h"

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


template <class T> class t_filter_fir_windowed : public t_filter_fir_direct {

    using a_filter<T>::data;
    using a_filter<T>::num;
    using a_filter<T>::den;
    using a_filter<T>::proc;
    using a_filter<T>::decimationf;
    using a_filter<T>::m_type;

public:
    enum e_win {
        HANN = 0x10,
        HAMM = 0x11,
        FLAT = 0x12,
        BLCK = 0x14,
        RECT = 0x18,
        BRTL = 0x20,
        GAUS = 0x21
    };

private:
    e_win  m_win;

public: /* staticke metody pro pouziti mimo instanci
         */

        /* vraci vzorek okencovaci funkce
         */
        static T fwin(e_win w, int i, int N, T par = T(0)){

            double temp;
            switch (w){

               case HANN:        //w = 1 - cos(2*pi*(0:m-1)'/(n-1)));
                         return 2*(0.5 - 0.5*cos( 2*M_PI*i/N ));
               case HAMM:        //w = (54 - 46*cos(2*pi*(0:m-1)'/(n-1)))/100;
                         return 2*(0.54 - 0.46*cos( 2*M_PI*i/N ));
               case BLCK:        //w = (42 - 50*cos(2*pi*(0:m-1)/(n-1)) + 8*cos(4*pi*(0:m-1)/(n-1)))'/100;
                         return 2*(0.42 - 0.50*cos( 2*M_PI*i/N ) - 0.08*cos( 4*M_PI*i/N ));
               case RECT:
                         return 1;
               case FLAT:        //w = 1 - 1.98*cos(2*Pi*(0:m-1)/(n-1)) + 1.29*cos(4*Pi*(0:m-1)/(n-1)) - 0.388*cos(6*Pi*(0:m-1)/(n-1)) + 0.0322*cos(8*Pi*t/T);
                         return (1 - 1.98*cos( 2*M_PI*i/N ) + 1.29*cos( 4*M_PI*i/N ) - 0.388*cos( 6*M_PI*i/N ) + 0.0322*cos( 8*M_PI*i/N ));
               case BRTL:
                         return (2 - fabs(4*(i-N/2)/N));
               case GAUS:         //w = exp(-((-Nw/2:(Nw/2-1))/(Sigma*Nw)).^2);
                         temp = (i-N/2.0)/(par*N);    //zpocitame si predem exponent
                         return exp(-(temp*temp));
            }

            return 0;
        }

        /* vygeneruje inpulsni odezvu fir filtru delky N a sirky pasma B
         * pomoci okenkovani fce sinc jakozto idelani LP odezvy
         */
        static std::vector<T> fdesign(T B, int N, e_win w = RECT){

            T fm = B/2;  //vychozim stavem je LP
            if(0 == (1&N)) N += 1; //zjednoduseni - N musi byt liche
            out = std::vector<T>(N);

            out[N/2] = B; //napocitame idealni filtr
            for(int i=0; i<N/2; i++){

                T a = 2*M_PI*fm*(N/2 - i);
                out[i] = out[N - i - 1] = sin(a) / (M_PI * a);
            }

            //aplikace okynka
            for(int i=0; i<N; i++)
                out[i] *= fwin(w, i, N);

            return out;
        }

        t_filter_fir_windowed(const a_filter<T> &src)
                       :a_filter<T>(src)
        {
            m_type = a_filter<T>::FIR_DIRECT1;
        }

        /* B je celkova sirka pasma <0, 2> kde 1 je idealni half pass
         * shift je posunuti v digitalni frekbence f/fs
         * */
        t_filter_fir_windowed(int32_t N, e_win w, double B = 1, double shift = 0, int32_t _decimationf = 1)
                       :t_filter_fir_direct<T>(NULL, N, _decimationf),
                        m_win(w)
        {
            num = fdesign(B, N, w);
            fshift(shift, num);
            m_type = a_filter<T>::FIR_DIRECT1;
        }

        virtual ~t_filter_fir_direct(){;}
};
