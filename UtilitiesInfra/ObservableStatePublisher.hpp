#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <algorithm>
#include <vector>
#include <mutex>

#include "Observable.hpp"

namespace infra
{

template<typename State,
         typename Dispatcher>
class ObservableStatePublisher
{

public:
    using subscriber_id = size_t;
    using callable = std::function<void(const State&)>;
    using delegates_container = std::vector<std::pair<subscriber_id, callable>>;

public:
    ObservableStatePublisher(const Observable<State> & observed_state, Dispatcher & task_dispatcher) :
        m_mutex{},
        m_observable{observed_state},
        m_task_dispatcher{task_dispatcher},
        m_delegates{}
    {
        m_delegates.reserve(5);
        init();
    }


public:
    subscriber_id addObserver(callable cb) {
          std::lock_guard<std::mutex> lock(m_mutex);
          auto & client_id = currentId();
          m_delegates.emplace_back(std::make_pair(++client_id, std::move(cb)));
          return client_id;
    }

    bool removeObserver(subscriber_id id){
        std::lock_guard<std::mutex> lock(m_mutex);
        if (auto removable_iter = std::remove_if(m_delegates.begin(), m_delegates.end(), [=] (auto & pair){return pair.first == id;});
                removable_iter != m_delegates.end()){
            m_delegates.erase(removable_iter);
            return true;
        }
        return false;
    }

private:
    static subscriber_id & currentId(){
        static subscriber_id current_id{0};
        return current_id;
    }

private:
    void init(){
        auto fn{std::bind(&ObservableStatePublisher::notify, this, std::placeholders::_1)};
        m_observable.setHook(std::move(fn));
    }

    delegates_container getDelegatesCopy() const{
        delegates_container copy_delegates{m_delegates.size()};
        std::copy(m_delegates.begin(), m_delegates.end(), copy_delegates.begin());
        return copy_delegates;
    }

    void notify(const State & state){
        //std::cout << "Notify\n";
        std::unique_lock<std::mutex> lck{m_mutex};
        auto copy_delegates = getDelegatesCopy();
        lck.unlock();
        std::for_each(copy_delegates.begin(), copy_delegates.end(), [this, &state](auto id_cb_pair){
              m_task_dispatcher.dispatch(id_cb_pair.second, std::cref(state));
        });
    }

private:
    mutable std::mutex m_mutex;
    const Observable<State> & m_observable;
    Dispatcher & m_task_dispatcher;
    delegates_container m_delegates;
};

template<typename Publisher>
class PublisherDelegate
{

    using callable = typename Publisher::callable;
    using subscriber_id = typename Publisher::subscriber_id;

public:
    PublisherDelegate(Publisher & publisher) :
        m_publisher{publisher}
    {}

    subscriber_id operator+=(callable cb){
        return m_publisher.addObserver(cb);
    }

    bool operator-=(subscriber_id id){
        return m_publisher.removeObserver(id);
    }

private:
    Publisher & m_publisher;
};

}



#endif // OBSERVER_HPP
