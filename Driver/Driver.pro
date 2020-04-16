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


LIBS += -L../IOInfra/ -lIOInfra
LIBS += -L../TransportInfra/ -lTransportInfra
LIBS += -L../UtilitiesInfra/ -lUtilitiesInfra

QMAKE_CXXFLAGS += -pthread
LIBS += -lpthread

SOURCES += \
        main.cpp

