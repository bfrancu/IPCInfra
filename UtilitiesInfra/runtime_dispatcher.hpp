#ifndef RUNTIME_DISPATCHER_HPP
#define RUNTIME_DISPATCHER_HPP
#include "typelist.hpp"
#include "function_traits.hpp"

namespace infra
{
namespace meta
{

namespace dispatch
{

template<bool IsConst>
struct object_wrapper
{
    void *object;
};

template<>
struct object_wrapper<true>
{
    const void *object;
};

template<typename T>
object_wrapper<false> wrap(T & value) {
    object_wrapper<false> res;
    res.object = reinterpret_cast<void*>(&value);
    return res;
}


template<typename T>
object_wrapper<true> wrap_const(const T & value) {
    object_wrapper<true> res;
    res.object = reinterpret_cast<const void*>(&value);
    return res;
}

object_wrapper<false> wrap(void *value);
object_wrapper<true> wrap_const(const void *value);

template<typename T, typename ObjectWrapper>
struct cast_object;

template<typename T, bool IsConst>
struct cast_object<T, object_wrapper<IsConst>>
{
    using type = T*;
};

template<typename T>
struct cast_object<T, object_wrapper<true>>
{
    using type =  std::add_const_t<T>*;
};

template<typename T, typename ObjectWrapper>
using cast_object_t = typename cast_object<T, ObjectWrapper>::type;

template<typename T, bool IsConst>
decltype(auto) extractValue(object_wrapper<IsConst> wrapper)
{
    return *(reinterpret_cast<cast_object_t<T, object_wrapper<IsConst>>>(wrapper.object));
}

template<typename TList, std::size_t N>
struct dispatch_helper
{
    template<typename F, typename... Args>
    static void dispatch(std::size_t tag, F && f, Args&&... args)
    {
        if (N == tag) {
            using object_t = tl::nth_element_t<TList, N>;
            if constexpr (!std::is_same_v<tl::empty_type, object_t>){
                f(object_t(), std::forward<Args>(args)...);
            }
        }
        else if constexpr (N < tl::size_v<TList> - 1) {
            dispatch_helper<TList, N+1>::dispatch(tag, std::forward<F>(f),
                                                       std::forward<Args>(args)...);
        } 
    }

    template<bool IsConst, typename F, typename... Args>
    static void dispatch(std::size_t tag, F && f, object_wrapper<IsConst> wrapper, Args&&... args)
    {
        if (N == tag) {
            using object_t = tl::nth_element_t<TList, N>;
            if constexpr (!std::is_same_v<tl::empty_type, object_t>){
                auto & ref = extractValue<object_t>(wrapper);
                f(ref, std::forward<Args>(args)...);
            }
        }
        else if constexpr (N < tl::size_v<TList> - 1) {
            dispatch_helper<TList, N+1>::dispatch(tag, std::forward<F>(f), wrapper,
                                                       std::forward<Args>(args)...);
        }
    }

};

template<typename TList, typename F, typename... Args>
decltype(auto) dispatch(std::size_t tag, F && f, Args &&... args)
{
    return dispatch_helper<TList, 0>::dispatch(tag, std::forward<F>(f), std::forward<Args>(args)...);
}

template<auto V, typename = void>
struct delegate_call
{
    static void call(...) {}
};

template<auto StaticMember>
struct delegate_call<StaticMember, traits::pointer_to_static_constraint<StaticMember>>
{
    using return_t = typename traits::return_type_from_function<decltype(StaticMember)>::type;

    template<typename Subject, typename... Args,
             typename = decltype(StaticMember(std::declval<Args&&>()...))>
    static void call(Subject && s, Args &&... args){
        (void) s;
        StaticMember(std::forward<Args>(args)...);
    }

    static void call(...) {}
};

template<auto PtrToMemberFunc>
struct delegate_call<PtrToMemberFunc, traits::pointer_to_member_constraint<PtrToMemberFunc>>
{
    using ptr_to_member_t = decltype(PtrToMemberFunc);
    using subject_t        = typename traits::types_from_class_member<ptr_to_member_t>::parent_type;
    using member_t        = typename traits::types_from_class_member<ptr_to_member_t>::member_type;
    using return_t        = typename traits::return_type_from_function<member_t>::type;

