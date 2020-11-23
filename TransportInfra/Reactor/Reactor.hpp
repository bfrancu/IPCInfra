#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <thread>
#include <mutex>
#include <algorithm>
//#include <unordered_map>

#include "SubscriberInfo.hpp"
#include "shared_lookup_table.hpp"
#include "shared_queue.hpp"

namespace infra
{

template<typename handle_t, typename DemultiplexPolicy>
class Reactor
{

    using SubscriberInfoT = SubscriberInfo<handle_t>;
    using DemuxTable      = shared_lookup_table<subscriber_id, SubscriberInfoT, std::unordered_map>;

public:
    Reactor() :
        m_demux_wait_thread{},
        m_event_processing_thread{},
        m_mutex{},
        m_subscribers_table{},
        m_event_queue{},
        m_demux_impl{m_event_queue},
        m_processing_ended{false},
        m_reactor_running{false}
    {}

    ~Reactor(){
        std::shared_lock lck{m_mutex, std::defer_lock};

        m_subscribers_table.for_each([this](const auto & pair){
            if (pair.second.registered_to_monitor) m_demux_impl.unregisterListener(pair.second);
        });

        lck.lock();
        if (m_reactor_running){
            m_mutex.unlock_shared();
            stop();
        }

        if (lck.owns_lock()) m_mutex.unlock_shared();
        clear();
    }

    Reactor(Reactor && other) :
        m_demux_wait_thread{},
        m_event_processing_thread{},
        m_mutex{},
        m_subscribers_table{},
        m_event_queue{},
        m_demux_impl{m_event_queue},
        m_processing_ended{false},
        m_reactor_running{false}
    {
        std::unique_lock other_lck{other.m_mutex};
        m_demux_wait_thread = std::move(other.m_demux_wait_thread);
        m_event_processing_thread = std::move(other.m_event_processing_thread);
        m_subscribers_table = std::move(other.m_subscribers_table);
        m_event_queue = std::move(other.m_event_queue);
        m_demux_impl = std::move(other.m_demux_impl);
    }

    Reactor & operator=(Reactor && other)
    {
        if (this == &other) return *this;

        std::lock(m_mutex, other.m_mutex);
        m_demux_wait_thread = std::move(other.m_demux_wait_thread);
        m_event_processing_thread = std::move(other.m_event_processing_thread);
        m_subscribers_table = std::move(other.m_subscribers_table);
        m_event_queue = std::move(other.m_event_queue);
        m_demux_impl = std::move(other.m_demux_impl);
        return *this;
    }

    Reactor(const Reactor &) = delete;
    Reactor & operator=(const Reactor &) = delete;

public:
    //void init();
    void start(){
        std::unique_lock lck{m_mutex};
        if (m_reactor_running) return;

        lck.unlock();
        if (!m_event_processing_thread.joinable()){
            m_event_processing_thread = std::thread(&Reactor<handle_t, DemultiplexPolicy>::eventProcessingThread, this);
        }

        if (!m_demux_wait_thread.joinable()){
            m_demux_wait_thread = std::thread(&DemultiplexPolicy::monitorWaitThread, &m_demux_impl);
        }

        lck.lock();
        m_reactor_running = true;
    }

    void stop(){
        std::unique_lock lck{m_mutex, std::defer_lock};
        if (m_demux_impl.monitoring()){
            m_demux_impl.stopMonitoring();
        }

        lck.lock();

        if (!m_reactor_running) return;
        m_processing_ended = true;

        lck.unlock();
        EventNotification<handle_t> last_event_notification{default_value<handle_t>::value, 0, true};
        m_event_queue.push(std::move(last_event_notification));

        if (m_demux_wait_thread.joinable()){
            m_demux_wait_thread.join();
        }

        if (m_event_processing_thread.joinable()){
            m_event_processing_thread.join();
        }

        lck.lock();
        m_reactor_running = false;
    }

    bool clear(){
        std::shared_lock lck{m_mutex};
        if (m_reactor_running) return false;
        m_subscribers_table.clear();
        return true;
    }

    template<typename EventHandler>
    subscriber_id subscribe(const events_array & events, handle_t handle, EventHandler & ev_handler){
        ConcreteEventHandler<EventHandler> handler{&ev_handler};
        //handle_t handle{ev_handler.getHandle()};
        return subscribeImpl(events, handler, handle);
    }

    bool unsubscribe(subscriber_id id){
        m_subscribers_table.remove_if([](const auto & pair){
            return pair.second.expired || !pair.second.registered_to_monitor;});

        if (auto [sub_info, res_found] = m_subscribers_table.value_for(id);
            res_found){
            if (sub_info.registered_to_monitor){
                m_demux_impl.unregisterListener(sub_info);
            }

            m_subscribers_table.remove_mapping(id);
            return true;
        }

        return false;
    }

protected:
    subscriber_id subscribeImpl(const events_array & events, AbstractEventHandler & handler, handle_t handle){
        uint32_t subscription_mask = m_demux_impl.getEventsMask(events);
        bool find_res{false};
        bool already_subscribed{false};
        subscriber_id sub_id{NULL_SUBSCRIBER_ID};

        do{
            sub_id = m_demux_impl.getKeyfor(handle);
            auto [sub_info, res] = m_subscribers_table.value_for(sub_id);
            find_res = res;
            if (find_res && handle == sub_info.handle){
                already_subscribed = true;
                break;
            }
        }
        while (find_res);

        SubscriberInfoT sub_info{subscription_mask, sub_id, handler};
        if(!m_subscribers_table.add_or_update_mapping(sub_id, sub_info)){
            return NULL_SUBSCRIBER_ID;
        }

        if (already_subscribed && m_demux_impl.updateListener(sub_info)){
            return sub_id;
        }
        else
        {   if (m_demux_impl.registerListener(sub_info)){
                sub_info.registered_to_monitor = true;
                return sub_id;
            }
            else m_subscribers_table.remove_mapping(sub_id);
        }
        return NULL_SUBSCRIBER_ID;
    }

    void eventProcessingThread()
    {
        for (;;)
        {
            EventNotification<handle_t> event_notification;
            m_event_queue.wait_and_pop(event_notification);
            subscriber_id sub_id = m_demux_impl.getKeyfor(event_notification.handle);
            auto [sub_info, value_found] = m_subscribers_table.value_for(sub_id);
            if (value_found){
                 events_array events = m_demux_impl.getEventsFromMask(event_notification.notified_events_mask);

                 for (auto event : events){
                     EHandleEventResult res = sub_info.event_handler.handleEvent(event);
                     if (EHandleEventResult::E_RESULT_INVALID_REFERENCE == res){
                         sub_info.expired = true;
                         break;
                     }
                 }
            }

            std::shared_lock lck{m_mutex};
            if (event_notification.last_event || m_processing_ended) return;
        }
    }

private:
    std::thread m_demux_wait_thread;
    std::thread m_event_processing_thread;

    mutable std::shared_mutex m_mutex;
    DemuxTable m_subscribers_table;
    shared_queue<EventNotification<handle_t>> m_event_queue;
    DemultiplexPolicy m_demux_impl;
    bool m_processing_ended;
    bool m_reactor_running;
};

} //infra

#endif // REACTOR_HPP


