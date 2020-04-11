#ifndef STATECHANGEADVERTISERPOLICY_H
#define STATECHANGEADVERTISERPOLICY_H

#include <future>

#include "crtp_base.hpp"
#include "Traits/socket_traits.hpp"
#include "ObservableStatePublisher.hpp"
#include "FileIODefinitions.h"


namespace infra
{

class ErrorMemberAccess
{
public:
    template<typename Device>
    static decltype(auto) getMember(Device & device) {
        return device.getError();
    }
};

template<typename Device>
using ObservedErrorMember = std::decay_t<std::invoke_result_t<decltype (&ErrorMemberAccess::getMember<Device>), std::reference_wrapper<Device>>>;

template<typename Device, typename = std::void_t<>>
struct HasObservableErrorMemberT : std::false_type
{};


template<typename Device>
struct HasObservableErrorMemberT<Device,
                                std::void_t<std::enable_if_t<is_observable<std::invoke_result_t<decltype (&ErrorMemberAccess::getMember<Device>), std::reference_wrapper<Device>>>::value
                                                            >>>
    : std::true_type
{};

template<typename Host, typename Device,
         typename = std::void_t<>>
class ErrorChangeAdvertiserPolicy{};

template<typename Host, typename Device>
class ErrorChangeAdvertiserPolicy<Host, Device, std::void_t<std::enable_if_t<HasObservableErrorMemberT<Device>::value>>>
                          : public crtp_base<ErrorChangeAdvertiserPolicy<Host, Device>, Host>
{
    template<typename T, typename U>
    using Publisher = ObservableStatePublisher<typename ObservedErrorMember<U>::observed_type,
                                               ErrorChangeAdvertiserPolicy<T, U>>;

public:
    using callback = typename Publisher<Host, Device>::callable;
    using subscriber_id = typename Publisher<Host, Device>::subscriber_id;

    public:
    ErrorChangeAdvertiserPolicy() :
        OnErrorChanged{m_publisher},
        m_publisher{ErrorMemberAccess::getMember(this->asDerived()), *this}
    {}


    PublisherDelegate<Publisher<Host, Device>> OnErrorChanged;


    subscriber_id onErrorChangedSubscribe(callback cb){
        return m_publisher.addObserver(cb);
    }

    bool onErrorChangedUnsubscribe(subscriber_id id){
        return m_publisher.removeObserver(id);
    }

private:
    template<typename State, typename Dispatcher>
    friend class ObservableStatePublisher;

    template <typename U, typename... Ts>
    std::future<typename std::result_of<U(Ts...)>::type>
    dispatch(U && task, Ts&&... params)
    {
        return std::async(std::launch::async,
                          std::forward<U>(task),
                          std::forward<Ts>(params)...);
    }


private:
    Publisher<Host, Device> m_publisher;

};


}
#endif // STATECHANGEADVERTISERPOLICY_H
