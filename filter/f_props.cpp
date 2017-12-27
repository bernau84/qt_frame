
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
        NULL
    };

    const char *units[] = {
#define F_PROPS_IT(name,unit,info)\
        #unit,
F_PROPS_ALL
#undef F_PROPS_IT
        NULL
    };

    unsigned t, n;
    const char *en_del = NULL;
    const char *en_p = NULL;

    while(s)
    {
        const char **o = list;
        for(; *o; o++)
        {
            n = strlen(*o);
            if(0 == memcmp(s, *o, n))
            {
                if((en_del = strchr((en_p = units[o - list]), '|')))
                {
                    //jde o vycet - musime najit poradi shody == cislo enumu
                    char en[32] = "";
                    int en_n = (sscanf(s+n+1, "%31[^#]", en) == 1) ? strlen(en) : 0;

                    for(t = 0; en_n && en_p; t++)
                    {
                        if(0 == memcmp(en_p + 1, en, en_n)) //+1 move behind '[' or '|'
                        {
                            ret[*o] = t;
                            break;
                        }

                        en_del = strchr((en_p = en_del) + 1, '|');
                    }

                    break;
                }
                else if(1 == sscanf(s+n+1, "%d", &t)) //posun za '='
                {

                    ret[*o] = t;
                    break;
                }
            }
        }

        s = strchr(s+1, '#');
    }

    return ret;
}
