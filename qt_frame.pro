QT += core multimedia
QT -= gui

DEFINES += ELPP_QT_LOGGING    \
          ELPP_STL_LOGGING   \
          ELPP_STRICT_SIZE_CHECK \
          ELPP_THREAD_SAFE

CONFIG += c++11

TARGET = qt_frame
#CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    f_props.cpp

HEADERS += \
    rt_setup.h \
    rt_exchange_i.h \
    rt_base_a.h \
    rt_audioinput.h \
    rt_recorder.h \
    wav_read_file.h \
    wav_write_file.h \
    rt_generator.h \
    rt_wavinput.h \
    filter_a.h \
    f_windowing.h \
    rt_processor.h \
    f_props.h \
    filter_fir.h \
    filter_avr.h
