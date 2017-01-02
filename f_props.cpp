
#include "f_props.h"

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
const char *s_wf_T_aver = "#TA"; //[us], averaging time = casova konstanta filtru

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

        s = strchr(s+1, '#');
    }


    return ret;
}
