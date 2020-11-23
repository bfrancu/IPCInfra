#ifndef SEEKABLEOPERATIONS_HPP
#define SEEKABLEOPERATIONS_HPP

#include <stdint.h>
#include <unistd.h>

#include "crtp_base.hpp"
#include "Devices/GenericDeviceAccess.hpp"
#include "FileIODefinitions.h"
#include "Traits/device_traits.hpp"

namespace infra
{

template<typename Host, typename Device, typename = std::void_t<>>
class SeekableOperations
{};


//template <typename Device>
//using SequentialUnixDeviceT = std::void_t<UnixHandlerT<Device>, SequentialDeviceT<Device>>;

//template <typename Host, typename Device>
//using SeekableOperationsT = SeekableOperations<Host, Device, std::void_t<SequentialUnixDeviceT<Device>>>;

template<typename Host, typename Device>
class SeekableOperations<Host, Device, std::void_t<std::enable_if_t<IsSeekableDeviceT<Device>::value>>> :
        public crtp_base<SeekableOperations<Host, Device, std::void_t<std::enable_if_t<IsSeekableDeviceT<Device>::value>>>, Host>
{
public:
    uint64_t position() const{
        return ::lseek(GenericDeviceAccess::getHandle(this->asDerived()), SEEK_CUR);
    }

    bool seek(int64_t distance_from, io::ESeekWhence origin){
        return -1 != ::lseek(GenericDeviceAccess::getHandle(this->asDerived()), distance_from, static_cast<int>(origin));
    }

    bool seekFromStart(uint64_t pos_from_start){
        return seek(pos_from_start, io::ESeekWhence::E_SEEK_SET);
    }

    bool seekFromCurrent(int64_t pos_from_current){
        return seek(pos_from_current, io::ESeekWhence::E_SEEK_CUR);
    }

    bool goToStartPosition(){
        auto current_pos = position();
        if (-1 == current_pos) return false;
        return seekFromCurrent(-current_pos);
    }
};

} //infra

#endif // SEEKABLEOPERATIONS_HPP
