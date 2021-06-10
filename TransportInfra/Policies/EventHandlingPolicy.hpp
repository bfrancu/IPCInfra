#ifndef EVENT_HANDLING_POLICY_H
#define EVENT_HANDLING_POLICY_H
#include <functional>
#include <optional>
#include <iostream>

#include "crtp_base.hpp"
#include "Reactor/EventTypes.h"

namespace infra
{

template<typename Derived, typename Listener>
class BaseEventHandlingPolicy
{
    using SubscriberID = typename Listener::SubscriberID;
    using Handle = typename Listener::Handle;

public:
    explicit BaseEventHandlingPolicy(Listener & listener) :
        m_listener{listener}
    {}

    EHandleEventResult handleEvent(SubscriberID id, EHandleEvent event)
    {
        std::cout << "BaseEventHandlingPolicy::handleEvent() id: " << id << " event " << static_cast<int>(event) << "\n";
        std::optional<bool> optional_process_res;
        if (m_listener_sub_id != id)
        {
            std::cout << "BaseEventHandlingPolicy::handleEvent() returning invalid reference\n";
            return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
        }

        switch(event)
        {
            case EHandleEvent::E_HANDLE_EVENT_ERR      : optional_process_res = this->asDerived().onErrorEvent(); break;
            case EHandleEvent::E_HANDLE_EVENT_HUP      : optional_process_res = this->asDerived().onHangupEvent(); break;
            case EHandleEvent::E_HANDLE_EVENT_IN       : optional_process_res = this->asDerived().onInputEvent(); break;
            case EHandleEvent::E_HANDLE_EVENT_OUT      : optional_process_res = this->asDerived().onWriteAvailable(); break;
            case EHandleEvent::E_HANDLE_EVENT_SHUTDOWN : optional_process_res = this->asDerived().onDisconnection(); break;
            default: break;
        }

        if (optional_process_res.has_value()){
            return optional_process_res.value() ? EHandleEventResult::E_RESULT_SUCCESS 
                                                : EHandleEventResult::E_RESULT_FAILURE;
        }

        std::cout << "BaseEventHandlingPolicy::handleEvent() return\n";
        return EHandleEventResult::E_RESULT_METHOD_NOT_IMPLEMENTED;
    }

    inline bool subscribedToListener() const { return Listener::NULL_SUBSCRIBER_ID != m_listener_sub_id; }

    bool listenerSubscribe(const events_array & events)
    {
        if (!m_optional_handle_ref.has_value())
        {
            std::cout << "BaseEventHandlingPolicy::listenerSubscribe() Device handle reference not set\n";
            return false;
        }

        auto handle = this->asDerived().getHandle();

        if (auto id = m_listener.subscribe(events, handle, *this);
            Listener::NULL_SUBSCRIBER_ID != id)
        {
            m_listener_sub_id = id;
            return true;
        }
        std::cout << "BaseEventHandlingPolicy::listenerSubscribe() returning false \n";

        return false;
    }

    bool listenerUnsubscribe()
    {
        std::cout << "BaseventHandlingPolicy::listenerUnsubscribe()\n";
        if (m_listener.unsubscribe(m_listener_sub_id))
        {
            m_listener_sub_id = Listener::NULL_SUBSCRIBER_ID;
            return true;
        }
        return false;
    }

protected:
    ~BaseEventHandlingPolicy()
    {
        if (subscribedToListener()) {
            listenerUnsubscribe();
        }
    }

    void setHandle(Handle handle)
    {
        m_optional_handle_ref = handle;
    }

    void resetHandle()
    {
        m_optional_handle_ref.reset();
    }

    Listener & getListener() { return m_listener; }
    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }

private:
    Listener & m_listener;
    SubscriberID m_listener_sub_id{Listener::NULL_SUBSCRIBER_ID};
    std::optional<Handle> m_optional_handle_ref;
  
};

template<typename Host, typename Derived,
         typename Enable = void>
class GenericEventHandlingPolicy;

template<typename Host, typename Derived>
class GenericEventHandlingPolicy<Host, Derived, std::void_t<typename Derived::Listener, 
                                                            decltype(std::declval<Derived>().getListener())>> 
        : public crtp_base<GenericEventHandlingPolicy<Host, Derived>, Host>
{
    using Listener = typename Derived::Listener;
    using SubscriberID = typename Listener::SubscriberID;
    using Handle = typename Listener::Handle;

public:
    bool init()
    {
        if (m_initDone)
        {
            return true;
        }
        m_listener = std::ref(this->asDerived().getListener());
        m_initDone = true;
        return true;
    }

    void deinit()
    {
        if (subscribedToListener()){
            listenerUnsubscribe();
        }
    }

    EHandleEventResult handleEvent(SubscriberID id, EHandleEvent event)
    {
        (void) event;
        if (m_listener_sub_id != id)
        {
            return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
        }
        return EHandleEventResult::E_RESULT_DEFAULT;
    }

    inline bool subscribedToListener() const { return Listener::NULL_SUBSCRIBER_ID != m_listener_sub_id; }

    bool listenerSubscribe(const events_array & events)
    {
        if (!m_initDone) return false;
        if (!m_optional_handle_ref.has_value())
        {
            return false;
        }

        if (auto id = m_listener.get().subscribe(events, m_optional_handle_ref.value(), *this);
            Listener::NULL_SUBSCRIBER_ID != id)
        {
            m_listener_sub_id = id;
            return true;
        }
        return false;
    }

    bool listenerUnsubscribe()
    {
        if (!m_initDone) return false;
        if (m_listener.get().unsubscribe(m_listener_sub_id))
        {
            m_listener_sub_id = Listener::NULL_SUBSCRIBER_ID;
            return true;
        }
        return false;
    }

protected:
    ~GenericEventHandlingPolicy() { deinit(); }

    void setHandle(Handle handle)
    {
        m_optional_handle_ref = handle;
    }

    void resetHandle()
    {
        m_optional_handle_ref.reset();
    }
    
protected:
    bool m_initDone{false};
    SubscriberID m_listener_sub_id{Listener::NULL_SUBSCRIBER_ID};
    std::reference_wrapper<Listener>  m_listener;
    std::optional<Handle> m_optional_handle_ref;
};

}//infra

#endif
