#ifndef RT_CONTROL_H
#define RT_CONTROL_H

#include <QString>
#include <QMap>
#include <QTimer>

#include "rt_base_a.h"
#include "control/i_comm_io_generic.h"

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

    std::string           cpath;  //aktualni node:povel:parametr...
    std::vector<const char *>  corder; //seznam podporovanych povelu
    std::vector<std::string>  cnode;  //seznam znamych nodu
    t_frame_map_it        ctopo;  //topologie nodu
    i_comm_generic  ccomm;  //komunikator
    bool            cpostpone;  //indikace pauzy ve vykonu skriptu

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
        int to = cmd.par.toInt(&ok);

        if(!ok)  //node neni definovany cislem
        {   //zkusime to jeste pres fname
            to = -1; //jako nepplatny
            auto it = std::find(cnode.begin(), cnode.end(), cmd.par.toStdString());
            if(it != cnode.end()) to = std::distance(cnode.begin(), it);
        }

        return to;
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

        QMetaObject::invokeMethod(cnode[cmd.no],
                                  SLOT(on_change(const QString &, QVariant &)),
                                  Qt::AutoConnection,
                                  property_name,
                                  property_value);

        _reply_ok();
        return true;
    }

    bool _create(const t_frame_cmd_decomp &cmd)
    {
        /*! \todo name, idn */
        if(cmd.par.compare("?") == 0)
        {
            QString help;
            help += "gen(type) - generator\r\n";
            help += "mic(card) - microphone input\r\n";
            help += "playback(card) - speaker outpu\r\n";
            help += "wav(filename) - record as signal source\r\n";
            help += "rec(filename) - record as signal sing\r\n";
            help += "dsp(type) - audio processor\r\n";
            help += "filter(props) - audio filter\r\n";
            ccomm.query(help, 0);
            return true;
        }

        if()
        {

        }
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
                cnode[cmd.no]->connect(cnode[to]); //connect(zdroj)
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
    /*! \todo */
    }

    void _help()
    {
    /*! \todo */
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
            if(pos != npos) chunk = loc.substr(pos+1);

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
                QMetaObject::invokeMethod(cnode[cmd.no], SLOT(on_start(int)), Qt::AutoConnection);
                //pokud ma stav omezene trvani zapnem casovac a toglujeme
                if((dl = cmd.par.toInt())) QTimer::singleShot(dl, cnode[cmd.no], SLOT(on_stop(int)));
            }
            else if(0 == strcmp(corder[cmd.ord], "stop"))
            {
                QMetaObject::invokeMethod(cnode[cmd.no], SLOT(on_stop(int)), Qt::AutoConnection);
                //pokud ma stav omezene trvani zapnem casovac a toglujeme
                if((dl = cmd.par.toInt())) QTimer::singleShot(dl, cnode[cmd.no], SLOT(on_start(int)));
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
                _help(cmd);
            }

            loc = chunk; //pokracujeme jen pokud bude chunk nenulovy
        }
    }

public:
    t_rt_control(i_comm_parser &parser, QString *initscript, QObject *parent = NULL):
        corder({"start", "stop", "wait", "create", "cfg", "connect"}),
        ccomm(parser, parent)
    {
        m_delay = 0;
        QObject::connect(&ccomm, SIGNAL(order(int,const QByteArray &),
                         this, SLOT(on_newline(int,const QByteArray &));

    }
};

#endif // RT_CONTROL_H
