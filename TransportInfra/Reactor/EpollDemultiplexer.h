#ifndef EPOLLDEMULTIPLEXER_H
#define EPOLLDEMULTIPLEXER_H

#include <mutex>

#include "SubscriberInfo.hpp"
#include "randomizer.hpp"

namespace infra
{

namespace demux
{

class EpollDemultiplexer
{
public:
    EpollDemultiplexer();
    ~EpollDemultiplexer();

    EpollDemultiplexer(const EpollDemultiplexer &) = delete;
    EpollDemultiplexer & operator=(const EpollDemultiplexer &) = delete;

public:
    static uint32_t getEventsMask(const events_array & events);
    static events_array getEventsFromMask(uint32_t mask);

    template<typename handle_type, typename = UnixCompatibleHandleT<handle_type>>
    static subscriber_id getHashedKey(handle_type handle){
        static std::hash<uint32_t> int_hash;
        // generate a random hash key
        //uint32_t random_hash_key = getRandomNumberInRange<uint32_t>(0, UINT32_MAX);
        // generate a hash value for the key
        return int_hash(handle);
    }

    template<typename handle_type, typename = UnixCompatibleHandleT<handle_type>>
    bool registerListener(const SubscriberInfo<handle_type> & sub_info){
        return registerImpl(sub_info.handle, sub_info.id, sub_info.subscribed_events_mask);
    }

    template<typename handle_type, typename = UnixCompatibleHandleT<handle_type>>
    bool unregisterListener(const SubscriberInfo<handle_type> & sub_info){
        return unregisterImpl(sub_info.handle);
    }

    inline void setTimeoutMs(unsigned int timeout){
        std::lock_guard<std::mutex> lck{m_mutex};
        m_wait_timeout_ms = timeout;
    }

    void monitorWaitThread();

private:
    bool registerImpl(int descriptor, subscriber_id id, uint32_t events_mask);
    bool unregisterImpl(int descriptor);

private:
    mutable std::mutex m_mutex;
    int m_epoll_fd;
    unsigned int m_wait_timeout_ms;

};

} //demux

} //infra
#endif // EPOLLDEMULTIPLEXER_H
