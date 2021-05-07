#ifndef EPOLLDEMULTIPLEXER_H
#define EPOLLDEMULTIPLEXER_H

#include <ios>
#include <unistd.h>
#include <shared_mutex>

#include "SubscriberInfo.hpp"
#include "randomizer.hpp"
#include "shared_queue.hpp"
#include "shared_lookup_table.hpp"
#include "sys_call_eval.h"

#include <sys/epoll.h>
namespace sys
{
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
        std::cout << "EpollDemultiplexer: creating epoll fd\n";
        m_epoll_fd = ::epoll_create(EPOLL_SIZE);
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

    subscriber_id getKeyFor(handle_t handle){
        std::shared_lock lck{m_mutex};
        auto [sub_id, exists] = m_handle_id_map.value_for(handle);
        std::cout << "EpollDemultiplexer::getKeyFor() handle: " << handle << "; sub_id: " << sub_id 
                  << "; exists: " << std::boolalpha << exists << "\n";
        if (exists) return sub_id;
        return getHashedKey(handle);
    }

    bool registerListener(const SubscriberContext<handle_t> & sub_info){
        std::cout << "EpollDemultiplexer::registerListener()\n";
        return registerImpl(sub_info.handle, sub_info.id, sub_info.subscribed_events_mask);
    }

    bool unregisterListener(const SubscriberContext<handle_t> & sub_info){
        std::cout << "EpollDemultiplexer::unregisterListener()\n";
        return unregisterImpl(sub_info.handle);
    }

    bool updateListener(const SubscriberContext<handle_t> & sub_info){
        std::cout << "EpollDemultiplexer::updateListener()\n";
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
        //using namespace sys;
        std::cout << "EpollDemultiplexer::monitorWaitThread() thread started\n";
        int epoll_items_ready{-1};
        epoll_event events_list[MAX_EPOLL_EVENTS];
        std::shared_lock lck{m_mutex, std::defer_lock};

        for (;;)
        {
            epoll_items_ready = ::epoll_wait(m_epoll_fd, events_list, MAX_EPOLL_EVENTS, m_wait_timeout_ms);
            if (0 == epoll_items_ready){
                lck.lock();
                if (m_monitoring_ended) {
                    std::cout << "EpollDemultiplexer::monitorWaitThread() thread stopped\n";
                    return;
                }
                else {
                    lck.unlock();
                    continue;
                }
            }

            if (-1 == epoll_items_ready){
                if (EINTR == errno) continue;
            }

            for (int i = 0; i < epoll_items_ready; ++i){
                std::cout << "EpollDemultiplexer::monitorWaitThread() new events mask: " << events_list[i].events << "\n";
                /* If EPOLLIN and EPOLLHUP were both set, then there might be more than MAX_BUF bytes to read. Therefore, we close
                the file descriptor only if EPOLLIN was not set. We'll read further bytes after the next epoll_wait(). 
                Michael Kerisk - The Linux Programming Interface, 63.4.3*/
                if ((events_list[i].events & EPOLLHUP) && (events_list[i].events & EPOLLIN)){
                    events_list[i].events = (events_list[i].events & ~EPOLLHUP);
                }
                std::cout << "EpollDemultiplexer::monitorWaitThread() passing events mask: " << events_list[i].events << "\n";
                if (events_list[i].events & EPOLLERR)
                {
                    std::cout << "EpollDemultiplexer::monitorWaitThread() error event received errno: " << errno << "\n";
                }
                if (events_list[i].events & EPOLLHUP)
                {
                    std::cout << "EpollDemultiplexer::monitorWaitThread() hangup event received errno: " << errno << "\n";
                }
                if (events_list[i].events & EPOLLRDHUP)
                {
                    std::cout << "EpollDemultiplexer::monitorWaitThread() shutdown event received errno: " << errno << "\n";
                }
                if (events_list[i].events & EPOLLOUT)
                {
                    std::cout << "EpollDemultiplexer::monitorWaitThread() out event received errno: " << errno << "\n";
                }
                m_consumer_queue.push(EventNotification<handle_t>{events_list[i].data.fd, events_list[i].events});
            }
        }

    }

protected:
    bool registerImpl(int descriptor, subscriber_id id, uint32_t events_mask){
        std::cout << "EpollDemultiplexer::registerImpl() handle: " << descriptor << "\n";
        //using namespace sys;
        epoll_event subscription_event;
        subscription_event.events = events_mask;
        std::cout << "EpollDemultiplexer::registerImpl() subscription mask: " << subscription_event.events << "\n";
        //subscription_event.events |= EPOLLIN;
        //std::cout << "EpollDemultiplexer::registerImpl() subscription mask after EPOLLIN: " << subscription_event.events << "\n";
        //subscription_event.events |= EPOLLOUT;
        //std::cout << "EpollDemultiplexer::registerImpl() subscription mask after EPOLLOUT: " << subscription_event.events << "\n";
        //subscription_event.events |= (EPOLLIN | EPOLLPRI | EPOLLOUT);
        subscription_event.events |= (EPOLLHUP | EPOLLERR);
        subscription_event.events |= EPOLLRDHUP;
        subscription_event.events |= EPOLLET;
        subscription_event.data.fd = descriptor;
        //bool res = sys_call_noerr_eval(::epoll_ctl, m_epoll_fd, EPOLL_CTL_ADD, descriptor, &subscription_event);
        bool res = (-1 != ::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, descriptor, &subscription_event));
        if (res) (addHandleIdMapping(descriptor, id));
        else std::cout << "EpollDemultiplexer::registerImpl() error adding to the epoll descriptor; errno " << errno << "\n";

        return res;
    }

    bool unregisterImpl(int descriptor){
        std::cout << "EpollDemultiplexer::unregisterImpl() descriptor: " << descriptor << "\n";
        using namespace sys;
        std::unique_lock lck{m_mutex};
        removeHandleIdMapping(descriptor);
        bool res = (-1 != ::epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, descriptor, nullptr));
        if (!res) std::cout << "EpollDemultiplexer::unregisterImpl() error removing from the epoll descriptor; errno " << errno << "\n";
        return res;
        //return sys_call_noerr_eval(::epoll_ctl, m_epoll_fd, EPOLL_CTL_DEL, descriptor, nullptr);
    }

    bool updateImpl(int descriptor, subscriber_id id, uint32_t events_mask){
        std::cout << "EpollDemultiplexer::updateImpl()\n";
        epoll_event subscription_event;
        subscription_event.events = events_mask;
        subscription_event.data.u64 = id;
        return sys_call_noerr_eval(::epoll_ctl, m_epoll_fd, EPOLL_CTL_MOD, descriptor, &subscription_event);
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


