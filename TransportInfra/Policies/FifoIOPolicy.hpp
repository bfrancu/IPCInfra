#ifndef FIFOIOPOLICY_H
#define FIFOIOPOLICY_H
#include "IOPolicy.hpp"

namespace infra
{

template<typename Host, typename Device, typename = std::void_t<>>
class FifoIOPolicy{};

template<typename Host, typename Device>
class FifoIOPolicy<Host, Device, std::void_t<std::enable_if_t<std::conjunction_v<HasUnixHandleTypeT<Device>,
                                                                                 IsFifoDeviceT<Device>>>>>
        : protected GenericIOPolicy<Host, Device>
{
    using io_profile = typename Device::io_profile;

public:
    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, size_t>
    read(std::string & result){
        return GenericIOPolicy<Host, Device>::read(static_cast<const Host &>(*this).getHandle(), result);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, size_t>
    readLine(std::string & result){
        return GenericIOPolicy<Host, Device>::readLine(static_cast<const Host &>(*this).getHandle(), result);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, size_t>
    readInBuffer(size_t buffer_len, char *buffer){
        return GenericIOPolicy<Host, Device>::readInBuffer(static_cast<const Host &>(*this).getHandle(), buffer_len, buffer);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<write_only_profile, typename T::io_profile>, ssize_t>
    write1(const std::string & data){
        return GenericIOPolicy<Host, Device>::write(static_cast<const Host &>(*this).getHandle(), data);
    }
};



} //infra



#endif // FIFOIOPOLICY_H
