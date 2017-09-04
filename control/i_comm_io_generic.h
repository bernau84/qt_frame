#ifndef I_COMM_GENERIC
#define I_COMM_GENERIC

#include <stdio.h>
#include <stdint.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>

#include "i_comm_parser.h"

#define VI_COMM_REFRESH_RT  20  //definuje reakcni cas v ms

class i_comm_generic : public QObject {

    Q_OBJECT

protected:
    i_comm_parser *parser;
    bool m_local_echo;

private slots:
    void timerEvent(QTimerEvent *event){

        Q_UNUSED(event);
        refresh();
    }

signals:
    void order(int ord, const QByteArray &par);

public:

    void local_echo(bool on)
    {
        m_local_echo = on;
    }

    virtual void on_read(QByteArray &dt) = 0;
    virtual void on_write(const QByteArray &dt) = 0;

    virtual void callback(int ord, const QByteArray &par){

        Q_UNUSED(ord); Q_UNUSED(par);
        //qDebug() << "ord" << ord << "par" << par;
    }

    e_comm_parser_res refresh(){

        QByteArray dt;
        on_read(dt);
        if(dt.isEmpty())
            return ECOMM_PARSER_WAITING_SYNC;

        if(m_local_echo) on_write(dt);

        int ret = 0;
        for(int i=0; i<dt.length(); i++)
            switch((ret = parser->feed(dt[i]))){

                case ECOMM_PARSER_ERROR:
                    return (e_comm_parser_res)ret;
                case ECOMM_PARSER_WAITING_SYNC:
                case ECOMM_PARSER_WAITING_ENDOFORD:   
                break;
                case ECOMM_PARSER_MISMATCH:
                    //ale pokracujeme - vyhodime signal
                    //duvodem muze byt treba jen to ze nemame v parseru zaregistrovany zadny order
                    //ale radek sme prijali
                default:
                case ECOMM_PARSER_MATCH_ORDNO_0:                    
                    std::vector<uint8_t> st_ord = parser->getlast();
                    QByteArray qt_ord((const char *)st_ord.data(), (int)st_ord.size());
                    callback(ret, qt_ord);
                    emit order(ret, qt_ord);
                break;
            }

        return (e_comm_parser_res)ret;
    }

    void query(const QByteArray &cmd, int timeout){

        on_write(cmd);

        while((timeout > 0) && (refresh() <= ECOMM_PARSER_WAITING_ENDOFORD)){
            //blokujici
            QMutex localMutex;
            localMutex.lock();
            QWaitCondition sleepSimulator;
            sleepSimulator.wait(&localMutex, VI_COMM_REFRESH_RT);
            localMutex.unlock();

            timeout -= VI_COMM_REFRESH_RT;
        }
    }

    i_comm_generic(i_comm_parser *_parser, QObject *parent = NULL) :
        QObject(parent),
        parser(_parser)
    {
        if(VI_COMM_REFRESH_RT)
            this->startTimer(VI_COMM_REFRESH_RT);
    }

    virtual ~i_comm_generic(){

    }
};


#endif // I_COMM_GENERIC

