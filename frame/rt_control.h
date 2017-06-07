#ifndef RT_CONTROL_H
#define RT_CONTROL_H

#include <QString>
#include <QMap>

#include "rt_base_a.h"
#include "control/i_comm_io_generic.h"

struct t_frame_cnode
{
    std::string fname;
    a_rt_base *node;
};

struct t_frame_map_it;
struct t_frame_map_it
{   //int je poradi nodu ve vektoru - see corder
    std::map<int, t_frame_map_it> flw;
};

class t_rt_control : public QObject
{
    Q_OBJECT

private:

    std::string           cpath;  //aktualni node:povel:parametr...
    std::vector<const char *>  corder; //seznam podporovanych povelu
    std::vector<std::string>  cnode;  //seznam znamych nodu
    t_frame_map_it        ctopo;  //topologie nodu
    i_comm_generic  ccomm;  //komunikator

    bool _validity(const std::string &cmd){

        //rozparserovat podle dvoutecek
        //adresa musi byt ze znameho jmena nebo do registrovaneho cisla
        //povel musi byt znam pokud je vyplnen
        //dalsi uz nezkoumame
    }

    bool _create()
    {

    }

    bool _connect()
    {

    }

    void _printout_map()
    {

    }

private slots:
    //sem pristane signal od io
    void on_newline(int ord, const QByteArray &line){

        //cmd-control process
        std::string loc = line.toStdString();

        if(loc.back() == ':')
        {   //aktualizace aktivni cesty
            cpath += loc;
        }
        else if(loc.front() != ':')
        {  //nejde o absolutni cestu, pridavame directory
           loc.insert(0, cpath);
        }
        else
        {   //test loc
            if(_validity(loc))
            {
                cpath = loc;
                std::cout << cpath << ">OK" << std::endl;
            }
            else
            {
                std::cout << cpath << ">ERROR" << std::endl;
            }

        }

    }

public:
    t_rt_control(i_comm_parser &parser, QObject *parent = NULL):
        ccomm(parser, parent)
    {
        QObject::connect(&ccomm, SIGNAL(order(int,const QByteArray &),
                         this, SLOT(on_newline(int,const QByteArray &));
    }
};

#endif // RT_CONTROL_H
