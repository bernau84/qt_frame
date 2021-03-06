#ifndef RT_AUDIOINPUT_H
#define RT_AUDIOINPUT_H

#include <stdlib.h>
#include <math.h>

#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QIODevice>

#include "rt_base_a.h"

class t_rt_audioinput;

/*! \todo prejmenovat asi na mic input
 * + do konstruktoru dat nazev pro m_device pokud se nema brat defaultni
 * */

class t_rt_audioinput_ex : public i_rt_exchange {

private:
    friend class t_rt_audioinput;

    QVector<int16_t> a;
    double t;
    double f;

    /*! length of individual array */
    virtual int n(){ return a.size(); }

    /*! indexed acces */
    virtual double f_t(unsigned i){ return (f > 0) ? (t + ((i+1) / f)) : t; }
    virtual double f_a(unsigned i){ return (1.0 * a[i]) / (1 << 15); }
    virtual double f_f(unsigned i){ i = i; return f; }

    /*! pointer to array */
    virtual const double *f_a(){ return NULL; }  //nemame - mame jen u16
    virtual const double *f_f(){ return NULL; }
    virtual const double *f_t(){ return NULL; }

    t_rt_audioinput_ex(){}
    virtual ~t_rt_audioinput_ex(){
        // LOG(TRACE) << "released";
    }
};

class t_rt_audioinput : public a_rt_base {

    Q_OBJECT

private:
    QAudioDeviceInfo m_device;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QIODevice *m_input;
    t_rt_audioinput_ex *m_data;
    double m_t;

    /*! init privates from configuration */
    virtual int reload(int p){
        Q_UNUSED(p);
        return 0;
    }

    /*! here do the work */
    virtual int proc(i_rt_exchange *p){

        Q_UNUSED(p);

        qint64 samples, len = 0;

        if((m_data = new t_rt_audioinput_ex()))  //alokujem novy buffer - stary se uvolni diky SharedPnt
            if((len = m_audioInput->bytesReady())){

                samples = len / (m_format.sampleSize()/8);
                m_data->a.resize(samples);  //reserve space

                if((len = m_input->read((char *)m_data->a.data(), len))){

                    m_data->f = m_format.sampleRate() / 2;  //rozliseni jednoho vzorku je nyguistovka
                    m_data->t = m_t;
#if 0
                    double rms = 0;
                    for(int i=0; i<samples; i++) rms += pow(m_data->a[i], 2);
                    rms = sqrt(rms) / len;
                    LOG(INFO) << "rms=" << rms;
#endif //1
                    QSharedPointer<i_rt_exchange> pp(m_data);
                    emit update(pp);

                    //pripravime pristi casovou znacku
                    m_t += (1.0 * samples) / m_format.sampleRate();
                    LOG(INFO) << m_t << "[s]/" << samples << "samples";
                }
            }

        return 0;
    }

signals:
    void forward(QSharedPointer<i_rt_exchange> d); //pomocny signal

private slots:
    void on_forward(){  //pomocny slot

        QSharedPointer<i_rt_exchange> dm; //dummy
        dm.clear();
        emit forward(dm);
    }

public:
    /*!  */
    void on_start(int p){

        Q_UNUSED(p);

        if(m_audioInput)
            m_audioInput->stop();

        m_format.setSampleRate(8000);
        m_format.setChannelCount(1);
        m_format.setSampleSize(16);
        m_format.setSampleType(QAudioFormat::SignedInt);
        m_format.setByteOrder(QAudioFormat::LittleEndian);
        m_format.setCodec("audio/pcm");

        m_device = QAudioDeviceInfo::defaultInputDevice();
        if (!m_device.isFormatSupported(m_format)) {
            LOG(WARNING) << "Default format not supported - trying to use nearest";
            m_format = m_device.nearestFormat(m_format);
        }

        LOG(INFO) << m_device.deviceName();
        m_audioInput = new QAudioInput(m_device, m_format, this);

        m_input = m_audioInput->start();
        QObject::connect(m_input, SIGNAL(readyRead()), this, SLOT(on_forward()));
        QObject::connect(this, SIGNAL(forward(QSharedPointer<i_rt_exchange>)),
                         this, SLOT(on_update(QSharedPointer<i_rt_exchange>)));

        m_t = 0;
    }

    /*! */
    void on_stop(int p){

        Q_UNUSED(p);
        if(m_audioInput)
            m_audioInput->stop();
    }

    t_rt_audioinput(const QString &js_config, QObject *parent = NULL):
        a_rt_base(js_config, parent){

        m_data = NULL;
        m_audioInput = NULL;
        m_input = NULL;
    }

    virtual ~t_rt_audioinput(){}
};

#endif // RT_AUDIOINPUT_H
