#ifndef EMPTYDEVICE_HPP
#define EMPTYDEVICE_HPP
#include "Traits/handler_traits.hpp"
#include "default_traits.hpp"

namespace infra
{

template<typename ResourceHandler>
class EmptyDevice
{
   using handle_type = typename handler_traits<ResourceHandler>::handle_type;
   handle_type getHandle() const { return meta::traits::default_value<handle_type>::value; }
};
} //infra


#endif // EMPTYDEVICE_HPP
