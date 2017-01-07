#ifndef F_PROPS_H
#define F_PROPS_H

#include "easylogging++.h"

/*
const char *s_wf_fs = "#fs";  //[Hz] sampling frequency
const char *s_wf_B = "#B";  //[Hz] bandwith na -3dB (tam kde apml. spadne na sqrt(2))
const char *s_wf_G = "#G";  //[dB] gain
const char *s_wf_Beff = "#Beff";  //efektivni bandwith fm/fs
const char *s_wf_D = "#D";  //[dB/dec] tlumeni na dekade
const char *s_wf_D_oct = "#Doct";  //[dB/oct] tlumeni na oktave
const char *s_wf_GD = "#GD"; //group delay - todo: tady je problem se definici protoze se meni s frekvenci, muze byt promerne, nebo efektivni
const char *s_wf_F_sh = "#f_sh"; //posun idealni LP (zakladni) charaketeristiky
const char *s_wf_F_type = "#f_ty"; //zakladni charakteristika filtru - lp, hp, bp, notch, all, diff...
const char *s_wf_F_central = "#f_ce"; //centralni frekvence - nahrazuje hodne typ vyse (0 - lp, 1 - hp, <0,1> - bp)
const char *s_wf_T_aver = "#TA"; //[us], averaging time = casova konstanta filtru
*/

#define F_PROPS_ALL\
    F_PROPS_IT(fs,[Hz],sampling frequency)\
    F_PROPS_IT(B,[Hz],bandwith na -3dB (tam kde apml. spadne na sqrt(2)))\
    F_PROPS_IT(G,[dB],gain)\
    F_PROPS_IT(Beff,[Hz],efektivni bandwith)\
    F_PROPS_IT(D,[dB/dec],tlumeni na dekade)\
    F_PROPS_IT(Doct,[dB/oct],tlumeni na oktave)\
    F_PROPS_IT(GD,[us],group delay)\
    F_PROPS_IT(f_sh,[Hz],posun idealni LP (zakladni) charaketeristiky)\
    F_PROPS_IT(f_ce,[Hz],centralni frekvence)\
    F_PROPS_IT(f_ty,[Hz],zakladni charakteristika filtru - lp/hp/bp/notch/all/diff...)\
    F_PROPS_IT(TA,[us],casova konstanta filtru(averaging time))

#define _F_PROPS_IT_STR(T) #T
#define F_PROPS_IT_STR(T) _F_PROPS_IT_STR(T)

/*! time-freq properties
    well known parameters
*/
#define F_PROPS_IT(name,unit,info)\
    extern const char *s_wf_##name;

    F_PROPS_ALL
#undef F_PROPS_IT

/*! string->value map definition
 */
typedef std::map<const char *, int> t_tf_props;


/*! konverze parametru ze stringu na map
 */
extern t_tf_props f_str2tf(const char *s);


/*! logovani
 */
template <typename T> void f_arr2log(std::vector<T> &arr){
    LOG(DEBUG) << arr;
}

#endif // F_PROPS_H
