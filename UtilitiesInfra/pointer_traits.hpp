#ifndef POINTER_TRAITS_HPP
#define POINTER_TRAITS_HPP
#include <memory>
#include <type_traits>

namespace infra
{
namespace meta
{
namespace traits
{
//https://stackoverflow.com/questions/21174593/downcasting-unique-ptrbase-to-unique-ptrderived#21174979
template<typename A, typename B>
struct are_related : std::disjunction<std::is_base_of<A, B>, std::is_base_of<B, A>> {};

template<typename A, typename B>
constexpr bool are_related_v = are_related<A, B>::value;

template<typename From, typename To,
         template <typename> typename Deleter,
         typename = std::enable_if_t<are_related_v<From, To>>>
std::unique_ptr<To, Deleter<To>> static_cast_unique_ptr(std::unique_ptr<From, Deleter<From>> && ptr_source)
{
    auto ptr_dest = static_cast<To*>(ptr_source.release());
    if constexpr (std::is_convertible_v<Deleter<From>, Deleter<To>>)
    {
        return std::unique_ptr<To, Deleter<To>>(ptr_dest, std::move(ptr_source.get_deleter()));
    }
    else
    {
        return std::unique_ptr<To, Deleter<To>>(ptr_dest, Deleter<To>{});
    }
}

template<typename From, typename To,
         template <typename> typename Deleter>
std::unique_ptr<To, Deleter<To>> dynamic_cast_unique_ptr(std::unique_ptr<From, Deleter<From>> && ptr_source)
{
    if (To *ptr_dest = dynamic_cast<To*>(ptr_source.get())){
        ptr_source.release();
        return std::unique_ptr<To, Deleter<To>>(ptr_dest, Deleter<To>{});
    }
    return std::unique_ptr<To, Deleter<To>>(nullptr, Deleter<To>{});
}

}//traits
}//meta

/*
namespace utils
{

template<typename T>
struct deferred_copy_shared_ptr
{
    deferred_copy_shared_ptr(const std::shared_ptr<T> & shared_ptr, bool deferred_copy = true) :
        shared_ref{std::ref(shared_ptr)}
    {
        if (!deferred_copy){
            aquire_copy();
        }
    }

    deferred_copy_shared_ptr(const deferred_copy_shared_ptr & other)
    {
    }

    deferred_copy_shared_ptr(deferred_copy_shared_ptr && other)
    {}

    inline void aquire_copy() { p_shared = shared_ref.get(); }

    inline T & operator*() {
        if (p_shared) return *p_shared;
        return *(shared_ref.get());
    }

    inline T * operator->(){
        if (p_shared) return p_shared.get();
        return (shared_ref.get()).get();
    }

    std::reference_wrapper<std::shared_ptr<T>> shared_ref;
    std::shared_ptr<T> p_shared{nullptr};
};
*/

}//utils

}//infra

#endif
