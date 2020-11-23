#ifndef SOCKETDEVICEACCESS_H
#define SOCKETDEVICEACCESS_H
#include <utility>
#include "Devices/GenericDeviceAccess.hpp"

namespace infra
{

class SocketDeviceAccess 
{
    template<typename Host, typename SocketDevice, typename>
    friend class ConnectionPolicy;

    template<typename Host, typename SocketDevice, typename>
    friend class AcceptorPolicy;

    template<typename Host, typename SocketDevice, typename>
    friend class StreamIOPolicy;

    template<typename Host, typename SocketDevice, typename>
    friend class DatagramIOPolicy;

    /*
    template<typename>
    friend class Connector;
    */

    template<typename SocketDevice, typename Handle>
    static void setHandle(SocketDevice & device, Handle handle){ device.setHandle(handle); }

    template<typename SocketDevice, typename State>
    static void setState(SocketDevice & device, State state){ device.setState(state); }

    template<typename SocketDevice, typename SocketAddress>
    static void setWorkingAddress(const SocketDevice & device, SocketAddress && sock_addr){
        device.setWorkingAddress(std::forward<SocketAddress>(sock_addr)); }

    template<typename SocketDevice>
    static void close(SocketDevice & device){ device.close(); }

    template<typename SockeDevice>
    static decltype(auto) createSocketHandle() { return SockeDevice::createSocketHandle(); }

    template<typename SocketDevice>
    static decltype(auto) getHandle(const SocketDevice & device) { return device.getHandle(); }
};

} //infra

#endif // SOCKETDEVICEACCESS_H
