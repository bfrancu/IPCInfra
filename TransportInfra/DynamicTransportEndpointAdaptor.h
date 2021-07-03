#ifndef DYNAMIC_TRANSPORT_ENDPOINT_ADAPTOR_H
#define DYNAMIC_TRANSPORT_ENDPOINT_ADAPTOR_H
#include <memory>
#include <string_view>
#include <functional>

#include "runtime_dispatcher.hpp"
#include "typelist.hpp"

#include "Traits/transport_traits.hpp"
#include "Reactor/EventTypes.h"

class ITransportEndpoint;

namespace infra
{

class DefinitionsContainer
{
    DEFINE_DISPATCH_TO_MEMBER_MAPPED(send);
    //DEFINE_DISPATCH_TO_MEMBER(send);
    template<typename> friend class DynamicTransportEndpointAdapter;
};

class ClientDefinitionsContainer
{
    DEFINE_DISPATCH_TO_MEMBER_MAPPED(connect);
    DEFINE_DISPATCH_TO_MEMBER_MAPPED(registerInputCallback);
    DEFINE_DISPATCH_TO_MEMBER_MAPPED(registerDisconnectionCallback);
    /*
    DEFINE_DISPATCH_TO_MEMBER(connect);
    DEFINE_DISPATCH_TO_MEMBER(registerInputCallback);
    DEFINE_DISPATCH_TO_MEMBER(registerDisconnectionCallback);
    */
    template<typename> friend class ClientDynamicTransportEndpointAdapter;
};

class ServerDefinitionsContainer
{
   DEFINE_DISPATCH_TO_MEMBER_MAPPED(bind);
   DEFINE_DISPATCH_TO_MEMBER_MAPPED(registerClientInputCallback);
   DEFINE_DISPATCH_TO_MEMBER_MAPPED(registerClientConnectionCallback);
   DEFINE_DISPATCH_TO_MEMBER_MAPPED(registerClientDisconnectionCallback);
   /*
   DEFINE_DISPATCH_TO_MEMBER(bind);
   DEFINE_DISPATCH_TO_MEMBER(registerClientInputCallback);
   DEFINE_DISPATCH_TO_MEMBER(registerClientConnectionCallback);
   DEFINE_DISPATCH_TO_MEMBER(registerClientDisconnectionCallback);
   */
   template<typename> friend class ServerDynamicTransportEndpointAdapter;
};

template<typename EndpointTList>
class DynamicTransportEndpointAdapter
{
public:
    using InputAvailableCB = std::function<void(std::string_view)>;
    using DisconnectedCB = std::function<void()>;

public:
    DynamicTransportEndpointAdapter() = default;
    DynamicTransportEndpointAdapter(std::unique_ptr<ITransportEndpoint> p_endpoint, std::size_t device_tag) :
        m_pEndpoint{std::move(p_endpoint)},
        m_device_tag{device_tag}
    {}

    virtual ~DynamicTransportEndpointAdapter() = default;

    virtual bool init(std::unique_ptr<ITransportEndpoint> p_endpoint, std::size_t device_tag)
    {
        m_pEndpoint = std::move(p_endpoint);
        m_device_tag = device_tag;
        return true;
    }

    template<typename... Args>
    decltype(auto) send(Args&&... args)
    {
        using return_t = typename Def::send_dispatcher_mapped<EndpointTList, type_to_device_tag>::return_send_variant;
        //using return_t = typename Def::send_dispatcher<EndpointTList>::return_send_variant;
        return_t ret;
        if (m_pEndpoint)
        {
            auto wrapper = meta::dispatch::wrap(m_pEndpoint->getInternalEndpoint());
            ret = Def::send_dispatcher_mapped<EndpointTList, type_to_device_tag>::call(this->getDeviceTag(), wrapper, std::forward<Args>(args)...);
            //ret = Def::send_dispatcher<EndpointTList>::call(this->getDeviceTag(), wrapper, std::forward<Args>(args)...);
        }
        return ret;
    }

    inline bool listenerSubscribe(const events_array & events) { 
        if (!m_pEndpoint) return false;
        return m_pEndpoint->listenerSubscribe(events); 
    }

    inline bool listenerUnsubscribe() {
        std::cout << "DynamicTransportEndpointAdapter::listenrUnsubscribe()\n";
        if (!m_pEndpoint) return false;
        return m_pEndpoint->listenerUnsubscribe();
    }

    inline bool subscribedToListener() const { 
        if (!m_pEndpoint) return false;
        return m_pEndpoint->subscribedToListener();
    }

