QT -= gui

TEMPLATE = lib
DEFINES += UTILITIESINFRA_LIBRARY

CONFIG += c++17

INCLUDEPATH += $$PWD/../IOInfra/
DEPENDPATH += $$PWD/../IOInfra/

LIBS += -L../IOInfra/ -lIOInfra

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    InetUtils/InetUtilities.cpp \
    LinuxUtils/LinuxIOUtilities.cpp \
    runtime_dispatcher.cpp

HEADERS += \
    AccessContextHierarchy.hpp \
    Host.hpp \
    InetUtils/InetUtilities.h \
    LinuxUtils/LinuxIOUtilities.h \
    ConversionUtils.h \
    Observable.hpp \
    ObservableStatePublisher.hpp \
    PoliciesHolder.hpp \
    UtilitiesInfra_global.h \
    crtp_base.hpp \
    enum_flag.h \
    randomizer.hpp \
    shared_lookup_table.hpp \
    runtime_dispatcher.hpp \
    function_traits.hpp \
    pointer_traits.hpp \
    shared_queue.hpp \
    sys_call_eval.h \
    traits_utils.hpp \
    utilities.hpp \
    typelist.hpp \
    policies_initializer.hpp \
    template_typelist.hpp

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
