#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <thread>
#include <mutex>
#include <algorithm>
#include <unordered_map>

#include "SubscriberInfo.hpp"

namespace infra
{

template<typename handle_type>
using DemuxTable = std::unordered_map<subscriber_id, SubscriberInfo<handle_type>>;

//template<typename handle_type>
//using SubscriberInfoT = SubscriberInfo<handle_type>;

template<typename handle_type, typename DemultiplexPolicy>
class Reactor
{

    using SubscriberInfoT = SubscriberInfo<handle_type>;

public:
    Reactor(){}
    ~Reactor(){}  

public:
    void init();

public:
    template<typename EventHandler>
    subscriber_id subscribe(const events_array & events, EventHandler & ev_handler){
        ConcreteEventHandler<EventHandler> handler{ev_handler};
        handle_type handle{ev_handler.getHandle()};
        return subscribeImpl(events, handler, handle);
    }

    bool unsubscribe(subscriber_id id){
        std::lock_guard<std::mutex> lock{m_mutex};

        for (auto it = m_subscribers_table.begin(); it != m_subscribers_table.end(); ++it){
            if (it->expired || !it->registered_to_monitor) m_subscribers_table.erase(it);
        }

        if (auto it = m_subscribers_table.find(id);
           (m_subscribers_table.end() != it)){
            const SubscriberInfoT & sub_info = *it;
            if (sub_info.registered_to_monitor) {
                m_demux_impl.unregisterListener(*it);
            }

            m_subscribers_table.erase(it);
            return true;
        }

        return false;
    }

protected:
    subscriber_id subscribeImpl(const events_array & events, AbstractEventHandler & handler, handle_type handle){
        uint32_t subscription_mask = m_demux_impl.getEventsMask(events);
        subscriber_id sub_id = m_demux_impl.getHashedKey(handle);

        std::lock_guard<std::mutex> lock{m_mutex};
        auto [success, it] = m_subscribers_table.insert_or_assign(sub_id, SubscriberInfoT(subscription_mask, sub_id, handler));
        if (!success) return NULL_SUBSCRIBER_ID;

        const SubscriberInfoT & sub_info = *it;
        if (m_demux_impl.registerListener(sub_info)){
            sub_info.registered_to_monitor = true;
            return sub_id;
        }

        return NULL_SUBSCRIBER_ID;
    }

private:
    std::thread m_demux_wait_thread;
    mutable std::mutex m_mutex;
    DemultiplexPolicy m_demux_impl;
    DemuxTable<handle_type> m_subscribers_table;
};

} //infra

#endif // REACTOR_HPP
