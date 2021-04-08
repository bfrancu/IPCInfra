#ifndef GENERICDEVICEACCESS_HPP
#define GENERICDEVICEACCESS_HPP

namespace infra
{
class GenericDeviceAccess
{
    template<typename Device>
    friend class ProxyDevice;

    template<typename Host, typename Device, typename>
    friend class SeekableOperations;

    template<typename Host, typename Device, typename>
    friend class GenericIOPolicy;
    
    template<typename Host, typename Device, typename>
    friend class ResourceStatusPolicy;

    template<typename, typename>
    friend class Connector;

    template<typename Device, 
             template<typename...> typename,
             template<typename...> typename,
             typename Listener,
             typename, typename>
    friend class TransportEndpoint;

    template<typename Device>
    static decltype(auto) getHandle(const Device & device) { return device.getHandle(); }
};

} //infra
#endif
