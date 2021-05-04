#ifndef POLICIES_INITIALIZER_HPP
#define POLICIES_INITIALIZER_HPP
//#include <iostream>
#include <type_traits>
#include "typelist.hpp"
#include "traits_utils.hpp"

namespace infra
{

class InitDelegator
{
    DEFINE_HAS_MEMBER(init);
public:
    template<typename Subject, typename... Args>
    static bool invokeInit(Subject && subject, Args&&... args)
    {
        //std::cout << "InitDelegator::invokeInit()\n";
        return invokeInitImpl(has_member_init<std::decay_t<Subject>>{},
                              std::forward<Subject>(subject), std::forward<Args>(args)...);
    }

protected:
    template<typename... Args>
    static bool invokeInitImpl(std::false_type, Args&&...) {
        //std::cout << "InitDelegator::invokeInitImpl(std::false_type) empty init\n";
        return true;
    }

    template<typename Subject, typename... Args>
    static bool invokeInitImpl(std::true_type, Subject && subject, Args&&... args)
    {
        if constexpr (std::is_invocable_v<decltype(&std::decay_t<Subject>::init), Subject, Args...>){
            //std::cout << "InitDelegator::invokeInitImpl(std::true_type) returning Subject.init(args)\n";
            return std::forward<Subject>(subject).init(std::forward<Args>(args)...);
        }
        else if constexpr(std::is_invocable_v<decltype(&std::decay_t<Subject>::init), Subject>) {
            //std::cout << "InitDelegator::invokeInitImpl(std::true_type) returning Subject.init()\n";
            return std::forward<Subject>(subject).init();
        }
        //std::cout << "InitDelegator::invokeInitImpl(std::true_type) returning true\n";
        return true;
    }
};

template<typename TList, std::size_t N>
struct init_helper
{
    template<typename Subject, typename... Args>
    static bool dispatch(Subject & subject, Args&&... args)
    {
        bool result{true};
        //std::cout << "init_helper::dispatch() N: " << N << "\n";
        if constexpr (N < meta::tl::size_v<TList>){
            using object_t = meta::tl::nth_element_t<TList, N>;
            if constexpr (std::is_same_v<meta::tl::empty_type, object_t>){
                //std::cout << "init_helper::dispatch() empty type\n";
                return result;
            }
            else
            {
                result = InitDelegator::invokeInit(static_cast<object_t&>(subject), std::forward<Args>(args)...)
                         && init_helper<TList, N+1>::dispatch(subject, std::forward<Args>(args)...);
            }
        }
        return result;
    }
};

template<typename TList, typename Subject, typename... Args>
bool initDispatch(Subject & subject, Args&&... args)
{
    //std::cout << "initDispatch\n";
    return init_helper<TList, 0>::dispatch(subject, std::forward<Args>(args)...);
}


template<typename TList, std::size_t N>
struct deinit_helper
{
    template<typename T>
    static void invokeDeinit(T&) {}

    template<typename Subject, typename = std::void_t<decltype(&std::decay_t<Subject>::deinit)>>
    static void invokeDeinit(Subject & subject) { subject.deinit(); }

    template<typename Subject>
    static void dispatch(Subject & subject)
    {
        if constexpr (N < meta::tl::size_v<TList> - 1){
            using object_t = meta::tl::nth_element_t<TList, N>;
            if constexpr (std::is_same_v<meta::tl::empty_type, object_t>){
                return;
            }
            else
            {
                object_t & base_subject = static_cast<object_t&>(subject);
                invokeDeinit(base_subject);
                deinit_helper<TList, N+1>::dispatch(subject);
            }
        }
    }
};

template<typename TList, typename Subject>
void deinitDispatcher(Subject & subject)
{
    return deinit_helper<TList, 0>::dispatch(subject);
}

}// infra

#endif
