#ifndef DISPATCHER_POLICY_HPP
#define DISPATCHER_POLICY_HPP
#include <functional>
#include <iostream>
#include <memory>
#include <string_view>
#include <string>

#include "traits_utils.hpp"
#include "typelist.hpp"
#include "SocketTypes.h"

#include "Traits/storage_traits.hpp"

namespace infra
{

DEFINE_HAS_MEMBER(read);
DEFINE_HAS_MEMBER(send);

template<typename Device,
         typename = std::enable_if_t<has_member_read<Device>::value>>
using ReadableDevice = Device;

/*
template<typename Derived, typename Device, typename = void>
class BaseServerDispatcherPolicy
{

    //registerClientInputCallback(cb)
    //registerClientDisconnectionCallback(cb)
    //registerClientConnectedCallback(cb)
protected:
    bool ProcessInputEvent() { return true; }

    void ProcessDisconnection() {}
};
*/

template<typename Derived, typename Device, typename Storage = meta::tl::empty_type, typename = void>
class BaseDispatcherPolicy{
};

template<typename Derived, typename Device, typename Storage>
class BaseDispatcherPolicy<Derived, Device, Storage, std::enable_if_t<traits::is_endpoint_storage_v<Storage>>>
{
    using ClientEndpoint = typename Storage::endpoint_t;
    using ClientKey = typename Storage::key_t;
public:
    using ClientInputAvailableCB = std::function<void(const ClientKey &, std::string_view)>;
    using ClientConnectedCB = std::function<void(const ClientKey &)>;
    using ClientDisconnectedCB = std::function<void(const ClientKey &)>;

    //Interface for users of the class
public:
    bool registerClientInputCallback(ClientInputAvailableCB cb){
        m_clientInputAvailableCB = std::move(cb);
        return true;
    }

    bool registerClientConnectionCallback(ClientConnectedCB cb){
        m_clientConnectedCB = std::move(cb);
        return true;
    }

    bool registerClientDisconnectionCallback(ClientDisconnectedCB cb){
        m_clientDisconnectedCB = std::move(cb);
        return true;
    }

protected:
    bool ProcessInputEvent() { return true; }
    void ProcessDisconnection() {}

    void ProcessClientInput(const ClientKey & key, std::string_view data)
    {
        std::cout << "BaseDispatcherPolicy::ProcessClientInput: " << key << "\n";
        if (m_clientInputAvailableCB){
            m_clientInputAvailableCB(key, data);
        }
    }

    void ProcessClientConnected(const ClientKey & key)
    {
        std::cout << "BaseDispatcherPolicy::ProcessClientConnected: " << key << "\n";
        if (m_clientConnectedCB){
            m_clientConnectedCB(key);
        }
    }

    void ProcessClientDisconnected(const ClientKey & key)
    {
        std::cout << "BaseDispatcherPolicy::ProcessClientDisconnected: " << key << "\n";
        if (m_clientDisconnectedCB){
            m_clientDisconnectedCB(key);
        }
    }

private:
    ClientInputAvailableCB m_clientInputAvailableCB;
    ClientConnectedCB m_clientConnectedCB;
    ClientDisconnectedCB m_clientDisconnectedCB;
};

template<typename Derived, typename Device>
class BaseDispatcherPolicy<Derived, ReadableDevice<Device>, meta::tl::empty_type>
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
        std::cout << "BaseDispatcherPolicy::ProcessDisconnection()\n";
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
