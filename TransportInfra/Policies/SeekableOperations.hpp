#ifndef SEEKABLEOPERATIONS_HPP
#define SEEKABLEOPERATIONS_HPP

#include <stdint.h>
#include <unistd.h>

#include "crtp_base.hpp"
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
        public crtp_base<SeekableOperations<Host, Device>, Host>
{
public:
    uint64_t currentPosition() const{
        return ::lseek(this->asDerived().getHandle(), 0, SEEK_CUR);
    }

    bool seek(int64_t distance_from, io::ESeekWhence origin){
        return -1 != ::lseek(this->asDerived().getHandle(), distance_from, static_cast<int>(origin));
    }

    bool seekFromStart(uint64_t pos_from_start){
        return seek(pos_from_start, io::ESeekWhence::E_SEEK_SET);
    }

    bool seekFromCurrent(int64_t pos_from_current){
        return seek(pos_from_current, io::ESeekWhence::E_SEEK_CUR);
    }

    bool goToStartPosition(){
        auto current_pos = currentPosition();
        if (-1 == current_pos) return false;
        return seekFromCurrent(-current_pos);
    }
};

} //infra

#endif // SEEKABLEOPERATIONS_HPP
