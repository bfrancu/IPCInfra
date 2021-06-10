#ifndef GENERICDEVICEACCESS_HPP
#define GENERICDEVICEACCESS_HPP

namespace infra
{
class GenericDeviceAccess
{
    template<typename Device>
    friend class ProxyDevice;

    template<typename, typename>
    friend class Connector;

    template<typename, typename>
    friend class Acceptor;

    template<typename, typename>
    friend class DeviceTestEventHandler;

    template<typename Device, 
             template<typename...> typename,
             template<typename...> typename,
             template<typename...> typename,
             typename Listener,
             typename Storage,
             typename, typename>
    friend class TransportEndpoint;

    template<typename Device>
    static decltype(auto) getHandle(const Device & device) { return device.getHandle(); }
};

} //infra
#endif
