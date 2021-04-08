#include "runtime_dispatcher.hpp"

namespace infra
{
namespace meta
{
namespace dispatch
{

object_wrapper<false> wrap(void *value)
{
    object_wrapper<false> res;
    res.object = value;
    return res;
}

object_wrapper<true> wrap_const(const void *value)
{
    object_wrapper<true> res;
    res.object = value;
    return res;
}
}//dispatch
}//meta
}//infra
