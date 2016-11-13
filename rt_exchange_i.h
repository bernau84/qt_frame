#ifndef RT_EXCHANGE_I_H
#define RT_EXCHANGE_I_H

#include <stdint.h>

class i_rt_exchange {

public:
    /*! length of record */
    virtual int n() = 0;

    /*! access to particular sample properties */
    virtual double f_t(unsigned i) = 0;
    virtual double f_a(unsigned i) = 0;
    virtual double f_f(unsigned i) = 0;

    /*! access raw data */
    virtual const double *f_t() = 0;
    virtual const double *f_a() = 0;
    virtual const double *f_f() = 0;

/*! \todo ideas

     typedef struct {
            double a, t, f;
    } t_f_v;

    typedef struct {
            uint32_t a, t, f;
    } t_u_v;

    virtual const uint32_t *u_t() = 0;
    virtual const uint32_t *u_a() = 0;
    virtual const uint32_t *u_f() = 0;

    virtual uint32_t u_t(unsigned i) = 0;
    virtual uint32_t u_a(unsigned i) = 0;
    virtual uint32_t u_f(unsigned i) = 0;

    // access to triple value
    virtual const t_f_v &operator[] = 0;
    virtual const t_f_v &f_v(unsigned i);
    virtual const t_u_v &u_v(unsigned i);

    // value given as function
    virtual double f_a_t(double t) = 0;
    virtual double f_a_f(double f) = 0;
*/   
    i_rt_exchange(){;}
    virtual ~i_rt_exchange(){;}
};

#endif // RT_EXCHANGE_I_H
