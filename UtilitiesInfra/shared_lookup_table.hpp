#ifndef DEMUXTABLE_HPP
#define DEMUXTABLE_HPP

#include <unordered_map>
#include <vector>
#include <set>
#include <shared_mutex>
#include <iterator>
#include <algorithm>
#include <mutex>

namespace infra
{

template<typename Key, typename Container>
std::vector<Key> getKeysFromContainer(const Container & container);

template<typename Key, typename Value, template<typename... > typename Container>
class shared_lookup_table
{};

class container_access
{

    template<typename LookupTable>
    friend class container_operations;

private:
    template<typename T>
    static decltype(auto) getInternalContainer(T & custom_container){
        return custom_container.getInternalContainer();
    }

    template<typename T>
    static decltype(auto) getInternalMutex(T & custom_container){
        return custom_container.getInternalMutex();
    }

    template<typename T, typename K>
    static void removeMapping(T & custom_container, const K & key){
        return custom_container.remove_mapping(key);
    }

};


template<typename LookupTable>
class container_operations
{

public:
    template<typename UnaryFunction>
    void for_each(UnaryFunction f){
        auto & sh_mutex           = container_access::getInternalMutex(static_cast<LookupTable&>(*this));
        auto & internal_container = container_access::getInternalContainer(static_cast<LookupTable&>(*this));
        std::unique_lock lck{sh_mutex};
        std::for_each(internal_container.begin(), internal_container.end(), f);
    }

    template<typename UnaryPredicate>
    void remove_if(UnaryPredicate pred){        
        auto & sh_mutex           = container_access::getInternalMutex(static_cast<LookupTable&>(*this));
        auto & internal_container = container_access::getInternalContainer(static_cast<LookupTable&>(*this));
        using key_type = std::decay_t<decltype(internal_container.begin()->first)>;

        static std::vector<key_type> removed_keys;
        removed_keys.reserve(internal_container.size());

        std::unique_lock lck{sh_mutex};
        std::for_each(internal_container.begin(), internal_container.end(),
                      [&pred](const auto & pair){ if (pred(pair)) removed_keys.push_back(pair.first); });
        lck.unlock();
        for (auto & key : removed_keys){
            container_access::removeMapping(static_cast<LookupTable&>(*this), key);
        }

        removed_keys.clear();
    }

    template<typename UnaryPredicate>
    decltype(auto) find_if(UnaryPredicate pred){
        auto & sh_mutex           = container_access::getInternalMutex(static_cast<LookupTable&>(*this));
        auto & internal_container = container_access::getInternalContainer(static_cast<LookupTable&>(*this));
        using mapped_t = std::decay_t<decltype(internal_container.begin()->second)>;

        std::unique_lock lck{sh_mutex};
        if (auto it = std::find_if(internal_container.begin(), internal_container.end(), pred);
                it != internal_container.end()){
            return std::make_pair(it->second, true);
        }

        return std::make_pair<mapped_t, bool>(mapped_t(), false);
    }

