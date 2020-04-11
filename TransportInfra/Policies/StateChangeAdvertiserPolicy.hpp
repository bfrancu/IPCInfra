#ifndef STATECHANGEADVERTISERPOLICY_HPP
#define STATECHANGEADVERTISERPOLICY_HPP

#include <future>

#include "crtp_base.hpp"
#include "Traits/socket_traits.hpp"
#include "ObservableStatePublisher.hpp"
#include "FileIODefinitions.h"

#define DEFINE_STATE_CHANGE_ADVERTISER_POLICY(State)                                                                                                                           \
class State##MemberAccess                                                                                                                                                      \
{                                                                                                                                                                              \
public:                                                                                                                                                                        \
    template<typename Device>                                                                                                                                                  \
    static decltype(auto) getMember(Device & device) {                                                                                                                         \
        return device.get##State();                                                                                                                                            \
    }                                                                                                                                                                          \
};                                                                                                                                                                             \
                                                                                                                                                                               \
template<typename Device>                                                                                                                                                      \
using Observed##State##Member = std::decay_t<std::invoke_result_t<decltype (&State##MemberAccess::getMember<Device>), std::reference_wrapper<Device>>>;                        \
                                                                                                                                                                               \
template<typename Device, typename = std::void_t<>>                                                                                                                            \
struct HasObservable##State##MemberT : std::false_type                                                                                                                         \
{};                                                                                                                                                                            \
                                                                                                                                                                               \
                                                                                                                                                                               \
template<typename Device>                                                                                                                                                      \
struct HasObservable##State##MemberT<Device,                                                                                                                                   \
                                std::void_t<std::enable_if_t<is_observable<std::invoke_result_t<decltype (&State##MemberAccess::getMember<Device>),                            \
                                                                                                std::reference_wrapper<Device>>>::value>>>                                     \
    : std::true_type                                                                                                                                                           \
{};                                                                                                                                                                            \
                                                                                                                                                                               \
template<typename Host, typename Device,                                                                                                                                       \
         typename = std::void_t<>>                                                                                                                                             \
class State##ChangeAdvertiserPolicy{};                                                                                                                                         \
                                                                                                                                                                               \
                                                                                                                                                                               \
template<typename Host, typename Device>                                                                                                                                       \
class State##ChangeAdvertiserPolicy<Host, Device, std::void_t<std::enable_if_t<HasObservable##State##MemberT<Device>::value>>>                                 \
                          : public infra::crtp_base<State##ChangeAdvertiserPolicy<Host, Device>, Host>                                                                        \
                                                                                                                                                                               \
{                                                                                                                                                                              \
    template<typename T, typename U>                                                                                                                                           \
    using Publisher = infra::ObservableStatePublisher<typename Observed##State##Member<U>::observed_type,                                                                      \
                                                      State##ChangeAdvertiserPolicy<T, U>>;                                                                                    \
                                                                                                                                                                               \
public:                                                                                                                                                                        \
    using callback = typename Publisher<Host, Device>::callable;                                                                                                               \
    using subscriber_id = typename Publisher<Host, Device>::subscriber_id;                                                                                                     \
                                                                                                                                                                               \
    public:                                                                                                                                                                    \
    State##ChangeAdvertiserPolicy() :                                                                                                                                          \
        On##State##Changed{m_publisher},                                                                                                                                       \
        m_publisher{State##MemberAccess::getMember(this->asDerived()), *this}                                                                                                  \
    {}                                                                                                                                                                         \
                                                                                                                                                                               \
    infra::PublisherDelegate<Publisher<Host, Device>> On##State##Changed;                                                                                                      \
                                                                                                                                                                               \
    subscriber_id on##State##ChangedSubscribe(callback cb){                                                                                                                    \
        return m_publisher.addObserver(cb);                                                                                                                                    \
    }                                                                                                                                                                          \
                                                                                                                                                                               \
    bool on##State##ChangedUnsubscribe(subscriber_id id){                                                                                                                      \
        return m_publisher.removeObserver(id);                                                                                                                                 \
    }                                                                                                                                                                          \
                                                                                                                                                                               \
private:                                                                                                                                                                       \
    template<typename State, typename Dispatcher>                                                                                                                              \
    friend class infra::ObservableStatePublisher;                                                                                                                              \
                                                                                                                                                                               \
    template <typename U, typename... Ts>                                                                                                                                      \
    std::future<typename std::result_of<U(Ts...)>::type>                                                                                                                       \
    dispatch(U && task, Ts&&... params)                                                                                                                                        \
    {                                                                                                                                                                          \
        return std::async(std::launch::async,                                                                                                                                  \
                          std::forward<U>(task),                                                                                                                               \
                          std::forward<Ts>(params)...);                                                                                                                        \
    }                                                                                                                                                                          \
                                                                                                                                                                               \
private:                                                                                                                                                                       \
    Publisher<Host, Device> m_publisher;                                                                                                                                       \
} //; intentionally skipped

#endif // STATECHANGEADVERTISERPOLICY_HPP
