
#ifndef F_WINDOWINGH
#define F_WINDOWINGH

#include "f_props.h"

/*! \enum supporting windows waveforms
 */
enum e_win {

    WHANN = 0x10,
    WHAMM = 0x11,
    WFLAT = 0x12,
    WBLCK = 0x14,
    WRECT = 0x18,
    WBRTL = 0x20,
    WGAUS = 0x21,
    WUSER = 0x80
};

static t_tf_props fwin_pro_0;  //empty settings

/*! \brief vraci vzorek okynka normalizovaneho na max == 1 pro analyzu v case
 	tj. okno nema plochu 1 (kterou potrebujeme pro ampl/pwr frekv char)
*/
template <typename T> T f_win(int i, int N, e_win w = WHAMM, const t_tf_props &p = fwin_pro_0){

    T t = (N & 1) ? i : (i+0.5);  //abysme meli okynko symetricky
    switch (w){

       case WHANN:        //w = 1 - cos(2*pi*(0:m-1)'/(n-1)));
            return 2*(0.5 - 0.5*cos( 2*M_PI*t/N ));
       case WHAMM:        //w = (54 - 46*cos(2*pi*(0:m-1)'/(n-1)))/100;
            return 2*(0.54 - 0.46*cos( 2*M_PI*t/N ));
       case WBLCK:        //w = (42 - 50*cos(2*pi*(0:m-1)/(n-1)) + 8*cos(4*pi*(0:m-1)/(n-1)))'/100;
       		/*! \todo parametrizace */
            return 2*(0.42 - 0.50*cos( 2*M_PI*t/N ) - 0.08*cos( 4*M_PI*t/N ));
       case WRECT:
            return 1;
       case WFLAT:        //w = 1 - 1.98*cos(2*Pi*(0:m-1)/(n-1)) + 1.29*cos(4*Pi*(0:m-1)/(n-1)) - 0.388*cos(6*Pi*(0:m-1)/(n-1)) + 0.0322*cos(8*Pi*t/T);
            return (1 - 1.98*cos( 2*M_PI*t/N ) + 1.29*cos( 4*M_PI*t/N ) - 0.388*cos( 6*M_PI*t/N ) + 0.0322*cos( 8*M_PI*t/N ));
       case WBRTL:
            return (2 - fabs(4*(t-N/2)/N));
       case WGAUS:  {

			 //w = exp(-((-Nw/2:(Nw/2-1))/(Sigma*Nw)).^2);
            t_tf_props::const_iterator it;

            int B = 0, fs = 0, TA = 0;
            if(p.end() != (it = p.find(s_wf_B))) B = it->second;
            if(p.end() != (it = p.find(s_wf_fs))) fs = it->second;
            if(p.end() != (it = p.find(s_wf_T_aver))) TA = it->second;

			T sigma = 0.5; //default
			if(fs && B) sigma = (1.0 * B) / fs; //frekvencni design
            else if(fs && TA) sigma = (TA * fs) / 1000000.0; //casovy design

            T temp = (t-N/2.0) / (sigma*N);    //zpocitame si predem exponent
			return exp(-(temp*temp));
        }
        default:
        break;
    }

    return 0;
}

template <typename T> class t_f_win {

private:
	e_win m_w;
	t_tf_props m_p;
	std::vector<T> m_c;

    int m_i;
    int m_N;

public:
	T operator [](int i)
	{
        i %= m_N;  //sanity prevency
        i = abs(m_N/2 - i);  //symetry

        while(m_i <= i){  //dopocotam vse mezi poslednim a pozadovanym
            m_c[m_i] = f_win<T>(m_i, m_N, m_w, m_p); //vypocitavame + pres ref se zapise do cache
            m_c[m_N - m_i - 1] = m_c[m_i]; //vyuzit symetrie okenek a nacachovat rovnou i zrcadlovou hodnotu
            m_i += 1;
        }

        return m_c[i];
	}

	int &operator [](const char *param)
	{
        if(0 == strcmp("#N", param)) return m_N;  //i pro zmenu delky ale bez resetu
		return m_p[param]; //nebo jiny parametr
	}

    void log(){

        f_arr2log(m_c);
    }

	void reset(int N){  //nastaveni nove delky 

		m_c.resize(N);
		m_i = 0;
		m_N = N;
	}

    t_f_win(e_win w, int N = 256, const t_tf_props &p = fwin_pro_0) :
		m_w(w),
		m_N(N),
		m_p(p),
		m_c(N)
	{
		m_i = 0;
	}

    t_f_win(e_win w, int N = 256, const char *config = "#B=500#fs=1000") :
		m_c(N)
	{
		m_w = w;
		m_N = N;
		m_i = 0;		
		m_p = f_str2tf(config);
	}
};

#endif // F_WINDOWINGH
