
#include "easylogging++.h"

/*! time-freq properties 
	well known parameters 
*/
const char *s_wf_fs = "#fs";  //[Hz] sampling frequency
const char *s_wf_B = "#B";  //[Hz] bandwith na -3dB (tam kde apml. spadne na sqrt(2))
const char *s_wf_G = "#G";  //[dB] gain
const char *s_wf_Beff = "#Beff";  //efektivni bandwith fm/fs 
const char *s_wf_D = "#D";  //[dB/dec] tlumeni na dekade
const char *s_wf_D_oct = "#Doct";  //[dB/oct] tlumeni na oktave
const char *s_wf_GD = "#GD"; //group delay - todo: tady je problem se definici protoze se meni s frekvenci, muze byt promerne, nebo efektivni
const char *s_wf_F_shift = "#F_sh"; //posun idealni LP (zakladni) charaketeristiky
const char *s_wf_F_type = "#F_ty"; //zakladni charakteristika filtru - lp, hp, bp, notch, all, diff...
const char *s_wf_F_central = "#F_fc"; //centralni frekvence - nahrazuje hodne typ vyse (0 - lp, 1 - hp, <0,1> - bp)
const char *s_wf_T_aver = "#T_av"; //[us], averaging time = casova konstanta filtru

typedef std::map<const char *, int> t_tf_props;

/*! konverze parametru ze stringu na map
 */
t_tf_props f_str2tf(const char *s){

	t_tf_props ret;
	const char *list[] = {
		s_wf_fs,
		s_wf_B,
		s_wf_G,
		s_wf_Beff,
		s_wf_D,
		s_wf_D_oct,
		s_wf_GD,
		s_wf_F_shift,
		s_wf_F_type,
		s_wf_F_central,
		s_wf_T_aver,
		NULL
    };

	int t, n;

	while(s){

        const char **o = list;
        for(; *o; o++){

            n = strlen(*o);
            if(0 == memcmp(s, *o, n))
				if(1 == sscanf(s+n+1, "%d", &t)){ //posun za '='

                    ret[*o] = t;
					break;
				}
		}

        s = strchr(s, '#');
	}
		

	return ret;
}


/*! logovani
 */
template <typename T> void f_arr2log(std::vector<T> &arr){
    LOG(DEBUG) << arr;
}

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

    switch (w){

       case WHANN:        //w = 1 - cos(2*pi*(0:m-1)'/(n-1)));
            return 2*(0.5 - 0.5*cos( 2*M_PI*i/N ));
       case WHAMM:        //w = (54 - 46*cos(2*pi*(0:m-1)'/(n-1)))/100;
            return 2*(0.54 - 0.46*cos( 2*M_PI*i/N ));
       case WBLCK:        //w = (42 - 50*cos(2*pi*(0:m-1)/(n-1)) + 8*cos(4*pi*(0:m-1)/(n-1)))'/100;
       		/*! \todo parametrizace */
            return 2*(0.42 - 0.50*cos( 2*M_PI*i/N ) - 0.08*cos( 4*M_PI*i/N ));
       case WRECT:
            return 1;
       case WFLAT:        //w = 1 - 1.98*cos(2*Pi*(0:m-1)/(n-1)) + 1.29*cos(4*Pi*(0:m-1)/(n-1)) - 0.388*cos(6*Pi*(0:m-1)/(n-1)) + 0.0322*cos(8*Pi*t/T);
            return (1 - 1.98*cos( 2*M_PI*i/N ) + 1.29*cos( 4*M_PI*i/N ) - 0.388*cos( 6*M_PI*i/N ) + 0.0322*cos( 8*M_PI*i/N ));
       case WBRTL:
            return (2 - fabs(4*(i-N/2)/N));
       case WGAUS:  {

			 //w = exp(-((-Nw/2:(Nw/2-1))/(Sigma*Nw)).^2);
            int B = p.find(s_wf_B)->second;
            int fs = p.find(s_wf_fs)->second;
            int TA = p.find(s_wf_T_aver)->second;

			T sigma = 0.5; //default
			if(fs && B) sigma = (1.0 * B) / fs; //frekvencni design
			if(fs && TA) sigma = (TA * fs) / 1000000; //casovy design

            T temp = (i-N/2.0) / (sigma*N);    //zpocitame si predem exponent
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
	int m_N;
	t_tf_props m_p;
	std::vector<T> m_c;
	int m_i;

public:
	T operator [](int i)
	{
		T &ret = m_c[i % m_c.size()]; //berem z cache
        if(i > m_i) ret = f_win<T>(i, m_N, m_w, m_p); //vypocitavame + pres ref se zapise do cache
		return ret;
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

    t_f_win(e_win w, int N = 256, const char *config = "#B=0.5#fs=1") :
		m_c(N)
	{
		m_w = w;
		m_N = N;
		m_i = 0;		
		m_p = f_str2tf(config);
	}
};
