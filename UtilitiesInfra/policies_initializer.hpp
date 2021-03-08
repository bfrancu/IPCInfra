#ifndef POLICIES_INITIALIZER_HPP
#define POLICIES_INITIALIZER_HPP
#include "typelist.hpp"

namespace infra
{

template<typename Subject, typename... Args>
bool invokeInit(Subject && subject, Args&&... args)
{
    if constexpr (std::is_invocable_v<decltype(&Subject::init), Subject, Args...>){
        return std::forward<Subject>(subject).init(std::forward<Args>(args)...);
    }
    else if constexpr(std::is_invocable_v<decltype(&Subject::init), Subject>) {
        return std::forward<Subject>(subject).init();
    }
    return true;
}

template<typename TList, std::size_t N>
struct init_helper
{
    template<typename Subject, typename... Args>
    static bool dispatch(Subject & subject, Args&&... args)
    {
        bool result{true};
        if constexpr (N < meta::tl::size_v<TList> - 1){
            using object_t = meta::tl::nth_element_t<TList, N>;
            result = invokeInit(static_cast<object_t&>(subject), std::forward<Args>(args)...)
                     && init_helper<TList, N+1>::dispatch(subject, std:::forward<Args>(args)...);
        }
        return result;
    }
};

template<typename TList, typename Subject, typename... Args>
bool initDispatch(Subject & subject, Args&&... args)
{
    return init_helper<TList, 0>::dispatch(subject, std::forward<Args>(args)...);
}

}// infra

#endif
