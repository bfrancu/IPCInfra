#ifndef ENDPOINT_STORAGE_HPP
#define ENDPOINT_STORAGE_HPP
#include <memory>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <iostream>

#include "default_traits.hpp"

namespace infra
{


template<typename Endpoint, typename Key>
class SingleEndpointStorage
{
public:
    using endpoint_t = Endpoint;
    using key_t = Key;
    using endpoint_ptr = std::unique_ptr<endpoint_t>;
    using optional_endpoint_ptr_ref = std::optional<std::reference_wrapper<endpoint_ptr>>; 
    using const_optional_endpoint_ptr_ref = std::optional<std::reference_wrapper<const endpoint_ptr>>;

public:
    bool store(key_t key, endpoint_ptr p_endpoint) {
        m_pEndpoint = std::move(p_endpoint);
        m_key = std::move(key);
        return true;
    }

    inline void erase(const key_t & key) {
        if (key == m_key){
            m_pEndpoint.reset();
            m_key = meta::traits::default_value<key_t>::value;
        }
    }

    optional_endpoint_ptr_ref getEndpointFor(const key_t & key) {
        optional_endpoint_ptr_ref ret;
        if (key == m_key){
            ret = std::ref(m_pEndpoint);
        }
        return ret;
    }

    const_optional_endpoint_ptr_ref getEndpointFor(const key_t & key) const{
        const_optional_endpoint_ptr_ref ret;
        if (key == m_key){
            ret = std::cref(m_pEndpoint);
        }
        return ret;
    }

    template<typename UnaryFunction>
    void forEach(UnaryFunction f){
        f(m_pEndpoint);
    }

private:
    endpoint_ptr m_pEndpoint{nullptr};
    key_t m_key{meta::traits::default_value<key_t>::value};
};

template<typename Endpoint, typename Key>
class AssociativeEndpointStorage
{
public:
    using endpoint_t = Endpoint;
    using key_t = Key;
    using endpoint_ptr = std::unique_ptr<endpoint_t>;
    using container_t = std::unordered_map<Key, endpoint_ptr>;
    using optional_endpoint_ptr_ref = std::optional<std::reference_wrapper<endpoint_ptr>>; 
    using const_optional_endpoint_ptr_ref = std::optional<std::reference_wrapper<const endpoint_ptr>>; 

public:
    bool store(key_t key, endpoint_ptr p_endpoint)
    {
        m_container.insert_or_assign(key, std::move(p_endpoint));
        return true;
    }

    void erase(const key_t & key)
    {
        auto [found, iter] = valueFor(key);
        if (found){
            std::cout << "AssociativeEndpointStorage::erase() removing by key: " << key << "\n";
            m_container.erase(iter);
        }
    }

    optional_endpoint_ptr_ref getEndpointFor(const key_t & key){
        optional_endpoint_ptr_ref ret;
        auto [found, iter] = valueFor(key);
        if (found){
            ret = std::ref(iter->second);
        }
        return ret;
    }

    const_optional_endpoint_ptr_ref getEndpointFor(const key_t & key) const{
        const_optional_endpoint_ptr_ref ret;
        auto [found, iter] = valueFor(key);
        if (found){
            ret = std::cref(iter->second);
        }
        return ret;
    }

    template<typename UnaryFunction>
    void forEach(UnaryFunction f)
    {
        std::for_each(std::begin(m_container), std::end(m_container), f);
    }

protected:
    std::pair<bool, typename container_t::iterator> valueFor(const Key & key)
    {
        auto it = m_container.find(key);
        return std::make_pair((std::end(m_container) != it), it);
    }

    std::pair<bool, typename container_t::const_iterator> valueFor(const key_t & key) const
    {
        auto it = m_container.find(key);
        return std::make_pair((std::cend(m_container) != it), it);
    }

private:
    container_t m_container;
};

}//infra

#endif
