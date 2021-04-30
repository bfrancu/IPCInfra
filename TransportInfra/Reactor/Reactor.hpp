#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <signal.h>

#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>
//#include <unordered_map>

#include "default_traits.hpp"
#include "SubscriberInfo.hpp"
#include "shared_lookup_table.hpp"
#include "shared_queue.hpp"

static void sigHandler(int signal)
{
    if (SIGINT == signal)
    {
        std::cout << "caught SIGINT\n";
    }
    else if (SIGQUIT == signal)
    {
        std::cout << "caught SIGQUIT\n";
    }
    else if (SIGKILL == signal)
    {
        std::cout << "caught SIGKILL\n";
    }
    else if (SIGSEGV == signal)
    {
        std::cout << "\n\n\nSegmentation fault ocurred thread " << std::this_thread::get_id() << "\n\n";
        exit(-1);
    }

}

namespace infra
{

template<typename handle_t, typename DemultiplexPolicy>
class Reactor
{
public:
    using SubscriberContextT = SubscriberContext<handle_t>;
    using SubscriberID       = subscriber_id;
    using DemuxTable         = shared_lookup_table<SubscriberID, SubscriberContextT, std::unordered_map>;
    using HandlersTable      = std::unordered_map<SubscriberID, EventHandlerWrapper>;
    using HandlersTableIter  = typename HandlersTable::iterator;
    using Handle             = handle_t;

    static constexpr SubscriberID NULL_SUBSCRIBER_ID{0};

public:
    Reactor() :
        m_demux_wait_thread{},
        m_event_processing_thread{},
        m_mutex{},
        m_subscribers_table{},
        m_handlers_table{},
        m_event_queue{},
        m_demux_impl{m_event_queue},
        //m_processing_ended{false},
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
        m_handlers_table{},
        m_event_queue{},
        m_demux_impl{m_event_queue},
        //m_processing_ended{false},
        m_reactor_running{false}
    {
        std::unique_lock other_lck{other.m_mutex};
        m_demux_wait_thread = std::move(other.m_demux_wait_thread);
        m_event_processing_thread = std::move(other.m_event_processing_thread);
        m_subscribers_table = std::move(other.m_subscribers_table);
        m_handlers_table = std::move(other.m_handlers_table);
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
        m_handlers_table = std::move(other.m_handlers_table);
        m_event_queue = std::move(other.m_event_queue);
        m_demux_impl = std::move(other.m_demux_impl);
        return *this;
    }