    template<typename... Args, typename,
             typename = std::enable_if_t<std::is_invocable_r_v<return_t, ptr_to_member_t, subject_t, Args...>>>
    static void call(const subject_t & s, Args &&... args) {
        (s.*PtrToMemberFunc)(std::forward<Args>(args)...);
    }

    template<typename... Args,
             typename = std::enable_if_t<std::is_invocable_r_v<return_t, ptr_to_member_t, subject_t, Args...>>>
    static void call(subject_t & s, Args &&... args) {
        (s.*PtrToMemberFunc)(std::forward<Args>(args)...);
    }

    static void call(...) {}
};

#define DEFINE_DISPATCH_TO_MEMBER(Member)                                                                          \
                                                                                                                   \
template<typename TList>                                                                                           \
class Member##_dispatcher                                                                                          \
{                                                                                                                  \
                                                                                                                   \
private:                                                                                                           \
    DEFINE_HAS_MEMBER(Member);                                                                                     \
    DEFINE_RETURN_TYPES_FROM_MEMBER(Member);                                                                       \
                                                                                                                   \
    using return_##Member##_variant = infra::meta::tl::to_variant_t<infra::meta::tl::push_front_t<                 \
                                                   infra::meta::tl::remove_duplicates_t<                           \
                                                   infra::meta::tl::erase_t<return_types_from_##Member##_t<        \
                                                   infra::meta::tl::filter_t<                                      \
                                                   has_member_##Member, TList, true>>, void>>, std::monostate>>;   \
                                                                                                                   \
    struct select_##Member##_overload                                                                              \
    {                                                                                                              \
        template<typename T, typename... Args,                                                                     \
                 typename = decltype(std::declval<T>().Member(std::declval<Args&&>()...)) >                        \
        static void call(T && object, return_##Member##_variant & result, Args&&... args){                         \
            using return_type = std::decay_t<decltype(std::declval<T>().Member(std::declval<Args&&>()...))>;       \
            if constexpr (infra::meta::tl::contains_v<return_##Member##_variant, return_type>) {                   \
                 result = std::forward<T>(object).Member(std::forward<Args>(args)...);                             \
            }                                                                                                      \
            else {                                                                                                 \
                 std::forward<T>(object).Member(std::forward<Args>(args)...);                                      \
            }                                                                                                      \
        }                                                                                                          \
                                                                                                                   \
        template<typename T, typename... Args,                                                                     \
                 typename = decltype(std::declval<T>().Member(std::declval<Args&&>()...)) >                        \
        static decltype(auto) call(T && object, Args&&... args) {                                                  \
            return std::forward<T>(object).Member(std::forward<Args>(args)...);                                    \
        }                                                                                                          \
                                                                                                                   \
        template<typename...>                                                                                      \
        static void call(...) {}                                                                                   \
    };                                                                                                             \
                                                                                                                   \
public:                                                                                                            \
    template<typename... Args>                                                                                     \
    static return_##Member##_variant call(std::size_t tag, Args&&... args)                                         \
    {                                                                                                              \
        return_##Member##_variant result;                                                                          \
        auto cb = [&result, &args...](auto && object){                                                             \
            select_##Member##_overload::call(std::move(object), result, std::forward<Args>(args)...);              \
        };                                                                                                         \
        infra::meta::dispatch::dispatch<TList>(tag, cb);                                                           \
        return result;                                                                                             \
    }                                                                                                              \
                                                                                                                   \
    template<bool IsConst, typename... Args>                                                                       \
    static return_##Member##_variant call(std::size_t tag, infra::meta::dispatch::object_wrapper<IsConst> wrapper, \
                                          Args&&... args)                                                          \
    {                                                                                                              \
        return_##Member##_variant result;                                                                          \
        auto cb = [&result, &args...](auto & object){                                                              \
            select_##Member##_overload::call(object, result, std::forward<Args>(args)...);                         \
        };                                                                                                         \
        infra::meta::dispatch::dispatch<TList>(tag, cb, wrapper);                                                  \
        return result;                                                                                             \
    }                                                                                                              \
}//; intentionally skipped


} //dispatch
} //meta
} //infra

#endif 
