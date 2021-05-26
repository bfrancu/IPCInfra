#ifndef TRANSPORT_DEFINITIONS_H
#define TRANSPORT_DEFINITIONS_H

#include "traits_utils.hpp"

namespace infra
{
namespace def
{
DEFINE_HAS_MEMBER(getDevice);
DEFINE_HAS_MEMBER(connect);
DEFINE_HAS_MEMBER(disconnect);
DEFINE_HAS_MEMBER(send);
DEFINE_HAS_MEMBER(write);
DEFINE_HAS_MEMBER(toString);
DEFINE_HAS_TYPE(Device);
DEFINE_MEMBER_TYPE_OR_DEFAULT(EndpointStorage);
DEFINE_MEMBER_TYPE_OR_DEFAULT(StateChangeCallbackDispatcher);
DEFINE_MEMBER_TYPE_OR_DEFAULT(EventHandlingPolicy);
DEFINE_MEMBER_TYPE_OR_DEFAULT(DispatcherPolicy);
}//def
}//infa

#endif
