#ifndef NAMEDPIPEADDRESS_H
#define NAMEDPIPEADDRESS_H
#include <string>

namespace infra
{

struct NamedPipeAddress
{
    using address_type = std::string;
    
    NamedPipeAddress() = default;
    NamedPipeAddress(address_type addr) : pathname{std::move(addr)} {}
    friend bool operator==(const NamedPipeAddress & first, const NamedPipeAddress & second){
        return first.pathname == second.pathname;
    }
    friend bool operator<(const NamedPipeAddress & first, const NamedPipeAddress & second){
        return first.pathname < second.pathname;
    }

    bool setAddress(address_type addr) {
        pathname = std::move(addr);
        return true;
    }

    void getAddress(address_type & out_addr) const
    {
        out_addr = pathname;
    }

    address_type getAddress() const { return pathname; }
    bool empty() const { return pathname.empty(); }

    address_type pathname;
};


} // infra

#endif
