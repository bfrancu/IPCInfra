#ifndef FIFOIOPOLICY_H
#define FIFOIOPOLICY_H
#include "IOPolicy.hpp"
#include "Devices/Pipes/NamedPipeDeviceAccess.hpp"
#include "Traits/device_constraints.hpp"

namespace infra
{

//template<typename Host, typename Device, typename = std::void_t<>>
template<typename Host, typename Device, typename = void>
class FifoIOPolicy{};

template<typename Host, typename Device>
class FifoIOPolicy<Host, traits::UnixNamedPipeDevice<Device>>
        : protected GenericIOPolicy<Host, Device>
{
    using io_profile = typename Device::io_profile;

public:
    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, std::size_t>
    read(std::string & result){
        return GenericIOPolicy<Host, Device>::readImpl(NamedPipeDeviceAccess::getHandle(asDerived()), result);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, std::size_t>
    readLine(std::string & result){
        return GenericIOPolicy<Host, Device>::readLine(NamedPipeDeviceAccess::getHandle(asDerived()), result);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, std::size_t>
    readInBuffer(std::size_t buffer_len, char *buffer){
        return GenericIOPolicy<Host, Device>::readInBuffer(NamedPipeDeviceAccess::getHandle(asDerived()), buffer_len, buffer);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<write_only_profile, typename T::io_profile>, ssize_t>
    write1(const std::string & data){
        return GenericIOPolicy<Host, Device>::write(NamedPipeDeviceAccess::getHandle(asDerived()), data);
    }

protected:
   Host & asDerived() { return *static_cast<Host*>(this); }
   const Host & asDerived() const { return *static_cast<Host const *>(this); }
};



} //infra



#endif // FIFOIOPOLICY_H
