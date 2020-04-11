TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../IOInfra/
DEPENDPATH += $$PWD/../IOInfra/
INCLUDEPATH += $$PWD/../TransportInfra/
DEPENDPATH += $$PWD/../TransportInfra/


LIBS += -L../IOInfra/ -lIOInfra
LIBS += -L../TransportInfra/ -lTransportInfra

SOURCES += \
        main.cpp