    template<typename UnaryPredicate, typename UnaryFunction>
    void update_if(UnaryPredicate pred, UnaryFunction update){
        auto & sh_mutex           = container_access::getInternalMutex(static_cast<LookupTable&>(*this));
        auto & internal_container = container_access::getInternalContainer(static_cast<LookupTable&>(*this));
        std::unique_lock lck{sh_mutex};
        std::for_each(internal_container.begin(), internal_container.end(),
                      [&pred, &update](auto & item){ if (pred(item)) update(item);});
    }
};

template<typename Key, typename Value>
class shared_lookup_table<Key, Value, std::unordered_map> :
        public container_operations<shared_lookup_table<Key, Value, std::unordered_map>>
{
    static_assert (std::is_default_constructible<Value>::value, "Value type must be default constructible");
    friend class container_access;

    using shared_lock       = std::shared_lock<std::shared_mutex>;
    using unique_lock       = std::unique_lock<std::shared_mutex>;
    using container         = std::unordered_map<Key, Value>;

public:
    using lookup_table      = shared_lookup_table<Key, Value, std::unordered_map>;
    using iterator          = typename container::iterator;
    using const_iterator    = typename container::const_iterator;
    using size_type         = typename container::size_type;
    using key_type          = typename container::key_type;
    using mapped_type       = typename container::mapped_type;

protected:
    container & getInternalContainer(){ return internal_container; }
    std::shared_mutex & getInternalMutex() { return sh_mutex; }    


public:
    shared_lookup_table() :
        internal_container{},
        sh_mutex{}
    {}

    ~shared_lookup_table() = default;
    shared_lookup_table(const shared_lookup_table & other) :
        sh_mutex{}
    {
        unique_lock lck{other.sh_mutex};
        internal_container = other.internal_container;
    }

    shared_lookup_table(shared_lookup_table && other) :
        sh_mutex{}
    {
        unique_lock lck{other.sh_mutex};
        internal_container = std::move(other.internal_container);
    }

    shared_lookup_table & operator=(const shared_lookup_table & other)
    {
        if (this == &other){
            return *this;
        }

        std::lock(sh_mutex, other.sh_mutex);
        internal_container = other.internal_container;
        return *this;
    }

    shared_lookup_table & operator=(shared_lookup_table && other)
    {
        if (this == &other){
            return *this;
        }

        std::lock(sh_mutex, other.sh_mutex);
        internal_container = std::move(other.internal_container);
        return *this;
    }

public:

    template<typename M>
    bool add_or_update_mapping(const key_type & key, M && mapped_value){
        unique_lock lck{sh_mutex};
        auto [it, res] = internal_container.insert_or_assign(key, std::forward<M>(mapped_value));
        return res;
    }

    std::pair<mapped_type, bool> value_for(const key_type & key) const{
        shared_lock lck{sh_mutex};
        if (auto it = internal_container.find(key);
            it != internal_container.end()){
            return std::make_pair(it->second, true);
        }
        return std::make_pair(mapped_type(), true);
    }

    void remove_mapping(const key_type & key){
         unique_lock lck{sh_mutex};
         internal_container.erase(key);
    }

    bool empty() const noexcept{
        shared_lock lck{sh_mutex};
        return internal_container.empty();
    }

    size_type size() const noexcept{
        shared_lock lck{sh_mutex};
        return internal_container.size();
    }

    std::vector<key_type> get_keys_snapshot() const noexcept{
        shared_lock lck{sh_mutex};
        return getKeysFromContainer<key_type, container>(internal_container);
    }

    lookup_table getSubsectionForKeys(const std::vector<key_type> & keys) const{
        shared_lock lck{sh_mutex};
        lookup_table subsection;
        for (const auto & key : keys){
            if (auto it = internal_container.find(key);
                it != internal_container.cend()){
                subsection.internal_container.insert_or_assign(it->first, it->second);
            }
        }
        return subsection;
    }

    lookup_table clone() const{
        lookup_table clone;
        shared_lock lck{sh_mutex};
        clone.internal_container = internal_container;
        return clone;
    }

    void clear() noexcept{
        unique_lock lck{sh_mutex};
        internal_container.clear();
    }

private:
    container internal_container;
    mutable std::shared_mutex sh_mutex;
};

template<typename Key, typename Value>
class shared_lookup_table<Key, Value, std::vector> :
        public container_operations<shared_lookup_table<Key, Value, std::vector>>
{
    static_assert (std::is_default_constructible<Value>::value, "Value type must be default constructible");
    friend class container_access;

    using shared_lock       = std::shared_lock<std::shared_mutex>;
    using unique_lock       = std::unique_lock<std::shared_mutex>;
    using container         = std::vector<std::pair<Key, Value>>;

public:
    using lookup_table      = shared_lookup_table<Key, Value, std::vector>;
    using iterator          = typename container::iterator;
    using const_iterator    = typename container::const_iterator;
    using size_type         = typename container::size_type;
    using key_type          = Key;
    using mapped_type       = Value;

protected:
    container & getInternalContainer(){ return internal_container; }
    std::shared_mutex & getInternalMutex() { return sh_mutex; }

public:
    shared_lookup_table() :
        internal_container{},
        sh_mutex{}
    {}

    ~shared_lookup_table() = default;
    shared_lookup_table(const shared_lookup_table & other) :
        sh_mutex{}
    {
        unique_lock lck{other.sh_mutex};
        internal_container = other.internal_container;
    }

    shared_lookup_table(shared_lookup_table && other) :
        sh_mutex{}
    {
        unique_lock lck{other.sh_mutex};
        internal_container = std::move(other.internal_container);
    }

    shared_lookup_table & operator=(const shared_lookup_table & other)
    {
        if (this == &other){
            return *this;
        }

        std::lock(sh_mutex, other.sh_mutex);
        internal_container = other.internal_container;
        return *this;
    }

    shared_lookup_table & operator=(shared_lookup_table && other)
    {
        if (this == &other){
            return *this;
        }

        std::lock(sh_mutex, other.sh_mutex);
        internal_container = std::move(other.internal_container);
        return *this;
    }

private:
    iterator get_value_position_for(const key_type & key){
        return std::find_if(internal_container.begin(), internal_container.end(),
                            [&key](const auto & pair){return key == pair.first;});
    }

    const_iterator get_value_position_for(const key_type & key) const{
        return std::find_if(internal_container.cbegin(), internal_container.cend(),
                            [&key](const auto & pair){return key == pair.first;});
    }

    void internal_sort(){
        std::sort(internal_container.begin(), internal_container.end(),
                  [] (const auto & pair1, const auto & pair2){return pair1.first < pair2.first;});
    }

public:

    template<typename M>
    bool add_or_update_mapping(const key_type & key, M && mapped_value){
        unique_lock lck{sh_mutex};
        if (iterator it = get_value_position_for(key);
            it != internal_container.end()){
            it->second = std::forward<M>(mapped_value);
            return true;
        }
        else{
            internal_container.emplace_back(std::make_pair(key, std::forward<M>(mapped_value)));
            internal_sort();
        }
        return false;
    }

    std::pair<mapped_type, bool> value_for(const key_type & key) const{
        shared_lock lck{sh_mutex};
        if (auto it = get_value_position_for(key);
           it != internal_container.cend()){
            return std::make_pair(it->second, true);
        }
        return std::make_pair(mapped_type(), false);
    }

    void remove_mapping(const key_type & key){
        unique_lock lck{sh_mutex};
        internal_container.erase(std::remove_if(internal_container.begin(), internal_container.end(),
                                                [&key](const auto & pair){return key == pair.first;}));
    }

    bool empty() const noexcept{
        shared_lock lck{sh_mutex};
        return internal_container.empty();
    }

    size_type size() const noexcept{
        shared_lock lck{sh_mutex};
        return internal_container.size();
    }

    std::vector<key_type> get_keys_snapshot() const noexcept{
        shared_lock lck{sh_mutex};
        return getKeysFromContainer<key_type, container>(internal_container);
    }

    lookup_table getSubsectionForKeys(const std::vector<key_type> & keys) const{
        lookup_table subsection;
        subsection.internal_container.reserve(keys.size());
        shared_lock lck{sh_mutex};
        for (const auto & key : keys){
            if (auto it = get_value_position_for(key);
                it != internal_container.cend()){
                subsection.add_or_update_mapping(it->first, it->second);
            }
        }
        return subsection;
    }

    lookup_table clone() const{        
        lookup_table clone;
        shared_lock lck{sh_mutex};
        clone.internal_container = internal_container;
        return clone;
    }

    void clear() noexcept{
        unique_lock lck{sh_mutex};
        internal_container.clear();
    }

    /*

    template<typename UnaryFunction>
    void for_each(UnaryFunction f){
        unique_lock lck{sh_mutex};
        std::for_each(internal_container.begin(), internal_container.end(), f);
    }

    template<typename UnaryPredicate>
    void remove_if(UnaryPredicate pred){
        static std::vector<key_type> removed_keys;
        removed_keys.reserve(internal_container.size());
        unique_lock lck{sh_mutex};
        std::for_each(internal_container.begin(), internal_container.end(),
                      [&pred](const auto & pair){ if (pred(pair)) removed_keys.push_back(pair.first); });

        lck.unlock();
        for (auto & key : removed_keys){
            remove_mapping(key);
        }

        removed_keys.clear();
    }

    template<typename UnaryPredicate>
    std::pair<mapped_type, bool> find_if(UnaryPredicate pred){
        unique_lock lck{sh_mutex};
        if (auto it = std::find_if(internal_container.begin(), internal_container.end(), pred);
                it != internal_container.end()){
            return std::make_pair(it->second, true);
        }
        return std::make_pair(mapped_type(), false);
    }

    template<typename UnaryPredicate, typename UnaryFunction>
    void update_if(UnaryPredicate pred, UnaryFunction update){
        unique_lock lck{sh_mutex};
        std::for_each(internal_container.begin(), internal_container.end(),
                      [&pred, &update](auto & item){ if (pred(item)) update(item);});
    }
    */

private:
    container internal_container;
    mutable std::shared_mutex sh_mutex;
};

template<typename Key, typename Container>
std::vector<Key> getKeysFromContainer(const Container & container)
{
    std::vector<Key> key_collection;
    key_collection.reserve(container.size());
    std::transform(container.begin(), container.end(), std::back_inserter(key_collection),
                   [](const auto & pair) {return pair.first;});
    return key_collection;
}

}//infra
#endif // DEMUXTABLE_HPP
