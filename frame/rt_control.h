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
#include "frame/rt_processor.h"

struct t_frame_cnode
{
    std::string  fname;
    a_rt_base   *node;
};

struct t_frame_cmd_decomp
{
    int        no;
    int        ord;
    QString    par;
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

    std::string                 cpath;  //aktualni node:povel:parametr...
    std::vector<const char *>   corder; //seznam podporovanych povelu
    std::vector<t_frame_cnode>  cnode;  //seznam znamych nodu
    t_frame_map_it  ctopo;  //topologie nodu
    i_comm_generic  &ccomm;  //komunikator

    int m_delay;
    QStringList m_script;

    bool _validity(const std::string &line, t_frame_cmd_decomp *cmd){

        /*! \todo */
        //rozparserovat podle dvoutecek
        //adresa musi byt ze znameho jmena nebo do registrovaneho cisla
        //povel musi byt znam pokud je vyplnen
        //dalsi uz nezkoumame
    }

    int _identify(const QString &ref)
    {
        bool ok;
        int to = ref.toInt(&ok);

        if(!ok)  //node neni definovany cislem
        {   //zkusime to jeste pres fname
            to = -1; //jako nepplatny
            auto it = std::find_if(cnode.begin(),
                                   cnode.end(),
                                   [&](auto const &v) -> { return cmd.par.toStdString() == v.fname; } );

            if(it != cnode.end()) to = std::distance(cnode.begin(), it);
        }

        return (to < corder.size()) ? to : -1;
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
        QString property_value = cmd.par.mid(del + 1);

        QMetaObject::invokeMethod(cnode[cmd.no].node,
                                  SLOT(on_change(const QString &, QVariant &)),
                                  Qt::AutoConnection,
                                  property_name,
                                  property_value);

        _reply_ok();
        return true;
    }

    bool _create(const t_frame_cmd_decomp &cmd)
    {
        QString path;

        /*! \todo zadani custom jmena */

        if(cmd.par.compare("?") == 0)
        {
            QString help;
            help += "gen{props} - generator with json mandatory fs, f_0, f_1, T\r\n";
            help += "mic(soundcard) - microphone input\r\n";
            help += "playback(soundcard) - speaker outpu\r\n";
            help += "wav(filename) - record as signal source\r\n";
            help += "rec(filename) - record as signal sing\r\n";
            help += "dsp(type) - audio processor\r\n";
            help += "filter(props) - audio filter\r\n";
            ccomm.query(help, 0);
            return true;
        }

        t_frame_cnode v = {"", NULL};
        if(cmd.par.startsWith("gen"))
        {
            v.fname = QString("gen%1").arg(cnode.size());
            v.node =  new t_rt_sweep_generator("\"fs\":{\"__def\":8000},"
                                               "\"f_0\":{\"__def\":500},"
                                               "\"f_1\":{\"__def\":2500},"
                                               "\"T\":{\"__def\":5000}");
        }
        else if(cmd.par.startsWith("mic"))
        {
            v.fname = QString("mic").arg(cnode.size());
            v.node =  new t_rt_audioinput("");
        }
        else if(cmd.par.startsWith("playback"))
        {
            v.fname = QString("playback").arg(cnode.size());
            v.node =  new t_rt_audiooutput("");
        }
        else if(cmd.par.startsWith("filter"))
        {
            v.fname = QString("filter").arg(cnode.size());
            v.node =  new t_rt_filter("\"params\":{\"__def\":\"#B=500#fs=1000#FILTER=FIRDIR1#WINDOW=WHANN\"}");
        }
        else if(cmd.par.startsWith("wav"))
        {
            v.fname = QString("wav").arg(cnode.size());
            path = (!cmd.par.isEmpty()) ? QString("{\"path\":{\"__def\":\"%1\"}}").arg(cmd.par) : "";
            v.node =  new t_rt_audiooutput(path);
        }
        else if(cmd.par.startsWith("rec"))
        {
            v.fname = QString("rec").arg(cnode.size());
            path = (!cmd.par.isEmpty()) ? QString("{\"path\":{\"__def\":\"%1\"}}").arg(cmd.par) : "";
            v.node =  new t_rt_recorder(path);
        }
        cnode.push_back(v);
    }