    Reactor(const Reactor &) = delete;
    Reactor & operator=(const Reactor &) = delete;

public:
    //void init();
    void start(){
        std::cout << "Reactor::start()\n";
        std::unique_lock lck{m_mutex};
        if (m_reactor_running) return;

        /*
        if (m_processing_ended){
            m_processing_ended = false;
        }
        */

        m_cleanup_ts = std::chrono::system_clock::now();
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
        std::cout << "Reactor::stop()\n";
        std::unique_lock lck{m_mutex, std::defer_lock};
        if (m_demux_impl.monitoring()){
            m_demux_impl.stopMonitoring();
        }

        lck.lock();

        if (!m_reactor_running) return;
        //m_processing_ended = true;

        lck.unlock();
        EventNotification<handle_t> last_event_notification{meta::traits::default_value<handle_t>::value, 0, true};
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
    SubscriberID subscribe(const events_array & events, handle_t handle, EventHandler & ev_handler){
        std::cout << "Reactor::subscribe()\n";
        SubscriberID ret_sub_id = subscribeImpl(events, handle);
        if (NULL_SUBSCRIBER_ID != ret_sub_id)
        {
            addHandlerInTable(ret_sub_id, ev_handler);
        }
        return ret_sub_id;
    }

    bool unsubscribe(SubscriberID id){
        std::cout << "Reactor::unsubscribe() id: " << id << "\n";

        cleanupExpired();

        if (auto [sub_info, res_found] = m_subscribers_table.value_for(id); res_found){
            /*
            std::cout << "Reactor::unsubscribe() id: " << id << " expired: " << sub_info.expired
                      << " registered: " << sub_info.registered_to_monitor << "\n";
            */
            if (sub_info.registered_to_monitor){
                m_demux_impl.unregisterListener(sub_info);
            }

            m_subscribers_table.remove_mapping(id);
            removeHandler(id);
            return true;
        }
        else
        {
            std::cout << "Reactor::unsubscribe() subscribe not found by id: " << id << "\n";
        }

        return false;
    }


    /*
    void testHandlers()
    {
        std::cout << "Reactor::testHandlers() thread " << std::this_thread::get_id() << "\n";
        m_subscribers_table.for_each([] (auto & pair) {
            pair.second.event_handler.handleEvent(pair.first, EHandleEvent::E_HANDLE_EVENT_IN); }
                                     );
    }
    */

protected:
    SubscriberID subscribeImpl(const events_array & events, handle_t handle){
        std::cout << "Reactor::subscribeImpl()\n";
        uint32_t subscription_mask = m_demux_impl.getEventsMask(events);
        bool find_res{false};
        bool already_subscribed{false};
        SubscriberID sub_id{NULL_SUBSCRIBER_ID};

        cleanupExpired();

        do{
            sub_id = m_demux_impl.getKeyFor(handle);
            std::cout << "Reactor::subscribeImpl() key for handle " << handle << " is: " << sub_id << "\n";

            auto [sub_info, res] = m_subscribers_table.value_for(sub_id);
            find_res = res;
            if (find_res && handle == sub_info.handle){
                //std::cout << "Reactor::subscribeImpl() already subscribed with sub id " << sub_id << "\n";
                already_subscribed = true;
                break;
            }
        }
        while (find_res);

        //std::cout << "Reactor::subscribeImpl() res found: " << sub_id << "\n";
        //std::cout << "Reactor::subscribeImpl() mapping added for sub id: " << sub_id << "\n";

        if (already_subscribed) {
            std::lock_guard lck{m_mutex};
            if(auto it = m_handlers_table.find(sub_id); std::end(m_handlers_table) != it) {
                if (it->second.in_progress) return NULL_SUBSCRIBER_ID;
            }
        }

        SubscriberContextT sub_info(subscription_mask, sub_id, handle);
        if(!m_subscribers_table.add_or_update_mapping(sub_id, sub_info)){
            std::cout << "Reactor::subscribeImpl() couldn't add mapping\n";
            return NULL_SUBSCRIBER_ID;
        }
        
        if (already_subscribed) {
            return m_demux_impl.updateListener(sub_info) ? sub_id : NULL_SUBSCRIBER_ID;
        }

        if (m_demux_impl.registerListener(sub_info)){
            std::cout << "Reactor::subscribeImpl() demultiplexer registered listener\n";
            sub_info.registered_to_monitor = true;
            m_subscribers_table.add_or_update_mapping(sub_id, sub_info);
            return sub_id;
        }

        m_subscribers_table.remove_mapping(sub_id);

        std::cout << "Reactor::subscribeImpl() returning null subscriber id\n";
        return NULL_SUBSCRIBER_ID;
    }

    template<typename EventHandler>
    void addHandlerInTable(SubscriberID id, EventHandler & ev_handler)
    {
        std::lock_guard lck{m_mutex};
        m_handlers_table[id] = EventHandlerWrapper(&ev_handler);
    }

    bool removeHandler(SubscriberID id)
    {
        std::lock_guard lck{m_mutex};
        if (auto it = m_handlers_table.find(id); std::end(m_handlers_table) != it)
        {
            if (!it->second.in_progress)
            {
                m_handlers_table.erase(it);
                return true;
            }
        }
        return false;
    }

    bool setHandlerInProgress(SubscriberID id, bool inProgress)
    {
        std::lock_guard lck{m_mutex};
        if (auto it = m_handlers_table.find(id); std::end(m_handlers_table) != it)
        {
            it->second.in_progress.store(inProgress);
            return true;
        }
        return false;
    }

    void cleanupExpired()
    {
        std::cout << "Reactor::cleanupExpired() cleaning up exired subscribers\n";
        m_subscribers_table.remove_if([this](const auto & pair){
            if (pair.second.expired || !pair.second.registered_to_monitor){
                return removeHandler(pair.first);
            }
            return false;
        });
        m_cleanup_ts = std::chrono::system_clock::now();
    }

    void eventProcessingThread()
    {
        std::cout << "Reactor::eventProcessingThread() thread started\n";
        signal(SIGQUIT, sigHandler);
        signal(SIGINT, sigHandler);
        signal(SIGKILL, sigHandler);
        signal(SIGSEGV, sigHandler);

        for (;;)
        {
            EventNotification<handle_t> event_notification;
            m_event_queue.wait_and_pop(event_notification);

            if(event_notification.last_event){
                std::cout << "Reactor::eventProcessingThread() last event incoming thread stopped\n";
                return;
            }

            SubscriberID sub_id = m_demux_impl.getKeyFor(event_notification.handle);
            std::cout << "Reactor::eventProcessingThread() new event incoming; subscriber id: " << sub_id  << "\n"; //<< " thread " << std::this_thread::get_id() << "\n";
            auto [sub_info, value_found] = m_subscribers_table.value_for(sub_id);
            if (value_found && !sub_info.expired){
                if (!setHandlerInProgress(sub_id, true)){
                    sub_info.expired = true;
                    m_subscribers_table.add_or_update_mapping(sub_id, sub_info);
                    continue;
                }
                std::cout << "Reactor::eventProcessingThread() subscriber id found\n";
                events_array events = m_demux_impl.getEventsFromMask(event_notification.notified_events_mask);

                for (auto event : events){
                     if (EHandleEvent::E_HANDLE_EVENT_NULL == event) continue;

                     std::cout << "Reactor::eventProcessingThread() handling event: " << static_cast<int>(event) << "\n";
                     EHandleEventResult res = m_handlers_table[sub_id].handleEvent(sub_id, event);
                     //EHandleEventResult res = EHandleEventResult::E_RESULT_METHOD_NOT_IMPLEMENTED;
                     std::cout << "Reactor::eventProcessingThread() event handled\n";

                     if (EHandleEventResult::E_RESULT_INVALID_REFERENCE == res){
                         std::cout << "Reactor::eventProcessingThread() sub id " << sub_id << "expired\n";
                         sub_info.expired = true;
                         m_subscribers_table.add_or_update_mapping(sub_id, sub_info);
                         break;
                     }
                 }

                m_handlers_table[sub_id].in_progress.store(false);
            }
            else
            {
                std::cout << "Reactor::eventProcessingThread() subscriber not found by id\n";
            }

            if (CLEANUP_INTERVAL_S <= elapsedFrom(m_cleanup_ts)){
                cleanupExpired();
            }
        }
    }

    std::size_t elapsedFrom(const std::chrono::time_point<std::chrono::system_clock> & ts)
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - ts).count();
    }
    
private:
    static constexpr std::size_t CLEANUP_INTERVAL_S{300};

private:
    std::thread m_demux_wait_thread;
    std::thread m_event_processing_thread;

    mutable std::shared_mutex m_mutex;
    DemuxTable m_subscribers_table;
    HandlersTable m_handlers_table;
    shared_queue<EventNotification<handle_t>> m_event_queue;
    DemultiplexPolicy m_demux_impl;
    //bool m_processing_ended;
    bool m_reactor_running;
    std::chrono::time_point<std::chrono::system_clock> m_cleanup_ts;
};

} //infra

#endif // REACTOR_HPP
