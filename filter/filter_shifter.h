#ifndef FILTER_SHIFTER_H
#define FILTER_SHIFTER_H

#include "filter_fir.h"

template <class T> class t_filter_shifter : public a_filter<T> {

private:
    T                fr_shift;  //aktulani posun
    uint64_t         m_counter;  //cislo vzorku = cas

    a_filter<T> *m_band;
    a_filter<T> *m_lowp;

    bool m_mine;  //true if filters are dynamicaly created
public:
    /*! \brief - indeirect way mofify filter properties via property
     */
    virtual void tune(const t_tf_props &p)
    {
        t_tf_props wp = p;

        if(p.find(s_wf_fs) == p.end())
            return;

        if(p.find(s_wf_f_ce) == p.end())
            return;

        if(p.find(s_wf_B) == p.end())
            return;

        //posun band pass
        m_band->tune(wp);

        //a preventivne zmena B u low pass
        auto ifce = p.find(s_wf_f_ce);
        wp.erase(ifce, ifce); //lp nechceme posouvat
        wp[s_wf_B] *= 2;  //a B muzeme zdvojnasobit protoze dc poustime
        wp[s_wf_G] = 2;  //aby byla zachovana amplituda (rozdilova cast po posunu ma A/2, souctova take)
        m_lowp->tune(wp);

        //nastavime o kolik posouvat pasma
        fr_shift = (1.0 * (wp[s_wf_f_ce] - wp[s_wf_B])) / wp[s_wf_fs];
    }

    /*! \brief bandpass -> shift -> lowpass
    */
    virtual const T *proc(const T &feed, unsigned *count = NULL)
    {
       count = count;
       const T *a = m_band->proc(feed);
       T v = a[0] * sin(2*M_PI*fr_shift * m_counter++);
       return m_lowp->proc(v);
    }


    /*!
     */
    t_filter_shifter(std::vector<T> &bp_fir_coe, std::vector<T> &lp_fir_coe, T _fr_shift, int32_t _decimationf = 1):
        a_filter<T>(NULL, NULL, 0), //empty a_filter - only as interface
        m_band(new t_filter_fir<T>(bp_fir_coe.data(), bp_fir_coe.size(), 1)),
        m_lowp(new t_filter_fir<T>(lp_fir_coe.data(), lp_fir_coe.size(), _decimationf)),
        fr_shift(_fr_shift)
    {
        m_mine = true;
        m_counter = 0;
    }

    /*!
     */
    t_filter_shifter(a_filter<T> *bp_fir, a_filter<T> *lp_fir, T _fr_shift):
        a_filter<T>(NULL, NULL, 0), //empty a_filter - only as interface
        m_band(bp_fir),
        m_lowp(lp_fir),
        fr_shift(_fr_shift)
    {
        m_mine = false;
        m_counter = 0;
    }

    t_filter_shifter(int32_t N, const char *config = "#B=500#fs=1000", int32_t _decimationf = 1):
        a_filter<T>(NULL, NULL, 0), //empty a_filter - only as interface
        m_band(new t_filter_fir<T>(NULL, N, 1)),
        m_lowp(new t_filter_fir<T>(NULL, N, _decimationf))
    {
        tune(config);
        m_mine = true;
        m_counter = 0;
    }

    t_filter_shifter(int32_t N, const t_tf_props &p, int32_t _decimationf = 1):
        a_filter<T>(NULL, NULL, 0), //empty a_filter - only as interface
        m_band(new t_filter_fir<T>(NULL, N, 1)),
        m_lowp(new t_filter_fir<T>(NULL, N, _decimationf))
    {
//        m_band = dynamic_cast<a_filter<T> * >( new t_filter_fir<T>(NULL, N, 1));
//        m_lowp = dynamic_cast<a_filter<T> * >( new t_filter_fir<T>(NULL, N, _decimationf));

        tune(p);
        m_mine = true;
        m_counter = 0;
    }

    virtual ~t_filter_shifter()
    {
        if(m_mine)
        {
            delete m_band;
            delete m_lowp;
        }
    }
};

#endif // FILTER_SHIFTER_H
