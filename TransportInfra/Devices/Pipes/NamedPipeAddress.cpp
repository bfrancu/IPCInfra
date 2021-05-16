#include <iostream>

#include "NamedPipeAddress.h"

namespace infra
{


bool NamedPipeAddress::fromString(std::string_view addr)
{
    pathname.assign(addr.data(), addr.size());
    return true;
}

bool NamedPipeAddress::setAddress(address_type addr)
{
    pathname = std::move(addr);
    return true;
}

std::ostream & operator<<(std::ostream & os, const NamedPipeAddress & addr)
{
    os  << "NamedPipeAddress; pathname = " << addr.pathname << "\n";
    return os;
}

}//infra