    void _connect(const t_frame_cmd_decomp &cmd)
    {
        if(cmd.par[0] == '?')
        {
            _printout_topo(cmd.no);
            _reply_ok();
            return true;
        }
        else
        {
            int to = _identify(cmd.par);
            if((to >= 0) && (to < cnode.size()))
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
    }

    void _printout_topo(int src)
    {
        for(const t_frame_cnode &i : cnode)
        {
            ccomm.query(i.fname + "\r\n", 0);
        }
    }

    void _help()
    {
        QString help;
        help += "help\r\n\tdisplay this\r\n";
        help += "cfg:A=B\r\n\tconfigure one parameter of node\r\n";
        help += "start:duration\r\n\tkeep running node for duration[ms] or start it forewer";
        help += "stop:duration\r\n\tpause node for duration[ms] or stop forewer";
        help += "create:?|mic|filter|recorder|playback...\r\n\tcreate unconnected specific node, query for extended help\r\n";
        help += "connect:?|name|no\r\n\tquery topology behind node or connet another node with given name or no\r\n";
        help += "wait:duration\r\n\tstop the script evauation\r\n";
        ccomm.query(help, 0);
    }

    void _reply_ok(const char *detail = NULL)
    {
        std::string reply = cpath + ">OK";
        if(detail)
        {
            reply += "(" + *detail + ")";
        }
        reply += "\r\n";
        ccomm.query(reply, 0);  //guery bez cekani na odpoved je reply
    }

    void _reply_error(const char *detail = NULL)
    {
        std::string reply = cpath + ">ERROR";
        if(detail)
        {
            reply += "(" + *detail + ")";
        }
        reply += "\r\n";
        ccomm.query(reply, 0);  //guery bez cekani na odpoved je reply
    }


private slots:

    //spracovani jednoho radku skriptu
    void on_procline()
    {
        on_newline(-1, m_script.first());
        if(m_script.size() > 0){

            m_script.removeFirst();
            QTimer::singleShot(m_delay, this, SLOT(on_procline));
        }
    }

    //povely od jinud nez od io
    void do_script(const QString &script)
    {
        m_script = script.split('\n');
        on_procline();
    }

    //sem pristane signal od io
    void on_newline(int ord, const QByteArray &line){

        //cmd-control process
        std::string loc = line.toStdString();
        t_frame_cmd_decomp cmd;
        int dl;

        while(!loc.empty()){  //rozpadnout na '&' sekce

            std::string chunk;
            int pos = loc.find('&');
            if(pos != std::string::npos) chunk = loc.substr(pos+1);

            if(loc.front() != ':')
            {  //nejde o absolutni cestu, pridavame directory
               loc.insert(0, cpath);
            }
            else
            {   //test loc
                  if(!_validity(loc, &cmd))
                {
                    if(!de.node) _reply_error("adress");
                    else if(de.ord < 0) _reply_error("order");
                    else _reply_error("syntax");
                    return;
                }
                //update path
                if(loc.back() == ':')
                {   //aktualizace aktivni cesty
                    cpath = loc;
                    _reply_ok();
                    return;
                }
            }

            //eval command
            if(0 == strcmp(corder[cmd.ord], "start"))
            {
                //cmd.node->on_start(); - volame na primo
                //volame pres "signal" - lepsi ze bude bezpecne i pro vicevlakonove systemy
                QMetaObject::invokeMethod(cnode[cmd.no].node, SLOT(on_start(int)), Qt::AutoConnection);
                //pokud ma stav omezene trvani zapnem casovac a toglujeme
                if((dl = cmd.par.toInt())) QTimer::singleShot(dl, cnode[cmd.no].node, SLOT(on_stop(int)));
            }
            else if(0 == strcmp(corder[cmd.ord], "stop"))
            {
                QMetaObject::invokeMethod(cnode[cmd.no].node, SLOT(on_stop(int)), Qt::AutoConnection);
                //pokud ma stav omezene trvani zapnem casovac a toglujeme
                if((dl = cmd.par.toInt())) QTimer::singleShot(dl, cnode[cmd.no].node, SLOT(on_start(int)));
            }
            else if(0 == strcmp(corder[cmd.ord], "wait"))
            {
                //pauza ve vykonu scriptu
                if((m_delay = cmd.par.toInt())){;}
            }
            else if(0 == strcmp(corder[cmd.ord], "create"))
            {
                _create(cmd);
            }
            else if(0 == strcmp(corder[cmd.ord], "connect"))
            {
                _connect(cmd);
            }
            else if(0 == strcmp(corder[cmd.ord], "cfg"))
            {
                _config(cmd);
            }
            else if(0 == strcmp(corder[cmd.ord], "help"))
            {
                _help();
            }

            loc = chunk; //pokracujeme jen pokud bude chunk nenulovy
        }
    }

public:
    t_rt_control(i_comm_parser &parser, QObject *parent = NULL):
        QObject(parent),
        corder({"start", "stop", "wait", "create", "cfg", "connect"}),
        ccomm(parser)
    {
        m_delay = 0;
        QObject::connect(&ccomm, SIGNAL(order(int,const QByteArray &),
                         this, SLOT(on_newline(int,const QByteArray &));
    }
};

#endif // RT_CONTROL_H
