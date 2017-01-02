#ifndef F_PROPS_H
#define F_PROPS_H

#include "easylogging++.h"

/*! time-freq properties
    well known parameters
*/
extern const char *s_wf_fs;  //[Hz] sampling frequency
extern const char *s_wf_B;  //[Hz] bandwith na -3dB (tam kde apml. spadne na sqrt(2))
extern const char *s_wf_G;  //[dB] gain
extern const char *s_wf_Beff;  //efektivni bandwith fm/fs
extern const char *s_wf_D;  //[dB/dec] tlumeni na dekade
extern const char *s_wf_D_oct;  //[dB/oct] tlumeni na oktave
extern const char *s_wf_GD; //group delay - todo: tady je problem se definici protoze se meni s frekvenci, muze byt promerne, nebo efektivni
extern const char *s_wf_F_shift; //posun idealni LP (zakladni) charaketeristiky
extern const char *s_wf_F_type; //zakladni charakteristika filtru - lp, hp, bp, notch, all, diff...
extern const char *s_wf_F_central; //centralni frekvence - nahrazuje hodne typ vyse (0 - lp, 1 - hp, <0,1> - bp)
extern const char *s_wf_T_aver; //[us], averaging time = casova konstanta filtru

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
