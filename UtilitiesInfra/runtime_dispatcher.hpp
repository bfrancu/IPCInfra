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

} //dispatch

} //meta
} //infra

#endif 
