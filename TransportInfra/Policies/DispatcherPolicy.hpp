#ifndef DISPATCHER_POLICY_HPP
#define DISPATCHER_POLICY_HPP
#include <functional>
#include <string_view>
#include <string>

#include "traits_utils.hpp"
#include "SocketTypes.h"

namespace infra
{

DEFINE_HAS_MEMBER(read);
DEFINE_HAS_MEMBER(send);

template<typename Device,
         typename = std::enable_if_t<has_member_read<Device>::value>>
using ReadableDevice = Device;

template<typename Derived, typename Device, typename = void>
class BaseDispatcherPolicy{
    static_assert(has_member_read<Device>::value, "The Device should have a policy implementing a \"ssize_t read(std::string &)\" method");
};

template<typename Derived, typename Device>
class BaseDispatcherPolicy<Derived, ReadableDevice<Device>>
{
public:
    using InputAvailableCB = std::function<void(std::string_view)>;
    using DisconnectedCB = std::function<void()>;

public:
    inline bool registerInputCallback(InputAvailableCB cb) {
        m_inputAvailableCB = std::move(cb);
        return true;
    }
    inline bool registerDisconnectionCallback(DisconnectedCB cb) {
        m_disconnectionCB = std::move(cb);
        return false;
    }

protected: 
    bool ProcessInputEvent()
    {
        Device & device = this->asDerived().getDevice();
        ssize_t size_read = device.read(m_local_buffer);
        if (-1 == size_read){
            this->asDerived().onErrorEvent();
            return false;
        }
        if (m_inputAvailableCB){
            m_inputAvailableCB(m_local_buffer);
        }
        m_local_buffer.clear();
        return true;
    }

    void ProcessDisconnection()
    {
        if (m_disconnectionCB){
            m_disconnectionCB();
        }
    }

protected:
    ~BaseDispatcherPolicy() = default;

    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }

private:
    InputAvailableCB m_inputAvailableCB;
    DisconnectedCB m_disconnectionCB;
    std::string m_local_buffer{};
};

}//infra

#endif
