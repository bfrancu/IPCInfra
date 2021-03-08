#ifndef EVENT_HANDLING_POLICY_H
#define EVENT_HANDLING_POLICY_H
#include <functional>
#include <optional>

#include "Reactor/EventTypes.h"
#include "Devices/GenericDeviceAccess.hpp"

namespace infra
{

template<typename Listener, typename Enable = void>
class GenericEventHandlingPolicy
{
    using SubscriberID = typename Listener::SubscriberID;
    using Handle = typename Listener::Handle;

public:
    explicit GenericEventHandlingPolicy(Listener & listener) :
        m_listener{listener}
    {}

    EHandleEventResult handleEvent(SubscriberID id, EHandleEvent event)
    {
        (void) event;
        if (m_listener_sub_id != id)
        {
            return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
        }
        return EHandleEventResult::E_RESULT_DEFAULT;
    }

    inline Listener & getListener() const { return m_listener; }
    inline void resetListener(Listener & listener) { m_listener = listener; }
    inline bool subscribedToListener() const { return Listener::NULL_SUBSCRIBER_ID != m_listener_sub_id; }

    bool listenerSubscribe(const events_array & events)
    {
        if (!m_optional_handle_ref.has_value())
        {
            return false;
        }

        if (auto id = m_listener.subscribe(events, m_optional_handle_ref.value(), *this);
            Listener::NULL_SUBSCRIBER_ID != id)
        {
            m_listener_sub_id = id;
            return true;
        }
        return false;
    }

    bool listenerUnsubscribe()
    {
        if (m_listener.unsubscribe(m_listener_sub_id))
        {
            m_listener_sub_id = Listener::NULL_SUBSCRIBER_ID;
            return true;
        }
        return false;
    }

protected:
    void setHandle(Handle handle)
    {
        m_optional_handle_ref = handle;
    }

    void resetHandle()
    {
        m_optional_handle_ref.reset();
    }
    
protected:
    Listener & m_listener;
    SubscriberID m_listener_sub_id{Listener::NULL_SUBSCRIBER_ID};
    std::optional<Handle> m_optional_handle_ref;
};

}//infra

#endif
