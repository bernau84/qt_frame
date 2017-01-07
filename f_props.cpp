
#include "f_props.h"

/*! time-freq properties
    well known parameters
*/

#define F_PROPS_IT(name,unit,info)\
    const char *s_wf_##name = "#" F_PROPS_IT_STR(name);

F_PROPS_ALL
#undef F_PROPS_IT

/*! konverze parametru ze stringu na map
 */
t_tf_props f_str2tf(const char *s){

    t_tf_props ret;
    const char *list[] = {
#define F_PROPS_IT(name,unit,info)\
        s_wf_##name,
F_PROPS_ALL
#undef F_PROPS_IT
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
