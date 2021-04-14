QT -= gui

TEMPLATE = lib
DEFINES += TRANSPORTINFRA_LIBRARY

CONFIG += c++17

INCLUDEPATH += $$PWD/../UtilitiesInfra/
DEPENDPATH += $$PWD/../UtilitiesInfra/
INCLUDEPATH += $$PWD/../IOInfra/
DEPENDPATH += $$PWD/../IOInfra/
INCLUDEPATH += $$PWD/../Configuration/
DEPENDPATH += $$PWD/../Configuration/

LIBS += -L../UtilitiesInfra/ -lUtilitiesInfra
LIBS += -L../IOInfra/ -lIOInfra
LIBS += -L../Configuration/ -lConfiguration

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
    ConnectorClient.cpp \
    Devices/Pipes/NamedPipeFactory.cpp \
    Devices/Sockets/UnixSocketAddress.cpp \
    Policies/UnixResourceHandler.cpp \
    Reactor/EpollDemultiplexer.cpp \
    Reactor/EpollDemux.cpp

HEADERS += \
    Connector.hpp \
    Devices/DeviceFactory.hpp \
    Devices/DeviceAddressFactory.hpp \
    Devices/EmptyDevice.hpp \
    Devices/FileDevice.hpp \
    Devices/GenericDeviceAccess.hpp \
    Devices/Pipes/NamedPipeDevice.hpp \
    Devices/Pipes/NamedPipeDeviceAccess.hpp \
    Devices/Pipes/NamedPipeFactory.h \
    Devices/Pipes/NamedPipeAddress.h \
    Devices/Sockets/HostAddress.hpp \
    Devices/Sockets/InetSocketAddress.hpp \
    Devices/Sockets/SocketDevice.hpp \
    Devices/Sockets/SocketDeviceAccess.hpp \
    Devices/Sockets/UnixSocketAddress.h \
    Devices/TemporaryFileDevice.hpp \
    Devices/TestDevice.h \
    Devices/DeviceDefinitions.h \
    Policies/AcceptorPolicy.hpp \
    Policies/ConnectionPolicy.hpp \
    Policies/DatagramIOPolicy.hpp \
    Policies/ErrorChangeAdvertiserPolicy.h \
    Policies/FifoIOPolicy.hpp \
    Policies/IOPolicy.hpp \
    Policies/ResourceStatusPolicy.hpp \
    Policies/SeekableOperations.hpp \
    Policies/StateChangeAdvertiserPolicy.hpp \
    Policies/StreamIOPolicy.hpp \
    Policies/DispatcherPolicy.hpp \
    Policies/EventHandlingPolicy.hpp \
    Policies/ExporterPolicy.hpp \
    Policies/UnixResourceHandler.h \
    Reactor/DeviceTestEventHandler.h \
    Reactor/EpollDemultiplexer.h \
    Reactor/EpollDemux.h \
    Reactor/EventHandlerSubscriber.h \
    Reactor/EventTypes.h \
    Reactor/Reactor.hpp \
    Reactor/SubscriberInfo.hpp \
    Traits/device_traits.hpp \
    Traits/handler_traits.hpp \
    Traits/socket_traits.hpp \
    Traits/fifo_traits.hpp \
    Traits/device_constraints.hpp \
    Traits/transport_traits.hpp \
    ConnectionParameters.hpp \
    ConnectorClient.h \
    TransportEndpoint.hpp \
    DynamicTransportEndpointAdaptor.h \
    TransportInfra_global.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
