#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP
#include <functional>
//#include <iostream>

namespace infra
{

template<typename T>
struct non_complex_type : std::negation<std::disjunction<std::is_fundamental<std::decay_t<T>>,
                                      std::is_enum<std::decay_t<T>>>>
{};

template<typename T>
struct complex_type : std::conjunction<std::is_class<std::decay_t<T>>,
                                     std::negation<non_complex_type<T>>>
{};


template<typename T, typename = void>
class Observable {};

template<typename T>
class Observable<T, std::enable_if_t<complex_type<T>::value>>//std::is_class_v<std::decay_t<T>>>>>
        : public T
{
    template<typename, typename>
    friend class ObservableStatePublisher;

public:
    using is_observable = std::true_type;
    using observed_type = T;

    using T::T;
    using T::operator=;

    Observable & operator=(const Observable & other){
        static_cast<T&>(*this) = static_cast<T&>(other);
        if (m_hook) m_hook(*this);
        return *this;
    }

    Observable &operator=(const T & other){
        static_cast<T&>(*this) = static_cast<T&>(other);
        if (m_hook) m_hook(*this);
        return *this;
    }

protected:
    void setHook(std::function<void(const T&)> hook) const{
        m_hook = std::move(hook);
    }

private:
     mutable std::function<void(const T &)> m_hook{};

};

template<typename T>
class Observable<T, std::enable_if_t<std::disjunction_v<std::is_fundamental<std::decay_t<T>>,
                                                                    std::is_enum<std::decay_t<T>>
                                                                    >
                                                >
                  >
{
    template<typename, typename>
    friend class ObservableStatePublisher;

public:
    using is_observable = std::true_type;
    using observed_type = T;

    Observable() = default;

    Observable(T other) :
        m_state{other}
    {}

    template<typename U, typename = std::enable_if_t<std::negation_v<std::is_same<std::decay_t<U>, T>>>>
    Observable(U && val) :
        m_state{std::forward<U>(val)}
    {}

    operator T() const {return m_state;}

    Observable & operator=(const Observable & other){
        m_state = other;
        if (m_hook) m_hook(*this);
        return *this;
    }

    Observable & operator=(const T & other){        
        m_state = other;
        if (m_hook) m_hook(*this);
        return *this;
    }

    template<typename U, typename = std::enable_if_t<std::negation_v<std::is_same<std::decay_t<U>, T>>>>
    Observable & operator=(U && val){
        m_state = std::forward<U>(val);
        if (m_hook) m_hook(*this);
        return *this;
    }

protected:
    void setHook(std::function<void(const T&)> hook) const{
        //std::cout << "hook set\n";
        m_hook = std::move(hook);
    }

private:
    mutable std::function<void(const T &)> m_hook{};
    T m_state;
};


template<typename T, typename = void>
struct is_observable : std::false_type
{};

template <typename T>
struct is_observable<T, std::void_t<std::enable_if_t<std::is_same_v<std::true_type, typename std::decay_t<T>::is_observable>>>> : std::true_type
{};

}

#endif // OBSERVABLE_HPP
