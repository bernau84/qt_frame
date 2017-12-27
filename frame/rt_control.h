#ifndef RT_CONTROL_H
#define RT_CONTROL_H

#include <QString>
#include <QMap>
#include <QTimer>

#include "rt_base_a.h"
#include "control/i_comm_io_generic.h"
#include "frame/rt_audioinput.h"
#include "frame/rt_audiooutput.h"
#include "frame/rt_wavinput.h"
#include "frame/rt_recorder.h"
#include "frame/rt_generator.h"
#include "frame/rt_filter.h"
#include "frame/rt_processor.h"

//tabulka moznosti povelu create - seznam prvku ktere umime a jak se daji konfigurovat
/*! \todo    IT(dsp,     t_rt_processor,         "", "TODO audio processor")\ */

#define RT_CREATE_MENU()\
    IT(sweep,   t_rt_sweep_generator,   "{\"fs\":{\"__def\":8000},\"f_0\":{\"__def\":500},\"f_1\":{\"__def\":2500},\"T\":{\"__def\":5000}}", "generator with json mandatory fs, f_0, f_1, T")\
    IT(multi,   t_rt_multisin_generator, "{\"f_n\":{\"__def\":[250, 800, 1500]},\"A_n\":{\"__def\":[]}}", "generator with json mandatory fs, [f_x], [A_x]")\
    IT(corr,    t_rt_harm_correlator,   "", "time based correlator")\
    IT(mic,     t_rt_audioinput,        "", "empty for default microphone input or dedicated name/channel")\
    IT(playback, t_rt_audiooutput,      "", "empty for default microphone input or dedicated name/channel")\
    IT(wav,     t_rt_wavinput,          "{\"path\":{\"__def\":\"input.wav\"}}", "record as signal source")\
    IT(rec,     t_rt_recorder,          "{\"path\":{\"__def\":\"output.wav\"}}", "record as signal sing")\
    IT(filter,  t_rt_filter,            "{\"properties\":{\"__def\":\"#B=500#fs=8000#FILTER=FIRDIR1#WINDOW=WHANN#fc=1500\"}}", "audio filter")

struct t_frame_cnode
{
    QString     fname;
    a_rt_base   *node;
};

struct t_frame_cmd_decomp
{
    int     no;
    int     ord;
    QString par;
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

    QString                 cpath;  //aktualni node:povel:parametr...
    QVector<const char *>   corder; //seznam podporovanych povelu
    QVector<t_frame_cnode>  cnode;  //seznam znamych nodu
    t_frame_map_it  ctopo;  //topologie nodu
    i_comm_generic  *ccomm;  //komunikator

    int m_delay;
    QStringList m_script;

    bool _validity(const QString &line, t_frame_cmd_decomp &cmd){

        QStringList atoms = line.split(':');
        if(atoms.size() < 3)
        {
            return false;
        }
        if(atoms[0] != "")
        {
            return false;
        }
        if((cmd.no = _identify_no(atoms[1])) < 0){;} // return false; - nevadi kdyz jde o broadcast
        if((cmd.ord = _identify_cmd(atoms[2])) < 0) return false;
        if(atoms.size() > 3) cmd.par = atoms[3];
        return true;
    }

    int _identify_cmd(const QString &ref)
    {
        auto it = std::find_if(corder.begin(),
                               corder.end(),
                               [ref](const char *s) -> bool {
                                    //std::cout << s;
                                    return ref.startsWith(s);
                               } );

        if(it == corder.end())
            return -1;

        return std::distance(corder.begin(), it);
    }

    int _identify_no(const QString &ref)
    {
        bool ok;
        int to = ref.toInt(&ok);

        if(!ok)  //node neni definovany cislem
        {   //zkusime to jeste pres fname
            to = -1; //jako neplatny
            auto it = std::find_if(cnode.begin(),
                                   cnode.end(),
                                   [&](t_frame_cnode v) -> bool {
                                        return (0 == v.fname.compare(ref));
                                   } );

            if(it != cnode.end())
                to = std::distance(cnode.begin(), it);
        }

        return (to < cnode.size()) ? to : -1;
    }

