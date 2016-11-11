#ifndef RT_BASE_H
#define RT_BASE_H

#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QSharedPointer>

#include "rt_setup.h"
#include "rt_exchange_i.h"

#include "easylogging++.h"

static int rt_base_counter = 0;

class a_rt_base;

class a_rt_base : public QObject
{
    Q_OBJECT

private:
    /*! constructor helper */
    QJsonObject _path2json(const QString &path){

        // default config
        QFile f_def(path);  //from resources
        if(f_def.open(QIODevice::ReadOnly | QIODevice::Text)){

            QByteArray f_data = f_def.read(64000);

            QJsonDocument js_doc = QJsonDocument::fromJson(f_data);
            if(!js_doc.isEmpty()){

                //LOG(INFO) << js_doc.toJson();
                return js_doc.object();
            }
        }

        return QJsonObject();
    }

//virtuals:
    /*! init privates from configuration */
    virtual int reload(int p) = 0;

    /*! here do the work */
    virtual int proc(const i_rt_exchange *p) = 0;

protected:
   t_rt_setup par;
   QString fancy_name;

signals:
    void update(QSharedPointer<i_rt_exchange> d);
    void change(const QString &name, QVariant &v);

public slots:
    /*!  */
    virtual void on_start(int p) = 0;

    /*! */
    virtual void on_stop(int p) = 0;

   /*! data processor */
   void on_update(QSharedPointer<i_rt_exchange> d){

       TIMED_FUNC(t1);
       if(d.isNull()) proc(NULL);
        else proc(d.data());
   }

   /*! reconfiguration */
   void on_change(const QString &name, QVariant &v){

       LOG(TRACE);
       config(name, v);  //cant return value trough signal
   }

public:
    /*! */
    void connect(const a_rt_base *follower){

        if(!QObject::connect(this, SIGNAL(update(QSharedPointer<i_rt_exchange>)),
                         follower, SLOT(on_update(QSharedPointer<i_rt_exchange>)))){

            LOG(ERROR) << "update signal not connected!";
        }
    }

    /*! read / update config parameter runtime
     * new value is not promoted to json file
     * if value is not valid than is used for read only, otherwise for write
     * */
    void config(const QString &name, QVariant &v){

        t_setup_entry e;
        if(!par.ask(name, &e)){ // + get copy of attr entry

            v.clear();
            return;
        }

        if(v.isValid()){

            QJsonValue jval_set = QJsonValue::fromVariant(v);
            QJsonValue jval_get = e.set(jval_set);
            par.replace(name, e);  //put back!

            reload(0); //apply
            v = jval_get.toVariant();  //writeback result
            return;
        }

        v = par[name].get().toVariant();
        return;
    }

    a_rt_base(const QString &js_config, QObject *parent = NULL):
        QObject(parent),
        par(_path2json(js_config))
    {
        LOG(INFO) << rt_base_counter++;
    }

    virtual ~a_rt_base(){;}
};

#endif // RT_BASE_H
