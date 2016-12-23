
/*! time-freq properties 
	well known parameters 
*/
/*
	for filter definition
*/
//const char *s_wf_N = "N"; //number of coeficient; finite impulse response length 
const char *s_wf_fs = "fs";  //[Hz] sampling frequency
const char *s_wf_B = "B";  //[Hz] bandwith na -3dB (tam kde apml. spadne na sqrt(2))
const char *s_wf_G = "G";  //[dB] gain
const char *s_wf_Beff = "Beff";  //efektivni bandwith fm/fs 
const char *s_wf_D = "D";  //[dB/dec] tlumeni na dekade
const char *s_wf_D_oct = "Doct";  //[dB/oct] tlumeni na oktave
const char *s_wf_GD = "GD"; //group delay - todo: tady je problem se definici protoze se meni s frekvenci, muze byt promerne, nebo efektivni
const char *s_wf_F_shift = "F_sh"; //posun idealni LP (zakladni) charaketeristiky
const char *s_wf_F_type = "F_ty"; //zakladni charakteristika filtru - lp, hp, bp, hp, notch, all, diff...
const char *s_wf_T_aver = "T_av"; //[us], averaging time = casova konstanta filtru

typedef std::map<const char *, int> t_tf_props;

/*
	\enum supporting windows waveforms
*/
enum e_win {

    HANN = 0x10,
    HAMM = 0x11,
    FLAT = 0x12,
    BLCK = 0x14,
    RECT = 0x18,
    BRTL = 0x20,
    GAUS = 0x21
};

static t_tf_props fwin_pro_0;  //empty settings

/*! \brief vraci vzorek okynka normalizovaneho na max == 1 pro analyzu v case
 	tj. okno nema plochu 1 (kterou potrebujeme pro ampl/pwr frekv char)
*/
template <typename T> T fwin(int i, int N, e_win w = HAMM, const t_tf_props &p = fwin_pro_0){

    switch (w){

       case HANN:        //w = 1 - cos(2*pi*(0:m-1)'/(n-1)));
            return 2*(0.5 - 0.5*cos( 2*M_PI*i/N ));
       case HAMM:        //w = (54 - 46*cos(2*pi*(0:m-1)'/(n-1)))/100;
            return 2*(0.54 - 0.46*cos( 2*M_PI*i/N ));
       case BLCK:        //w = (42 - 50*cos(2*pi*(0:m-1)/(n-1)) + 8*cos(4*pi*(0:m-1)/(n-1)))'/100;
       		/*! \todo parametrizace */
            return 2*(0.42 - 0.50*cos( 2*M_PI*i/N ) - 0.08*cos( 4*M_PI*i/N ));
       case RECT:
            return 1;
       case FLAT:        //w = 1 - 1.98*cos(2*Pi*(0:m-1)/(n-1)) + 1.29*cos(4*Pi*(0:m-1)/(n-1)) - 0.388*cos(6*Pi*(0:m-1)/(n-1)) + 0.0322*cos(8*Pi*t/T);
            return (1 - 1.98*cos( 2*M_PI*i/N ) + 1.29*cos( 4*M_PI*i/N ) - 0.388*cos( 6*M_PI*i/N ) + 0.0322*cos( 8*M_PI*i/N ));
       case BRTL:
            return (2 - fabs(4*(i-N/2)/N));
       case GAUS:  {

			 //w = exp(-((-Nw/2:(Nw/2-1))/(Sigma*Nw)).^2);
			int B = p[s_wf_B];
			int fs = p[s_wf_fs];
			int TA = p[s_wf_T_aver];

			T sigma = 0.5; //default
			if(fs && B) sigma = (1.0 * B) / fs; //frekvencni design
			if(fs && TA) sigma = (TA * fs) / 1000000; //casovy design

			temp = (i-N/2.0) / (sigma*N);    //zpocitame si predem exponent
			return exp(-(temp*temp));
        }
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
		if(i > m_i) ret = fwin(i, m_N, m_w, m_p); //vypocitavame + pres ref se zapise do cache
		return ret;
	}

	int &operator [](const char *param)
	{
		if(0 == strcmp("N")) return N;  //i pro zmenu delky ale bez resetu
		return m_p[param]; //nebo jiny parametr
	}

	int reset(int N){  //nastaveni nove delky 

		m_c.resize(N);
		m_i = 0;
		m_N = N;
	}

	t_f_win(e_win w, N = 256, const t_tf_props &p = fwin_pro_0) :
		m_w(w),
		m_N(N),
		m_p(p),
		m_c(N)
	{
		m_i = 0;
	}

	t_f_win(e_win w, N = 256, const char *descr = "B=0.5,fs=1") :
		m_c(N)
	{

		m_w = w;
		m_N = N;
		m_i = 0;		

		int t;
		const char *p;

		if((p = strstr(descr, "B=")))
			if(1 == sscanf(p+2, "%d", &t))
				m_p[s_wf_B] = t;

		if((p = sscanf(descr, "fs=")))
			if(1 == sscanf(p+2, "%d", &t))
				m_p[s_wf_fs] = t;		
	}
}