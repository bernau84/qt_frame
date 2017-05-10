#ifndef RT_AUDIOOUTPUT_H
#define RT_AUDIOOUTPUT_H

#include <stdlib.h>
#include <math.h>

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QIODevice>

#include "rt_base_a.h"

class t_rt_audiooutput : public a_rt_base {

    Q_OBJECT

private:
    QAudioDeviceInfo m_device;
    QAudioFormat m_format;
    QAudioOutput *m_audioOutput;
    QIODevice *m_output;

    /*! init privates from configuration */
    virtual int reload(int p){
        Q_UNUSED(p);
        return 0;
    }

    /*! here do the work */
    virtual int proc(i_rt_exchange *p){

        if(!p || !p->n())
            return 0;

        int i_fs = 1.0 / (p->f_t(1) - p->f_t(0));
        if(abs(m_format.sampleRate() - i_fs) > 1){  //rozdil do 2Hz ignorujemem

            on_start(i_fs);  //zmena vzorkovacky
        }

        QVector<int16_t> d(p->n());  //floatovy buffer si musime vyrobit
        for(int i=0; i < p->n(); i++) d[i] = p->f_a(i) * (1 << 15);

        return m_output->write((const char *)d.constData(), p->n());
    }

public:
    /*!  */
    void on_start(int p){

        Q_UNUSED(p);

        if(m_audioOutput)
            m_audioOutput->stop();

        m_format.setSampleRate((p) ? p : 8000);
        m_format.setChannelCount(1);
        m_format.setSampleSize(16);
        m_format.setSampleType(QAudioFormat::SignedInt);
        m_format.setByteOrder(QAudioFormat::LittleEndian);
        m_format.setCodec("audio/pcm");

        m_device = QAudioDeviceInfo::defaultOutputDevice();
        if (!m_device.isFormatSupported(m_format)) {
            LOG(WARNING) << "Default format not supported - trying to use nearest";
            m_format = m_device.nearestFormat(m_format);
        }

        LOG(INFO) << m_device.deviceName();
        m_audioOutput = new QAudioOutput(m_device, m_format, this);
        m_output = m_audioOutput->start();
    }

    /*! */
    void on_stop(int p){

        Q_UNUSED(p);
        if(m_audioOutput)
            m_audioOutput->stop();
    }

    t_rt_audiooutput(const QString &js_config, QObject *parent = NULL):
        a_rt_base(js_config, parent){

        m_audioOutput = NULL;
        m_output = NULL;
    }

    virtual ~t_rt_audiooutput(){}
};

#endif // RT_AUDIOOUTPUT_H
