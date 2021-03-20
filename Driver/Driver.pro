TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../IOInfra/
DEPENDPATH += $$PWD/../IOInfra/
INCLUDEPATH += $$PWD/../TransportInfra/
DEPENDPATH += $$PWD/../TransportInfra/
INCLUDEPATH += $$PWD/../UtilitiesInfra/
DEPENDPATH += $$PWD/../UtilitiesInfra/
INCLUDEPATH += $$PWD/../Configuration/
DEPENDPATH += $$PWD/../Configuration/


LIBS += -L../IOInfra/ -lIOInfra
LIBS += -L../TransportInfra/ -lTransportInfra
LIBS += -L../UtilitiesInfra/ -lUtilitiesInfra
LIBS += -L../Configuration/ -lConfiguration

QMAKE_CXXFLAGS += -pthread
LIBS += -lpthread

SOURCES += \
        main.cpp \
        meta.cpp \
        transport.cpp

HEADERS += \
    meta.h \
    transport.h

