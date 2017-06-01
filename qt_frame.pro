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

INCLUDEPATH += frame \
        filter \
        wav \
        control

#DEPENDPATH += $$PWD/control
#include(control/vi_interface.pri) - nefunguje?!

SOURCES += main.cpp \
    filter/f_props.cpp

HEADERS += \
    frame/rt_setup.h \
    frame/rt_exchange_i.h \
    frame/rt_base_a.h \
    frame/rt_audioinput.h \
    frame/rt_recorder.h \
    wav/wav_read_file.h \
    wav/wav_write_file.h \
    frame/rt_generator.h \
    frame/rt_wavinput.h \
    filter/filter_a.h \
    filter/f_windowing.h \
    frame/rt_processor.h \
    filter/f_props.h \
    filter/filter_fir.h \
    filter/filter_avr.h \
    frame/rt_audiooutput.h \
    control/i_comm_io_generic.h \
    control/i_comm_parser.h \
    control/t_comm_parser_string.h \
    control/t_comm_io_std.h \
    control/t_comm_io_tcp.h \
    control/t_comm_parser_binary_ex.h \
    control/t_comm_parser_binary.h
