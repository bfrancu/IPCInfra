#ifndef IOPOLICY_HPP
#define IOPOLICY_HPP

#include <stdint.h>
#include <unistd.h>
#include <cstring>

#include <string>
#include <algorithm>
#include <type_traits>

#include "crtp_base.hpp"
#include "Devices/GenericDeviceAccess.hpp"
#include "Traits/device_traits.hpp"
#include "Traits/device_constraints.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"
#include "StreamIOPolicy.hpp"
#include "SocketTypes.h"

namespace infra
{

template<typename Host, typename Device, typename = std::void_t<>>
class GenericIOPolicy{};

template<typename Host, typename Device>
class GenericIOPolicy<Host, Device, std::enable_if_t<IsUnixPlatformSocketDeviceT<Device>::value>> 
        //: public crtp_base<GenericIOPolicy<Host, Device>, Host>
        : public StreamIOPolicy<Host, Device>
{
     using handle_type = typename device_traits<Device>::handle_type;
     using Base = StreamIOPolicy<Host, Device>;
public:
     ssize_t read(std::string & result){
         return Base::recv(result.max_size(), SocketIOFlags{io::ESocketIOFlag::E_MSG_NOSIGNAL}, result); 
     }

     ssize_t readInBuffer(size_t buffer_len, char *buffer) {
         return Base::recvInBuffer(buffer_len, SocketIOFlags{io::ESocketIOFlag::E_MSG_NOSIGNAL}, buffer);
     }
     
     ssize_t write(std::string_view data){
         return Base::send(data, SocketIOFlags{io::ESocketIOFlag::E_MSG_NOSIGNAL});
     }

protected:
    ~GenericIOPolicy() = default;
};


template<typename Host, typename Device>
class GenericIOPolicy<Host, Device, std::enable_if_t<HasUnixHandleTypeT<Device>::value &&
                                                     !IsUnixPlatformSocketDeviceT<Device>::value>>
        : public crtp_base<GenericIOPolicy<Host, Device>, Host>
{
     using handle_type = typename device_traits<Device>::handle_type;
     //using base = crtp_base<GenericIOPolicy<Host, Device>, Host>;

public:
    ssize_t read(std::string & result){ return utils::unx::LinuxIOUtilities::read(GenericDeviceAccess::getHandle(this->asDerived()), result);}

    size_t readLine(std::string & result){ return utils::unx::LinuxIOUtilities::readLine(GenericDeviceAccess::getHandle(this->asDerived()), result); }

    ssize_t readInBuffer(size_t buffer_len, char *buffer){ return utils::unx::LinuxIOUtilities::readInBuffer(GenericDeviceAccess::getHandle(this->asDerived()), buffer_len, buffer); }

    ssize_t write(std::string_view data) { return utils::unx::LinuxIOUtilities::write(GenericDeviceAccess::getHandle(this->asDerived()), data); }

protected:
    ~GenericIOPolicy() = default;
};

} //infra

#endif // IOPOLICY_HPP
