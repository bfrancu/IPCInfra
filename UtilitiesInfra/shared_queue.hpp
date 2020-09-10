#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <iostream>

namespace infra
{
template <typename T>
class shared_queue
{
    using lck_guard = std::lock_guard<std::mutex>;
    using lck_unique = std::unique_lock<std::mutex>;

public:
    shared_queue()         
    {}

    shared_queue(const shared_queue & other)                 
    {
        lck_guard lock(other.m_mutex);
        m_queue = other.m_queue;
    }

    shared_queue(shared_queue && other)
    {
        lck_guard lock(other.m_mutex);
        m_queue = std::move(other.m_queue);
    }

    shared_queue & operator=(const shared_queue & other)
    {
        if (this == &other){
            return *this;
        }

        std::lock(m_mutex, other.m_mutex);
        m_queue = other.m_queue;
        return *this;
    }

    shared_queue & operator=(shared_queue && other)
    {
        if (this == &other){
            return *this;
        }

        std::lock(m_mutex, other.m_mutex);
        m_queue = std::move(other.m_queue);
        return *this;
    }

    template<typename U>
    void push(U && new_value)
    {
        lck_guard lock(m_mutex);
        m_queue.emplace(std::forward<U>(new_value));
        //std::cout << "shared_queue: value added to queue\n";
        m_data_cond.notify_all();
    }

    bool try_pop(T & value)
    {
        lck_guard lock(m_mutex);
        if (m_queue.empty())
        {
            return false;
        }
        value = m_queue.front();
        m_queue.pop();
        //std::cout << "shared_queue: value popped returning true\n";
        return true;
    }

    std::shared_ptr<T> try_pop()
    {    
        lck_guard lock(m_mutex);
        if (m_queue.empty())
        {
            return std::shared_ptr<T>(nullptr);
        }
        else
        {
            std::shared_ptr<T> ret = std::make_shared<T>(m_queue.front());
            m_queue.pop();
            return ret;
        }
    }

    void wait_and_pop(T & value)
    {
        lck_unique u_lock(m_mutex);
        m_data_cond.wait(u_lock, [this] { return !m_queue.empty(); });
        value = m_queue.front();
        m_queue.pop();        
    }

    std::shared_ptr<T> wait_and_pop()
    {
        lck_unique u_lock(m_mutex);
        m_data_cond.wait(u_lock, [this] {return !m_queue.empty(); });
        std::shared_ptr<T> ret = std::make_shared<T>(m_queue.front());
        m_queue.pop();
        return ret;
    }

    bool empty() const
    {
        lck_guard lock(m_mutex);
        return m_queue.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_data_cond;
    std::queue<T> m_queue;
    
};

} //infra
