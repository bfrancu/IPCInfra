#ifndef TRANSPORT_DEFINITIONS_H
#define TRANSPORT_DEFINITIONS_H

#include "traits_utils.hpp"

namespace infra
{
namespace def
{
DEFINE_HAS_MEMBER(getDevice);
DEFINE_HAS_MEMBER(connect);
DEFINE_HAS_MEMBER(bind);
DEFINE_HAS_MEMBER(listen);
DEFINE_HAS_MEMBER(accept);
DEFINE_HAS_MEMBER(disconnect);
DEFINE_HAS_MEMBER(send);
DEFINE_HAS_MEMBER(write);
DEFINE_HAS_MEMBER(read);
DEFINE_HAS_MEMBER(toString);
DEFINE_HAS_TYPE(Device);
DEFINE_MEMBER_TYPE_OR_DEFAULT(EndpointStorage);
DEFINE_MEMBER_TYPE_OR_DEFAULT(EndpointStorageKey);
DEFINE_MEMBER_TYPE_OR_DEFAULT(StateChangeCallbackDispatcher);
DEFINE_MEMBER_TYPE_OR_DEFAULT(EventHandlingPolicy);
DEFINE_MEMBER_TYPE_OR_DEFAULT(DispatcherPolicy);
DEFINE_HAS_MEMBER(store);
DEFINE_HAS_MEMBER(erase);
DEFINE_HAS_TYPE(endpoint_t);
DEFINE_HAS_TYPE(key_t);
DEFINE_HAS_TYPE(PeerTraits);
DEFINE_HAS_TYPE(EndpointStoragePolicy);
}//def
}//infa

#endif
