#include "filter_a.h"
#include "f_windowing.h"

template <class T> class t_filter_fir : public a_filter<T> {

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
    virtual T *proc(const T &feed, unsigned *count = NULL){

        int N = (num.size() < data.size()) ? num.size() : data.size();  //teor. muze byt delka ruzna

        data[counter++ % data.size()] = feed;
        if(count) *count = counter;

        if(0 == (counter % decimationf)){

            prev = 0;
            for(int i=0; i<N; i++){

                T v_i = data[(counter + i) % data.size()];
                prev += num[num.size() - i - 1] * v_i;   //nejstarsi data nasobime nejvyssimi koeficienty
            }

            return &prev;
        }

        return NULL;  //pokud vypocet neprobihal (kvuli decimaci) vracime null
    }

    /* frekvencni posuv fir charakteristiky
     * nasobeni v case vede na konvoluci ve spektrech
     */
    static void fshift(double f, std::vector<T> &in){

        for(unsigned i=0; i<in.size(); i++)
            in[i] *= cos(2*M_PI*f*i);
    }

    /* modifikace filtru posunem
     * */
    virtual void tune(t_tf_props &p)
    {
        if(p.end() != (it = p.find(s_wf_fs)))
            fshift((double)it->second, num);
    }

    /* obecna modifikace za behu
     * */
    void tune(const a_filter<T> &coe, int32_t _decimationf = 1){

        num = coe;   //neresetujem - muze se lisit i delka; nic se nedeje, v pameti je jen delay line
        decimationf = _decimationf;
    }

    t_filter_fir(const a_filter<T> &src)
                   :a_filter<T>(src){

        typef = a_filter<T>::FIR_DIRECT1;
    }

    t_filter_fir(const T *_num, int32_t _N, int32_t _decimationf = 1)
                   :a_filter<T>(_num, NULL, _N, _decimationf){

        typef = a_filter<T>::FIR_DIRECT1;
    }

    virtual ~t_filter_fir(){;}
};


template <class T> class t_filter_wfir : public t_filter_fir<T> {

protected:
    using t_filter_fir<T>::num;
    using t_filter_fir<T>::typef;
    using t_filter_fir<T>::par;
    using t_filter_fir<T>::counter;

private:
    e_win  m_win;
    int m_N;

public:
    using t_filter_fir<T>::fshift;



    /* vygeneruje inpulsni odezvu idelaniho low pass fir filtru delky N a mezni frekvence fm
     * pomoci okenkovani fce sinc jakozto idelani LP odezvy
     * nenormalizuje
     *
     * staticka proto aby se dala pouzit i jinde (pro vypocet sync())
     */
    static std::vector<T> fdesign(double fm, int N, e_win w = WRECT){

        std::vector<T> out(N);

        //eval sinc(x)
        out[N/2] = fm; //z lHopitalova hodnota v nule (pouzije se pro N licha)
        for(int i=0; i<N/2; i++){ //napocitame ostatni hodnoty idealni lp

            T a = 2*M_PI*fm*(N/2 - i);
            out[i] = out[N - i - 1] = sin(a) / (M_PI * a);
        }

        //aplikace okynka
        for(int i=0; i<N; i++)
            out[i] *= f_win<T>(i, N, w);  //from f_windovin

        return out;
    }

    /* zmena za behu
     * fs [Hz] sampling freq
     * G [dB] gain (== normalization)
     * B [Hz] (0, fs) sirka pasma
     * fc [Hz] (0, f_nyq) centralni frekvence
     **/
    void tune(const t_tf_props &p){

        par = p;  //save

        double fm = 1.0; //default - udelame half band
        double A = 1.0;

        t_tf_props::const_iterator it;

        int B = 0, fs = 1000, G = 0, fc = 0;
        if(p.end() != (it = p.find(s_wf_B))) B = it->second;
        if(p.end() != (it = p.find(s_wf_G))) G = it->second;
        if(p.end() != (it = p.find(s_wf_fs))) fs = it->second;
        if(p.end() != (it = p.find(s_wf_f_ce))) fc = it->second;

        if(B && fs) fm = (1.0 * B/2) / fs;  //mezni fr je polovinou B u ideal LP
        if(G) A = pow(10, G/20.0);

        num = fdesign(fm, m_N, m_win);
        if(fc) fshift((1.0 * fc) / fs, num);
        for(int i=0; i<m_N; i++) num[i] *= A;  //gain
    }

    t_filter_wfir(const a_filter<T> &src)
                   :a_filter<T>(src)
    {
        typef = a_filter<T>::FIR_DIRECT1;
    }

    /* v par musi byt je celkova sirka pasma B <0, 1> kde 0.5 je idealni half pass
     * pozor B se zadava jako idealni LP takze musi byt 2*fm (mezni frekvence)
     * fc je centralni frekvence pro realizaci bp, resp hp
     **/
    t_filter_wfir(int32_t N, e_win w, t_tf_props &_par, int32_t _decimationf = 1)
                   :t_filter_fir<T>(NULL, N, _decimationf),
                    m_win(w),
                    m_N(N)
    {
        tune(par);
    }

    t_filter_wfir(int32_t N, e_win w, const char *config = "#B=500#fs=1000", int32_t _decimationf = 1)
                   :t_filter_fir<T>(NULL, N, _decimationf),
                    m_win(w),
                    m_N(N)
    {
        tune(config);
    }

    virtual ~t_filter_wfir(){;}
};
