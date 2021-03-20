#ifndef CONNSTATE_CHANGE_ADVERTISER_HPP
#define CONNSTATE_CHANGE_ADVERTISER_HPP

#include "traits_utils.hpp"
#include "ObservableStatePublisher.hpp"
#include <utility>

namespace infra
{

DEFINE_HAS_MEMBER(getConnectionState);

template<typename T, typename = traits::select_if<has_member_getConnectionState<T>,
                                                  std::true_type, std::false_type>>
struct HasObservableConnectionState;

template<typename T>
struct HasObservableConnectionState<T, std::false_type> : std::false_type
{};

template<typename T>
struct HasObservableConnectionState<T, std::true_type> : is_observable<decltype(std::declval<T>().getConnectionState())>
{};

struct SerialCallbackDispatcher
{
    template<typename Callable, typename... Args>
    decltype(auto) dispatch(Callable && cb, Args&&... args) 
    {
        return std::forward<Callable>(cb)(std::forward<Args>(args)...);
    }
};

template<typename Endpoint, typename Dispatcher = SerialCallbackDispatcher, typename = void>
class ConnectionStateAdvertiser{};


template<typename Endpoint, typename Dispatcher>
class ConnectionStateAdvertiser<Endpoint, Dispatcher, std::enable_if_t<HasObservableConnectionState<Endpoint>::value>>
{
    using ObservableConnectionState = decltype(std::declval<Endpoint>().getConnectionState());
    using Publisher = ObservableStatePublisher<typename ObservableConnectionState::observed_type, Dispatcher>;
    using SubscriberId = typename Publisher::subscriber_id;
    using Callback = typename Publisher::callable;

private:
    Dispatcher m_dispatcher;
    Publisher m_publisher;

public:
    ConnectionStateAdvertiser() :
        m_publisher{static_cast<const Endpoint&>(*this).getConnectionState(), m_dispatcher},
        OnConnectionStateChanged{m_publisher}
    {}

    inline SubscriberId onConnectionStateChangedSubscribe(Callback cb) { return m_publisher.addObserver(std::move(cb)); }
    inline bool onConnectionStateChangedUnsubscribe(SubscriberId id) { return m_publisher.removeObserver(id); }

    PublisherDelegate<Publisher> OnConnectionStateChanged;
};
}//infra

#endif
