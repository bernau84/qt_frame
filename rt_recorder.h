#ifndef RT_RECORDER_H
#define RT_RECORDER_H

#include "rt_base_a.h"
#include "wav_write_file.h"

class t_rt_recorder : public a_rt_base {

    Q_OBJECT

private:
    QString path;
    int fs;

    t_waw_file_writer *file;

    /*! init privates from configuration */
    virtual int reload(int p){

        Q_UNUSED(p);
        return 0;
    }

    /*! here do the work */
    virtual int proc(i_rt_exchange *p){

        if(!p)
            return 0;

        int i_fs = p->f_f(0) * 2; //z nyguista na fs + zaokrouhleni na cele Hz
        if(fs != i_fs)
            on_stop(0);  //zmena vzorkovacky - musime zalozit novy soubor

        /*! \todo novy soubor pod novym (unikatnim, inkeremntovanym, timestampoym) nazvem
         * podobnou podminku i na maximalni velikost souboru */

        if(p->n() == 0)
            return 0;

        if(file == NULL){

            fs = i_fs;
            file = (t_waw_file_writer *) new t_waw_file_writer(path.toLatin1().constData(), fs);
            /*! \todo mohl bych rozmyslet jak vyuzit vicekanalovy wav kdybych umel rozeznat
             * a mel ocislovane ruzne zdroje signalu */
        }

        if(p->f_a())
            return file->write(p->f_a(), p->n());

        QVector<double> d(p->n());  //floatovy buffer si musime vyrobit
        for(int i=0; i < p->n(); i++) d[i] = p->f_a(i);

        return file->write(d.constData(), p->n());
    }

public slots:
    /*!  */
    virtual void on_start(int p){

        Q_UNUSED(p);
        fs = 0; //forces new recording file be created
    }

    /*! */
    virtual void on_stop(int p){

        Q_UNUSED(p);
        if(file) delete file;  //close recent write file
        file = NULL;
    }

public:
    t_rt_recorder(const QString &js_config, QObject *parent = NULL):
        a_rt_base(js_config, parent)
    {
        t_setup_entry e;

        path = (par.ask("path", &e)) ? e.get().toString() : "out.wav";
        file = NULL;
    }
};

#endif // RT_RECORDER_H
