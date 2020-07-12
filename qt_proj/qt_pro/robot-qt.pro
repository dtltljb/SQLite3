#QT += core
#QT -= gui

CONFIG += c++11

TARGET = robot-qt
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += thread
CONFIG += -lrt
CONFIG += -static
TEMPLATE = app


INCLUDEPATH +=\
    ../\
    ../../\
    ../../src/\
    ../../src/db_libs/\
    ../../src/download/\
    ../../src/TimerQueue/\
    ../../src/TimerQueue/Logger/\
    ../../third_party/spdlog/include/\
    ../../third_party/json/single_include/\
    ../../third_party/concurrentqueue/\
    ../../third_party/serial/include/\
    ../../third_party/unit/

HEADERS += \
    ../../src/db_libs/table_opration.h \

SOURCES += \
    ../../src/app.cpp\
    ../../src/db_libs/select.cpp\
    ../../src/db_libs/table_opration.cpp\
    ../../src/db_libs/db_entry.cpp\
    ../../src/download/down_load_file.cpp\
    ../../src/download/update_version.cpp\
    ../../src/download/forlder_util.cpp \
    ../../src/download/infor_unpacket.cpp \
    ../../src/download/version_infor.cpp

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/curls/lib/ -lcurl

INCLUDEPATH += $$PWD/../../../../../../usr/local/curls/include
DEPENDPATH += $$PWD/../../../../../../usr/local/curls/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -levent

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -levent_pthreads

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include



unix:!macx: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -lrt

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../../usr/local/lib/librt.a

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/lib/x86_64-linux-gnu/ -lsqlite3

INCLUDEPATH += $$PWD/../../../../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../../usr/lib/x86_64-linux-gnu

