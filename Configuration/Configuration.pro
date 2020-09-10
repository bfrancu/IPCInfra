QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../UtilitiesInfra/
DEPENDPATH += $$PWD/../UtilitiesInfra/

LIBS += -L../UtilitiesInfra/ -lUtilitiesInfra

HEADERS += \
    ConfigurationReader.h \
    ConfigurationBook.h

SOURCES += \
        ConfigurationReader.cpp \
        ConfigurationBook.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
