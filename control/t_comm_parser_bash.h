#ifndef T_COMM_PARSER_BASH_H
#define T_COMM_PARSER_BASH_H

#include <stdio.h>
#include <stdint.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QWaitCondition>

#include "i_comm_parser.h"
#include "i_comm_io_generic.h"

class t_comm_parser_string : public i_comm_parser {

    using i_comm_parser::tmp;
    using i_comm_parser::last;

private:
    std::vector<const char *> orders;
    std::vector<const char *> destination;

    std::string path;

public:
    virtual e_comm_parser_res feed(uint8_t p){

        if((p == '\r') || (p == '\n')){

            if(tmp[0] == ':')
            {   //zmena adresy; pouzijeme jako novou cestu
                path = tmp;
                return ECOMM_PARSER_WAITING_SYNC;
            }
            else
            {   //prepend path to last
                tmp.insert(0, path);
            }

            //musi to zacinat : jinak synytax error
            //pokud neni za : cislo tak projit destination
            for(unsigned i=0; i < destination.size(); i++)
            {
            }

            //pokud nemam destination tak syntax
            //posunem se za dalsi : a porovnavame s orders
            for(unsigned i=0; i < orders.size(); i++)
            {
                if(s.compare(orders[i]))
                {
                    last = tmp;
                    tmp.clear();
                    return (e_comm_parser_res)i;
                }
            }

            tmp.clear();
            return ECOMM_PARSER_MISMATCH;
        }

        tmp.push_back(p);
        return ECOMM_PARSER_WAITING_ENDOFORD;
    }

    //vraci kod pro registrovany povel
    unsigned reg_command(const char *ord){

        orders.push_back(ord);
        return orders.size();
    }

    //vraci kod pro registrovany povel
    unsigned reg_destination(const char *dest){

        destination.push_back(ord);
        return destination.size();
    }

    t_comm_parser_string(const char *_orders[]) :
        i_comm_parser()
    {
        if(_orders)
            for(int i=0; (_orders[i]) && (*_orders[i]); i++)
                reg_command(_orders[i]);
    }

    virtual ~t_comm_parser_string(){

    }
};


#endif // T_COMM_PARSER_BASH_H
