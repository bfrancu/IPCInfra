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
#include "BaseMemberPair.hpp"

#include "Traits/fifo_traits.hpp"
#include "Traits/storage_traits.hpp"
#include "default_traits.hpp"

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

/*
 * If we use the Acceptor with the NamedPipe TransporEndpoint working as a server,
 * we will use the first input we receive due to the listener unsubscribe and subscribe
 * mechanism employed by the Acceptor
 * The buffer read will be cleared before a client has the chance to register input Cb and consume it
 * We will use to LocalInputBuffer to make that first message available to the clients of the class
 */


template<typename Device, typename Key, typename = void>
class LocalInputBuffer;

template<typename Device, typename Key>
class LocalInputBuffer<Device, Key, std::enable_if_t<!IsNamedPipeDeviceT<Device>::value>>
{
public:
    using implemented = std::false_type;
};

template<typename Device, typename Key>
class LocalInputBuffer<Device, Key, std::enable_if_t<IsNamedPipeDeviceT<Device>::value>>
{
public:
    using implemented = std::true_type;

    void store(const Key & key, std::string_view data) {
        if (meta::traits::default_value<Key>::value == m_key){
            m_key = key;
        }

        if (key != m_key){
            return;
        }

        m_local_buffer.reserve(data.size());
        m_local_buffer.append(data.data(), data.size());
    };

    std::string & buffer() { return m_local_buffer; }
    const std::string & buffer() const { return m_local_buffer; }
    Key key() const { return m_key; }

    void clear() {
        m_local_buffer.clear();
        m_key = meta::traits::default_value<Key>::value;
    }

private:
    std::string m_local_buffer;
    //A named pipe device should have only one unique key correspondent (the local descriptor used for io)
    Key m_key{meta::traits::default_value<Key>::value};
};

template<typename Derived, typename Device, typename Storage = meta::tl::empty_type, typename = void>
class BaseDispatcherPolicy{
};

template<typename Derived, typename Device, typename Storage>
class BaseDispatcherPolicy<Derived, Device, Storage, std::enable_if_t<traits::is_endpoint_storage_v<Storage>>>
{
    using ClientEndpoint = typename Storage::endpoint_t;
    using ClientKey = typename Storage::key_t;
    using LocalBuffer = LocalInputBuffer<Device, ClientKey>;
public:
    using ClientInputAvailableCB = std::function<void(const ClientKey &, std::string_view)>;
    using ClientConnectedCB = std::function<void(const ClientKey &)>;
    using ClientDisconnectedCB = std::function<void(const ClientKey &)>;

    //Interface for users of the class
public:
    bool registerClientInputCallback(ClientInputAvailableCB cb){
        std::cout << "BaseDispatcherPolicy::registerInputCallback()\n";
        bool alreadySet{getClientInputAvailableCB()};
        getClientInputAvailableCB() = std::move(cb);

        if constexpr (LocalBufferAvailable()){
            if (!alreadySet){
                ProcessClientInput(getLocalBuffer().key(), getLocalBuffer().buffer());
                getLocalBuffer().clear();
            }
        }
        return true;
    }

    bool registerClientConnectionCallback(ClientConnectedCB cb){
        std::cout << "BaseDispatcherPolicy::registerClientConnectionCallback()\n";
        m_clientConnectedCB = std::move(cb);
        return true;
    }

    bool registerClientDisconnectionCallback(ClientDisconnectedCB cb){
        std::cout << "BaseDispatcherPolicy::registerClientDisconnectionCallback()\n";
        m_clientDisconnectedCB = std::move(cb);
        return true;
    }

protected:
    static constexpr bool LocalBufferAvailable() { return LocalBuffer::implemented::value; }

    bool ProcessInputEvent()  { return true;  }
    void ProcessDisconnection() {}

    void ProcessClientInput(const ClientKey & key, std::string_view data)
    {
        std::cout << "BaseDispatcherPolicy::ProcessClientInput: " << key << "\n";
        if (getClientInputAvailableCB()){
            getClientInputAvailableCB()(key, data);
            return;
        }
        if constexpr (LocalBufferAvailable()) {
            getLocalBuffer().store(key, data);
        }
        /*
        if (m_clientInputAvailableCB){
            m_clientInputAvailableCB(key, data);
        }
        */
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

    ClientInputAvailableCB & getClientInputAvailableCB() { return m_optionalBufferAndClientInputAvailableCB.member(); }
    LocalBuffer & getLocalBuffer() { return m_optionalBufferAndClientInputAvailableCB.base(); }

private:
    BaseMemberPair<LocalBuffer, ClientInputAvailableCB> m_optionalBufferAndClientInputAvailableCB;
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
        std::cout << "BaseDispatcherPolicy::registerInputCallback()\n";
        /*
        if (cb){
            std::cout << "BaseDispatcherPolicy::registerInputCallback() cb is valid \n";
        }
        else {
            std::cout << "BaseDispatcherPolicy::registerInputCallback() cb is not valid \n";
        }
        */

        m_inputAvailableCB = std::move(cb);
        //m_inputAvailableCB("2 10 2 10");
        return true;
    }

    inline bool registerDisconnectionCallback(DisconnectedCB cb) {
        std::cout << "BaseDispatcherPolicy::registerDisconnectionCallback()\n";
        m_disconnectionCB = std::move(cb);
        return false;
    }

protected: 
    bool ProcessInputEvent()
    {
        std::cout << "BaseDispatcherPolicy::ProcessInputEvent()\n";
        Device & device = this->asDerived().getDevice();
        ssize_t size_read = device.read(m_local_buffer);

        /*
        std::cout << "BaseDispatcherPolicy::ProcessInputEvent() local buffer : " <<  m_local_buffer
                  << "; size read: " << size_read << "\n";
        */

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
