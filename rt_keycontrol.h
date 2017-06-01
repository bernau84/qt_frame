#ifndef RT_KEYCONTROL_H
#define RT_KEYCONTROL_H

#include <QObject>

#define RT_ORD_TABLE\
    RT_IT(RT_ORD_START, "START"),\
    RT_IT(RT_ORD_STOP, "STOP"),\
    RT_IT(RT_ORD_WAIT, "WAIT"),\
    RT_IT(RT_ORD_SET, "SET"),\
    RT_IT(RT_ORD_GET, "GET"),\


class rt_central_control : public QObject
{

};

#endif // RT_KEYCONTROL_H