    inline void setDeviceTag(std::size_t tag) { m_device_tag = tag; }
    inline std::size_t getDeviceTag() const { return m_device_tag; }
    inline std::size_t getConnectionState() const { return m_pEndpoint->getConnectionState(); }

protected:
    using DeviceTList = typename get_devices_from_endpoint_typelist<EndpointTList>::type;
    using Def = DefinitionsContainer;

protected:
    inline std::size_t deviceTag() const { return m_device_tag; }
    inline std::unique_ptr<ITransportEndpoint> & endpoint() { return m_pEndpoint; }
    inline const std::unique_ptr<ITransportEndpoint> & endpoint() const { return m_pEndpoint; }

    template<typename IDerivedInterface>
    std::unique_ptr<IDerivedInterface> & getDownCastedEndpoint()
    {
        return static_cast<std::unique_ptr<IDerivedInterface>&>(m_pEndpoint);
    }

    template<template<typename, template<typename...> typename > typename MethodDispatcher, typename CB >
    //template<template<typename> typename MethodDispatcher, typename CB >
    bool registerCallback(CB && cb)
    {
        if (!m_pEndpoint) return false;
        std::cout << "DynaicTransportEndpointAdapter::registerCallback()\n";
        using return_t = typename MethodDispatcher<EndpointTList, type_to_device_tag>::return_variant;
        //using return_t = typename MethodDispatcher<EndpointTList>::return_variant;
        auto wrapper = meta::dispatch::wrap(m_pEndpoint->getInternalEndpoint());
        return_t ret;
        if (std::holds_alternative<bool>(ret)) {
            ret = false;
        }

        std::cout << "DynaicTransportEndpointAdapter::registerCallback() calling dispatcher\n";
        ret = MethodDispatcher<EndpointTList, type_to_device_tag>::call(m_device_tag, wrapper, std::forward<CB>(cb));
        //ret = MethodDispatcher<EndpointTList>::call(m_device_tag, wrapper, std::forward<CB>(cb));
        if (std::holds_alternative<bool>(ret)) {
            return std::get<bool>(ret);
        }
        return false;
    }

private:
    std::size_t m_device_tag;
    std::unique_ptr<ITransportEndpoint> m_pEndpoint;
};

template<typename EndpointTList>
class ClientDynamicTransportEndpointAdapter : public DynamicTransportEndpointAdapter<EndpointTList>
{
    using Base = DynamicTransportEndpointAdapter<EndpointTList>;
    using DeviceTList = typename Base::DeviceTList;
    using Def = ClientDefinitionsContainer;

public:
    ClientDynamicTransportEndpointAdapter() = default;

    ClientDynamicTransportEndpointAdapter(std::unique_ptr<IClientTransportEndpoint> p_endpoint, std::size_t device_tag) :
        Base{std::move(p_endpoint), device_tag}
    {}

public:
    bool init(std::unique_ptr<ITransportEndpoint> p_endpoint, std::size_t device_tag) override
    {
        if (!p_endpoint){
            return false;
        }

        if (IClientTransportEndpoint *p_clientEndpoint = dynamic_cast<IClientTransportEndpoint*>(p_endpoint.get());
           nullptr == p_clientEndpoint){
            return false;
        }
        return Base::init(std::move(p_endpoint), device_tag);
    }

    // dispatcher callbacks
    template<typename InputAvailableCB>
    bool registerInputCallback(InputAvailableCB && cb) {
        std::cout << "ClientDynamicTransportEndpointAdapter::registerInputCallback()\n";
        return Base::template registerCallback<Def::registerInputCallback_dispatcher_mapped>(std::forward<InputAvailableCB>(cb));
        //return Base::template registerCallback<Def::registerInputCallback_dispatcher>(std::forward<InputAvailableCB>(cb));
    }

    template<typename DisconnectedCB>
    bool registerDisconnectionCallback(DisconnectedCB && cb) {
        return Base::template registerCallback<Def::registerDisconnectionCallback_dispatcher_mapped>(std::forward<DisconnectedCB>(cb));
        //return Base::template registerCallback<Def::registerDisconnectionCallback_dispatcher>(std::forward<DisconnectedCB>(cb));
    }
    
    void onConnected()
    {
        if (!Base::endpoint()) return;
        std::unique_ptr<IClientTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IClientTransportEndpoint>&>(Base::endpoint());
        p_endpoint->onConnected();
    }

    bool connect(std::string_view addr, bool non_blocking)
    {
        if (!Base::endpoint()) return false;
        std::unique_ptr<IClientTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IClientTransportEndpoint>&>(Base::endpoint());
        return p_endpoint->connect(addr, non_blocking);
    }

