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
DEFINE_HAS_TYPE(Device);
}//def
}//infa

#endif
