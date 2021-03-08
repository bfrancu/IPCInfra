#ifndef RESOURCESTATUSPOLICY_HPP
#define RESOURCESTATUSPOLICY_HPP
#include <stdint.h>
#include <unistd.h>

#include "crtp_base.hpp"
#include "FileInfo.h"
#include "FileStatusFlags.h"
#include "FilePermissions.h"
#include "Devices/GenericDeviceAccess.hpp"
#include "Traits/handler_traits.hpp"
#include "Traits/device_constraints.hpp"

namespace infra
{

template<typename Host, typename Device, typename = void>
class ResourceStatusPolicy
{};

template<typename Host, typename Device>
class ResourceStatusPolicy<Host, Device, std::void_t<traits::UnixDevice<Device>>>
        : public crtp_base<ResourceStatusPolicy<Host, Device>, Host>
{

public:
    bool isReadable() const{
        using namespace io;
        FileStatusFlags flags{GenericDeviceAccess::getHandle(this->asDerived())};
        return (EAccessMode::E_READ_ONLY == flags.accessMode() || EAccessMode::E_READ_WRITE == flags.accessMode());
    }

    bool isWritable() const{
        using namespace io;
        FileStatusFlags flags{GenericDeviceAccess::getHandle(this->asDerived())};
        return (EAccessMode::E_WRITE_ONLY == flags.accessMode() || EAccessMode::E_READ_WRITE == flags.accessMode());
    }

    inline bool isTty() const {
        return 1 == ::isatty(GenericDeviceAccess::getHandle(this->asDerived()));
    }

    inline uint64_t position() const{
        return ::lseek(GenericDeviceAccess::getHandle(this->asDerived()), 0, SEEK_CUR);
    }

    inline off_t bytesAvailable() const {
        return fileInfo().availableSize();
    }

    inline io::EFileType fileType() const{
        return fileInfo().type();
    }

    inline io::EAccessMode accessMode() const{
        return fileStatusFlags().accessMode();
    }

    inline io::FilePermissions permissions() const{
        return fileInfo().permissions();
    }

protected:
    inline io::FileInfo fileInfo() const{
        return io::FileInfo{GenericDeviceAccess::getHandle(this->asDerived())};
    }

    inline io::FileStatusFlags fileStatusFlags() const{
        return io::FileStatusFlags{GenericDeviceAccess::getHandle(this->asDerived())};
    }

    /*
private:
    Derived & asDerived(){
        return *static_cast<Derived*>(this);
    }
    const Derived & asDerived() const{
        return *static_cast<Derived const*>(this);
    }*/
};

}

#endif // RESOURCESTATUSPOLICY_HPP
