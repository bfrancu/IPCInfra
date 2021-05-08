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
    DEFINE_DISPATCH_TO_MEMBER(connect);
    DEFINE_DISPATCH_TO_MEMBER(disconnect);
    DEFINE_DISPATCH_TO_MEMBER(registerInputCallback);
    DEFINE_DISPATCH_TO_MEMBER(registerDisconnectionCallback);

    template<typename> friend class DynamicTransportEndpointAdapter;
};

class ClientDefinitionsContainer
{
    DEFINE_DISPATCH_TO_MEMBER(onConnected);
    template<typename> friend class ClientDynamicTransportEndpointAdapter;
};

template<typename EndpointTList>
class DynamicTransportEndpointAdapter
{
    using DeviceTList = typename get_devices_from_endpoint_typelist<EndpointTList>::type;
    using Def = DefinitionsContainer;

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

    // dispatcher callbacks
    template<typename InputAvailableCB>
    bool registerInputCallback(InputAvailableCB cb) {
        if (!m_pEndpoint) return false;
        using return_t = typename Def::registerInputCallback_dispatcher<EndpointTList>::return_registerInputCallback_variant;
        auto wrapper = meta::dispatch::wrap(m_pEndpoint->getInternalEndpoint());
        return_t ret;
        if (std::holds_alternative<bool>(ret)) {
            ret = false;
        }

        ret = Def::registerInputCallback_dispatcher<EndpointTList>::call(m_device_tag, wrapper,
                                                                         std::forward<InputAvailableCB>(cb));
        if (std::holds_alternative<bool>(ret)) {
            return std::get<bool>(ret);
        }
        return false;
    }

    template<typename DisconnectedCB>
    bool registerDisconnectionCallback(DisconnectedCB && cb) {
        if (!m_pEndpoint) return false;
        using return_t = typename Def::registerDisconnectionCallback_dispatcher<EndpointTList>::return_registerDisconnectionCallback_variant;;
        auto wrapper = meta::dispatch::wrap(m_pEndpoint->getInternalEndpoint());
        return_t ret;
        if (std::holds_alternative<bool>(ret)) {
            ret = false;
        }

        ret = Def::registerDisconnectionCallback_dispatcher<EndpointTList>::call(m_device_tag, wrapper,
                                                                                 std::forward<DisconnectedCB>(cb));
        if (std::holds_alternative<bool>(ret)) {
            return std::get<bool>(ret);
        }
        return false;
    }

    // device interface wrappers
    bool connect(std::string_view address)
    {
        (void) address;
        return true;
    }

    template<typename Address>
    decltype(auto) connect(Address && address)
    {
        using return_t = typename Def::connect_dispatcher<DeviceTList>::return_connect_variant;
        return_t ret;
        if (m_pEndpoint)
        {
            auto wrapper = meta::dispatch::wrap(m_pEndpoint->getDevice());
            ret = Def::connect_dispatcher<DeviceTList>::call(m_device_tag, wrapper, std::forward<Address>(address));
        }
        return ret;
    }

    void disconnect()
    {
        if (!m_pEndpoint) return;
        auto wrapper = meta::dispatch::wrap(m_pEndpoint->getDevice());
        Def::disconnect_dispatcher<DeviceTList>::call(m_device_tag, wrapper);
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
    inline std::size_t deviceTag() const { return m_device_tag; }
    inline std::unique_ptr<ITransportEndpoint> & endpoint() { return m_pEndpoint; }
    inline const std::unique_ptr<ITransportEndpoint> & endpoint() const { return m_pEndpoint; }

private:
    std::size_t m_device_tag;
    std::unique_ptr<ITransportEndpoint> m_pEndpoint;
};

template<typename EndpointTList>
class ClientDynamicTransportEndpointAdapter : public DynamicTransportEndpointAdapter<EndpointTList>
{
    using Base = DynamicTransportEndpointAdapter<EndpointTList>;
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

    void onConnected()
    {
        if (!Base::endpoint()) return;
        std::unique_ptr<IClientTransportEndpoint> & p_endpoint = static_cast<std::unique_ptr<IClientTransportEndpoint>&>(Base::endpoint());
        auto wrapper = meta::dispatch::wrap(Base::endpoint()->getInternalEndpoint());

        Def::onConnected_dispatcher<EndpointTList>::call(Base::deviceTag(), wrapper);
    }

};
}//infra
#endif
