#ifndef RT_BASE_H
#define RT_BASE_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QSharedPointer>
#include <QKeyEvent>

#include "rt_setup.h"
#include "rt_exchange_i.h"

#include "easylogging++.h"

static int rt_base_counter = 0;

class a_rt_base;

class a_rt_base : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString _name READ name WRITE rename) //QString fancy_name;
    Q_PROPERTY(int _no READ no)

protected:
   t_rt_setup par;

private:

    QJsonObject _str2json(const QByteArray &array){

        QJsonDocument js_doc = QJsonDocument::fromJson(array);
        if(!js_doc.isEmpty()){

            //LOG(INFO) << js_doc.toJson();
            return js_doc.object();
        }

        return QJsonObject();
    }

    /*! constructor helper */
    QJsonObject _path2json(const QString &conf){

        //filepath of json config dist.
//        if((false == QDir::isAbsolutePath(conf)) && //is???Path nefunguje
//           (false == QDir::isRelativePath(conf)))
//            _str2json(conf.toLatin1());  //nejde o filepath -> zkusime primo json

        // default config
        QFile f_def(conf);  //from resources
        if(f_def.open(QIODevice::ReadOnly | QIODevice::Text)){

            QByteArray cons = f_def.read(64000);
            return _str2json(cons);
        }
        //nemuselo jit o soubor ale primo o json
        return _str2json(conf.toLatin1());
    }



//virtuals:
    /*! init privates from configuration */
    virtual int reload(int p) = 0;

    /*! here do the work */
    virtual int proc(i_rt_exchange *p) = 0;

public:
    QString name(){
        return _name;
    }

    void rename(QString name){
        _name = name;
    }

    int no(){
        return _no;
    }

    /*! key/async event processing */
    virtual bool event(QEvent* ev)
    {
        if (ev->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(ev);
            LOG(INFO) << fancy_name << "captured [" << ke->text << "] key";
        }
        return QObject::event(ev);
    }

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
    void connect(a_rt_base *src){

        if(!QObject::connect(src, SIGNAL(update(QSharedPointer<i_rt_exchange>)),
                         this, SLOT(on_update(QSharedPointer<i_rt_exchange>)))){

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

    a_rt_base(const QString &conf, QObject *parent = NULL):
        QObject(parent),
        par(_path2json(conf))
    {
        _no = rt_base_counter++;
        _name = QString("node[%1]").arg(_no);
        LOG(INFO) << _name << "created";
    }

    virtual ~a_rt_base(){;}
};

#endif // RT_BASE_H
