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
    QVector<int16_t> m_buffer;
    QVector<int16_t>::Iterator m_it;

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

        for(int i=0; i < p->n(); i++){

            *m_it++ = p->f_a(i) * (1 << 15);
            if(m_it == m_buffer.end()){

                m_it = m_buffer.begin();
                m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t));
                //LOG(INFO) << m_audioOutput->bufferSize() - m_audioOutput->bytesFree() << "samples playback queued";
            }
        }

        return p->n();
    }

public:
    /*!  */
    void on_start(int p){

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

        LOG(INFO) << "rt_audiooutput::periodSize()" << m_audioOutput->periodSize();
        m_buffer.resize(m_audioOutput->periodSize() / sizeof(int16_t));  //na rozdil od reserve nuluje
        m_it = m_buffer.begin();

        LOG(INFO) << "rt_audiooutput::bufferSize()" << m_buffer.size();
        m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t)); //o jednu periodu posuneme plaback aby v tom nelupalo
        m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t)); //o jednu periodu posuneme plaback aby v tom nelupalo
//        m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t)); //o jednu periodu posuneme plaback aby v tom nelupalo
//        m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t)); //o jednu periodu posuneme plaback aby v tom nelupalo
//        m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t)); //o jednu periodu posuneme plaback aby v tom nelupalo
//        m_output->write((const char *)m_buffer.constData(), m_buffer.size()*sizeof(int16_t)); //o jednu periodu posuneme plaback aby v tom nelupalo

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
