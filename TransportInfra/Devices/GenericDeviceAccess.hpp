#ifndef GENERICDEVICEACCESS_HPP
#define GENERICDEVICEACCESS_HPP

namespace infra
{
class GenericDeviceAccess
{
    template<typename Host, typename Device, typename>
    friend class SeekableOperations;

    template<typename Host, typename Device, typename>
    friend class GenericIOPolicy;
    
    template<typename Host, typename Device, typename>
    friend class ResourceStatusPolicy;

    template<typename, typename>
    friend class Connector;

    template<typename Device>
    static decltype(auto) getHandle(const Device & device) { return device.getHandle(); }
};

} //infra
#endif