    template<typename Address>
    decltype(auto) connect(Address && address)
    {
        using return_t = typename Def::connect_dispatcher_mapped<EndpointTList, type_to_device_tag>::return_connect_variant;
        //using return_t = typename Def::connect_dispatcher<EndpointTList>::return_connect_variant;
        return_t ret;
        if (Base::endpoint())
        {
            auto wrapper = meta::dispatch::wrap(Base::endpoint()->getInternalEndpoint());
            ret = Def::connect_dispatcher_mapped<EndpointTList, type_to_device_tag>::call(this->getDeviceTag(), wrapper, std::forward<Address>(address));
            //ret = Def::connect_dispatcher<EndpointTList>::call(this->getDeviceTag(), wrapper, std::forward<Address>(address));
        }
        return ret;
    }
    /*
    ssize_t send(std::string_view data)
    {
        if (!Base::endpoint()) return -1;
        std::unique_ptr<IClientTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IClientTransportEndpoint>&>(Base::endpoint());
        p_endpoint->send();
    }
    */

    void disconnect()
    {
        if (!Base::endpoint()) return;
        std::unique_ptr<IClientTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IClientTransportEndpoint>&>(Base::endpoint());
        p_endpoint->disconnect();
    }
};

template<typename EndpointTList>
class ServerDynamicTransportEndpointAdapter : public DynamicTransportEndpointAdapter<EndpointTList>
{
    using Base = DynamicTransportEndpointAdapter<EndpointTList>;
    using DeviceTList = typename Base::DeviceTList;
    using Def = ServerDefinitionsContainer;

public:
    ServerDynamicTransportEndpointAdapter() = default;

    ServerDynamicTransportEndpointAdapter(std::unique_ptr<IServerTransportEndpoint> p_endpoint, std::size_t device_tag) :
        Base{std::move(p_endpoint), device_tag}
    {}

public:
    bool init(std::unique_ptr<ITransportEndpoint> p_endpoint, std::size_t device_tag) override
    {
        if (!p_endpoint){
            return false;
        }

        if (IServerTransportEndpoint *p_clientEndpoint = dynamic_cast<IServerTransportEndpoint*>(p_endpoint.get());
           nullptr == p_clientEndpoint){
            return false;
        }
        return Base::init(std::move(p_endpoint), device_tag);
    }

    template<typename ClientInputAvailableCB>
    bool registerClientInputCallback(ClientInputAvailableCB && cb) {
        return Base::template registerCallback<Def::registerClientInputCallback_dispatcher_mapped>(std::forward<ClientInputAvailableCB>(cb));
        //return Base::template registerCallback<Def::registerClientInputCallback_dispatcher>(std::forward<ClientInputAvailableCB>(cb));
    }

    template<typename ClientConnectedCB>
    bool registerClientConnectionCallback(ClientConnectedCB && cb) {
        return Base::template registerCallback<Def::registerClientConnectionCallback_dispatcher_mapped>(std::forward<ClientConnectedCB>(cb));
        //return Base::template registerCallback<Def::registerClientConnectionCallback_dispatcher>(std::forward<ClientConnectedCB>(cb));
    }

    template<typename ClientDisconnectedCB>
    bool registerClientDisconnectionCallback(ClientDisconnectedCB && cb) {
        return Base::template registerCallback<Def::registerClientDisconnectionCallback_dispatcher_mapped>(std::forward<ClientDisconnectedCB>(cb));
        //return Base::template registerCallback<Def::registerClientDisconnectionCallback_dispatcher>(std::forward<ClientDisconnectedCB>(cb));
    }

    bool bind(std::string_view addr, bool reusable)
    {
        if (!Base::endpoint()) return false;
        std::unique_ptr<IServerTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IServerTransportEndpoint>&>(Base::endpoint());
        return p_endpoint->bind(addr, reusable);
    }

    template<typename Address>
    decltype(auto) bind(Address & address, bool reusable)
    {
        using return_t = typename Def::bind_dispatcher_mapped<EndpointTList, type_to_device_tag>::return_bind_variant;
        //using return_t = typename Def::bind_dispatcher<EndpointTList>::return_bind_variant;
        return_t ret;
        if (Base::endpoint())
        {
            auto wrapper = meta::dispatch::wrap(Base::endpoint()->getInternalEndpoint());
            ret = Def::bind_dispatcher_mapped<EndpointTList, type_to_device_tag>::call(this->getDeviceTag(), wrapper, std::forward<Address>(address), reusable);
            //ret = Def::bind_dispatcher<EndpointTList>::call(this->getDeviceTag(), wrapper, std::forward<Address>(address), reusable);
        }
        return ret;
    }

    bool listen(int backlog)
    {
        if (!Base::endpoint()) return false;
        std::unique_ptr<IServerTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IServerTransportEndpoint>&>(Base::endpoint());
        return p_endpoint->listen(backlog);
    }

    bool accept(bool non_blocking)
    {
        if (!Base::endpoint()) return false;
        std::unique_ptr<IServerTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IServerTransportEndpoint>&>(Base::endpoint());
        return p_endpoint->accept(non_blocking);
    }
};

}//infra
#endif
