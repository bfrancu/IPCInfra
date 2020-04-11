#ifndef SYS_CALL_EVAL_H
#define SYS_CALL_EVAL_H
#include <utility>
#include <type_traits>
#include <functional>

namespace infra
{

template <typename Sys_callable, typename... Args>
std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Sys_callable, Args...>, int>, bool>
sys_call_zero_eval(Sys_callable f, Args&&...args){
    return (0 ==  f(std::forward<Args>(args)...));
}

template <typename Sys_callable, typename... Args>
std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Sys_callable, Args...>, int>, bool>
sys_call_noerr_eval(Sys_callable f, Args&&...args){
    return (-1 !=  f(std::forward<Args>(args)...));
}

template <typename Sys_callable, typename... Args>
std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Sys_callable, Args...>, int>, bool>
sys_call_nonull_eval(Sys_callable f, Args&&...args){
    return (nullptr !=  f(std::forward<Args>(args)...));
}

/*template <typename Sys_callable, typename... Args>
std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Sys_callable, Args...>, int>, bool>
sys_call(Sys_callable f, Args&&...args){
    return (nullptr !=  f(std::forward<Args>(args)...));
}*/


template <typename Comp, typename T,
          typename Callable,  typename... Args>
std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Callable, Args...>, T>, bool>
callable_eval(Comp comparator, T evaluated_value, Callable f, Args&&... args){
    return(comparator(evaluated_value, f(std::forward<Args>(args)...)));
}





}

#endif // SYS_CALL_EVAL_H