    bool _config(const t_frame_cmd_decomp &cmd)
    {
        int del = cmd.par.indexOf('=');
        if(del < 1)
        {
            _reply_error("unknown");
            return false;
        }

        QString property_name = cmd.par.left(del - 1);
        QVariant property_value = cmd.par.mid(del + 1);

        if(property_name[0] == '#')
        {   //nenastavujeme konkretrni cfg property ale propery "parameters"
            //tyka se hlaven asi filtru
            //bypass
            property_name = "properties";
            property_value = cmd.par;
        }

        QMetaObject::invokeMethod(cnode[cmd.no].node,
                                  "on_change",
                                  Qt::AutoConnection,
                                  Q_ARG(QString, property_name),
                                  Q_ARG(QVariant &, property_value));

        _reply_ok();
        return true;
    }

    bool _create(const t_frame_cmd_decomp &cmd)
    {
        QString path;

        if(cmd.par.compare("?") == 0)
        {
            QString help;

#define IT(type, ty, def, shelp)\
        help += #type;\
        help += "{props} - ";\
        help += shelp;\
        help += "; default props are ";\
        help += def;\
        help += "\n";\

RT_CREATE_MENU()

#undef IT

            ccomm->query(help.toLocal8Bit(), 0);
            return true;
        }

        t_frame_cnode v = {"", NULL};
        QRegExp f("^([^{]+)({(?:.*)})?$");  //regex pro nalezeni nazvu prvku a json objektu jako parametry
            //zachytava 1 typ, 2 param (nemusi byt)

        if(f.indexIn(cmd.par) < 0)
        {
            _reply_error("invalid"); //konec - nebylo validni
            return false;
        }

        /*! \todo zadani custom jmena - zatim dano poradim */
        v.fname = f.cap(0) + QString("%1").arg(cnode.size());

        if(1){}

#define IT(type, ty, def, shelp)\
        else if(!f.cap(0).compare(#type))\
        {\
            QString param = (f.captureCount() > 1) ? f.cap(1) : def;\
            v.node = new ty(param);\
        }\

RT_CREATE_MENU()

#undef IT

        if(v.node)
        {
            cnode.push_back(v);

            if(cmd.no >= 0) //nejde o zdroj?
            {
                v.node->connect(cnode[cmd.no].node); //connect(zdroj)
                _update_topo(cnode.size() - 1, cmd.no); //zdroj, pijavice - aktualizace topologie
            }

            _reply_ok();
            return true;
        }

        _reply_error("unknown");
        return false;
    }

    bool _connect(const t_frame_cmd_decomp &cmd)
    {  //connect jako takovy jsme zrusili
        if(cmd.par[0] == '?')
        {   //todo  - presunout do extra fce na vypis topologie
            _printout_topo(cmd.no);
            _reply_ok();
            return true;
        }
        else
        {
            int to = _identify_no(cmd.par);
            if((to >= 0) && (to < (int)cnode.size()))
            {
                cnode[cmd.no].node->connect(cnode[to].node); //connect(zdroj)
                _update_topo(to, cmd.no); //zdroj, pijavice - aktualizace topologie
                _reply_ok();
                return true;
            }

            _reply_error("unknown");
            return false;
        }
    }

    void _update_topo(int src, int leach)
    {
       /*! \todo */
        Q_UNUSED(src);
        Q_UNUSED(leach);
    }

    void _printout_topo(int src)
    {
        Q_UNUSED(src);
        for(const t_frame_cnode &i : cnode)
        {
            QString s = i.fname + "\r\n";
            ccomm->query(s.toLocal8Bit(), 0);
        }
    }

    void _help()
    {
        QString help;
        help += "display this help\r\n";
        help += "cfg:A=B\t\tconfigure one parameter of node\r\n";
        help += "start:duration\t\tkeep running node for duration[ms] or start it forewer\r\n";
        help += "stop:duration\t\tpause node for duration[ms] or stop forewer\r\n";
        help += "create:?|mic|filter|recorder|playback...\t\tcreate unconnected specific node, query for extended help\r\n";
        help += "connect:?|name|no\t\tquery topology behind node or connet another node with given name or no\r\n";
        help += "wait:duration\t\tstop the script evauation\r\n";
        ccomm->query(help.toLocal8Bit(), 0);
    }

    void _reply_ok(const char *detail = NULL)
    {
        QString reply = cpath + ">OK";
        if(detail)
        {
            reply += "(";
            reply += *detail;
            reply += ")";
        }
        reply += "\r\n";
        ccomm->query(reply.toLocal8Bit(), 0);  //guery bez cekani na odpoved je reply
    }

    void _reply_error(const char *detail = NULL)
    {
        QString reply = cpath + ">ERROR";
        if(detail)
        {
            reply += "(";
            reply += detail;
            reply += ")";
        }
        reply += "\r\n";
        ccomm->query(reply.toLocal8Bit(), 0);  //guery bez cekani na odpoved je reply
    }


private slots:

    //spracovani jednoho radku skriptu
    void on_procline()
    {
        while(m_script.size() > 0)
        { //a dalsi
            QString s = m_script.first();
            on_newline(-1, s.toLocal8Bit());
            m_script.removeFirst();

            if(m_delay)
            {   //z prikazu vyplyva pauza
                QTimer::singleShot(m_delay, this, SLOT(on_procline()));
                m_delay = 0;
                return;
            }
        }
    }

    //sem pristane signal od io
    void on_newline(int ord, const QByteArray &line){

        ord = ord;

        //cmd-control process
        QString loc = QString::fromLocal8Bit(line.data(), line.size());
        t_frame_cmd_decomp cmd;
        int dl;
        QStringList list = loc.split("&");  //rozpadnout na '&' sekce

        foreach(QString i, list){

            if(i[0] != ':')
            {  //nejde o absolutni cestu, pridavame directory
               i.insert(0, cpath);
            }

            //update path
            if(i.endsWith(':'))
            {   //aktualizace aktivni cesty
                cpath = i;
                _reply_ok();
                return;
            }

            //test loc
            if(!_validity(i, cmd))
            {
                if(cmd.no < -1) _reply_error("adress");  //-1 broadcast
                else if(cmd.ord < 0) _reply_error("order");
                else _reply_error("syntax");
                return;
            }

            //eval command
            if(0 == strcmp(corder[cmd.ord], "start"))
            {
                //cmd.node->on_start(); - volame na primo
                //volame pres "signal" - lepsi ze bude bezpecne i pro vicevlakonove systemy
                QMetaObject::invokeMethod(cnode[cmd.no].node, "on_start", Qt::AutoConnection, Q_ARG(int, 0));
                //pokud ma stav omezene trvani zapnem casovac a toglujeme
                if((dl = cmd.par.toInt()))
                    QTimer::singleShot(dl, cnode[cmd.no].node, SLOT(on_stop()));
            }
            else if(0 == strcmp(corder[cmd.ord], "stop"))
            {
                QMetaObject::invokeMethod(cnode[cmd.no].node, "on_stop", Qt::AutoConnection, Q_ARG(int, 0));
                //pokud ma stav omezene trvani zapnem casovac a toglujeme
                if((dl = cmd.par.toInt()))
                    QTimer::singleShot(dl, cnode[cmd.no].node, SLOT(on_start()));
            }
            else if(0 == strcmp(corder[cmd.ord], "pause"))
            {
                //pauza ve vykonu scriptu
                if((m_delay = cmd.par.toInt())){;}
            }
            else if(0 == strcmp(corder[cmd.ord], "create"))
            {
                _create(cmd);
            }
            else if(0 == strcmp(corder[cmd.ord], "cfg"))
            {
                _config(cmd);
            }
            else if(0 == strcmp(corder[cmd.ord], "help"))
            {
                _help();
            }
        }
    }

public slots:
    //povely od jinud nez od io
    void do_script(const QString &script)
    {
        m_script = script.split('\n');
        on_procline();
    }


public:
    t_rt_control(i_comm_generic *parser, QObject *parent = NULL):
        QObject(parent),
        cpath("::"),
        corder({"start", "stop", "pause", "create", "cfg", "help"}),
        ccomm(parser)
    {
        m_delay = 0;
        QObject::connect(ccomm, SIGNAL(order(int,const QByteArray &)),
                         this, SLOT(on_newline(int,const QByteArray &)));
    }
};

#endif // RT_CONTROL_H
