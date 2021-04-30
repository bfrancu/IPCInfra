#ifndef FIFOIOPOLICY_H
#define FIFOIOPOLICY_H
#include "crtp_base.hpp"
#include "Traits/device_constraints.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"

namespace infra
{

template<typename Host, typename Device, typename = void>
class FifoIOPolicy{};

template<typename Host, typename Device>
class FifoIOPolicy<Host, traits::UnixNamedPipeDevice<Device>> : public crtp_base<FifoIOPolicy<Host, Device>, Host>

{
    using io_profile = typename Device::io_profile;

public:
    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, std::size_t>
    read(std::string & result){
        return utils::unx::LinuxIOUtilities::read(this->asDerived().getHandle(), result);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, std::size_t>
    readLine(std::string & result){
        return utils::unx::LinuxIOUtilities::readLine(this->asDerived().getHandle(), result);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<read_only_profile, typename T::io_profile>, std::size_t>
    readInBuffer(std::size_t buffer_len, char *buffer){
        return utils::unx::LinuxIOUtilities::readInBuffer(this->asDerived().getHandle(), buffer_len, buffer);
    }

    template<typename T = Device>
    std::enable_if_t<std::is_same_v<write_only_profile, typename T::io_profile>, ssize_t>
    write(std::string_view data){
        return utils::unx::LinuxIOUtilities::write(this->asDerived().getHandle(), data);
    }
};



} //infra



#endif // FIFOIOPOLICY_H
