#ifndef EPOLLDEMULTIPLEXER_H
#define EPOLLDEMULTIPLEXER_H

#include <unistd.h>
#include <shared_mutex>

#include "SubscriberInfo.hpp"
#include "randomizer.hpp"
#include "shared_queue.hpp"
#include "shared_lookup_table.hpp"
#include "sys_call_eval.h"

namespace sys
{
#include <sys/epoll.h>
//#include <unistd.h>
}


namespace infra
{

namespace demux
{

uint32_t getEventsMask(const events_array & events);
events_array getEventsFromMask(uint32_t mask);

template<typename handle_t, typename = void>
class EpollDemultiplexer
{};

template<typename handle_t>
class EpollDemultiplexer<handle_t, std::enable_if_t<UnixCompatibleHandle<handle_t>::value>>
{
public:
    EpollDemultiplexer(shared_queue<EventNotification<handle_t>> & notifications_queue) :
        m_epoll_fd{-1},
        m_wait_timeout_ms{EPOLL_WAIT_TIMEOUT_MS},
        m_monitoring_ended{false},
        m_mutex{},
        m_consumer_queue{notifications_queue},
        m_handle_id_map{}
    {
        m_epoll_fd = sys::epoll_create(EPOLL_SIZE);
    }

    ~EpollDemultiplexer() { ::close(m_epoll_fd); }


    EpollDemultiplexer(EpollDemultiplexer && other) :
        m_mutex{},
        m_consumer_queue{other.m_consumer_queue}
    {
        std::unique_lock other_lck{other.m_mutex};
        m_epoll_fd = ::dup(other.m_epoll_fd);
        m_wait_timeout_ms = other.m_wait_timeout_ms;
        m_monitoring_ended = other.m_monitoring_ended;
        m_handle_id_map = std::move(other.m_handle_id_map);
    }

    EpollDemultiplexer & operator=(EpollDemultiplexer && other)
    {
        if (this == &other) return *this;
        std::lock(m_mutex, other.m_mutex);
        ::close(m_epoll_fd);
        m_epoll_fd = ::dup(other.m_epoll_fd);
        m_wait_timeout_ms = other.m_wait_timeout_ms;
        m_monitoring_ended = other.m_monitoring_ended;
        m_consumer_queue = other.m_consumer_queue;
        m_handle_id_map = std::move(other.m_handle_id_map);
        return *this;
    }

    EpollDemultiplexer(const EpollDemultiplexer &) = delete;
    EpollDemultiplexer & operator=(const EpollDemultiplexer &) = delete;

public:

    static uint32_t getEventsMask(const events_array & events){
        return demux::getEventsMask(events);
    }

    static events_array getEventsFromMask(uint32_t mask){
        return demux::getEventsFromMask(mask);
    }

    subscriber_id getKeyfor(handle_t handle){
        std::shared_lock lck{m_mutex};
        auto [sub_id, exists] = m_handle_id_map.value_for(handle);
        if (exists) return sub_id;
        return getHashedKey(handle);
    }

    bool registerListener(const SubscriberInfo<handle_t> & sub_info){
        return registerImpl(sub_info.handle, sub_info.id, sub_info.subscribed_events_mask);
    }

    bool unregisterListener(const SubscriberInfo<handle_t> & sub_info){
        return unregisterImpl(sub_info.handle);
    }

    bool updateListener(const SubscriberInfo<handle_t> & sub_info){
         return updateImpl(sub_info.handle, sub_info.id, sub_info.subscribed_events_mask);
    }

    inline void setTimeoutMs(unsigned int timeout){
        std::lock_guard lck{m_mutex};
        m_wait_timeout_ms = timeout;
    }

    inline void stopMonitoring(){
        std::lock_guard lck{m_mutex};
        m_monitoring_ended = true;
    }

    inline bool monitoring() const {
        std::shared_lock lck{m_mutex};
        return !m_monitoring_ended;
    }

    inline unsigned int getTimeoutMs() const{
        std::shared_lock lck{m_mutex};
        return m_wait_timeout_ms;
    }


    void monitorWaitThread()
    {
        using namespace sys;
        int epoll_items_ready{-1};
        epoll_event events_list[MAX_EPOLL_EVENTS];
        std::shared_lock lck{m_mutex, std::defer_lock};

        for (;;)
        {
            epoll_items_ready = epoll_wait(m_epoll_fd, events_list, MAX_EPOLL_EVENTS, m_wait_timeout_ms);
            if (0 == epoll_items_ready){
                lck.lock();
                if (m_monitoring_ended) return;
                else {
                    lck.unlock();
                    continue;
                }
            }

            if (-1 == epoll_items_ready){
                if (EINTR == errno) continue;
            }

            for (int i = 0; i < epoll_items_ready; ++i){
                m_consumer_queue.push(EventNotification<handle_t>{events_list[i].data.fd, events_list[i].events});
            }
        }

    }

protected:
    bool registerImpl(int descriptor, subscriber_id id, uint32_t events_mask){
        using namespace sys;
        epoll_event subscription_event;
        subscription_event.events = events_mask;
        subscription_event.events |= EPOLLHUP;
        subscription_event.events |= EPOLLERR;
        subscription_event.data.fd = descriptor;
        bool res = sys_call_noerr_eval(epoll_ctl, m_epoll_fd, EPOLL_CTL_ADD, descriptor, &subscription_event);
        if (res) (addHandleIdMapping(descriptor, id));
        return res;
    }

    bool unregisterImpl(int descriptor){
        std::unique_lock lck{m_mutex};
        removeHandleIdMapping(descriptor);
        return sys_call_noerr_eval(sys::epoll_ctl, m_epoll_fd, EPOLL_CTL_DEL, descriptor, nullptr);
    }

    bool updateImpl(int descriptor, subscriber_id id, uint32_t events_mask){
        sys::epoll_event subscription_event;
        subscription_event.events = events_mask;
        subscription_event.data.u64 = id;
        return sys_call_noerr_eval(sys::epoll_ctl, m_epoll_fd, EPOLL_CTL_MOD, descriptor, &subscription_event);
    }

    void addHandleIdMapping(handle_t handle, subscriber_id id){
         m_handle_id_map.add_or_update_mapping(handle, id);
    }

    void removeHandleIdMapping(handle_t handle){
        m_handle_id_map.remove_mapping(handle);
    }

    static subscriber_id getHashedKey(handle_t handle){
        static std::hash<uint32_t> int_hash;
        return int_hash(handle);
    }

private:
    static inline constexpr unsigned int EPOLL_SIZE{15};
    static inline constexpr unsigned int MAX_EPOLL_EVENTS{5};
    static inline constexpr unsigned int EPOLL_WAIT_TIMEOUT_MS{5000};

private:
    int m_epoll_fd;
    unsigned int m_wait_timeout_ms;
    bool m_monitoring_ended;
    mutable std::shared_mutex m_mutex;
    shared_queue<EventNotification<handle_t>> & m_consumer_queue;
    shared_lookup_table<handle_t, subscriber_id, std::vector> m_handle_id_map;

};

} //demux

} //infra
#endif // EPOLLDEMULTIPLEXER_H


