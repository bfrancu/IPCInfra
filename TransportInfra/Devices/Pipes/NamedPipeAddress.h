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

    bool fromString(std::string_view addr);
    bool setAddress(address_type addr) ;

    inline void getAddress(address_type & out_addr) const { out_addr = pathname; }
    inline address_type getAddress() const { return pathname; }
    inline std::string toString() const { return getAddress(); }
    inline bool empty() const { return pathname.empty(); }
    friend std::ostream & operator<<(std::ostream & os, const NamedPipeAddress & addr);

    address_type pathname;
};

} // infra

#endif
