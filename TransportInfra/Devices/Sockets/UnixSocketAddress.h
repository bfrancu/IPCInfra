#ifndef UNIXSOCKETADDRESS_H
#define UNIXSOCKETADDRESS_H
#include <sys/un.h>
#include <sys/socket.h>

#include <string>
#include <ostream>
#include <memory>

namespace infra
{

namespace unx
{
class UnixSocketAddress;
bool operator<(const UnixSocketAddress & left, const UnixSocketAddress & other);

struct UnixAddress
{
    UnixAddress() = default;
    UnixAddress(std::string path) : pathname{std::move(path)} {}
    std::string pathname;
    friend bool operator==(const UnixAddress & first, const UnixAddress & second){
        return first.pathname == second.pathname;
    }
    friend bool operator<(const UnixAddress & first, const UnixAddress & second){
        return first.pathname < second.pathname;
    }
    
    friend std::ostream & operator<<(std::ostream & os, const UnixAddress & addr)
    {
        os << "UnixAddress: pathame = " << addr.pathname;
        return os;
    }

    void setPathname(std::string_view path);
};

class UnixSocketAddress
{
public:
    using address_type = sockaddr_un;

    UnixSocketAddress();
    UnixSocketAddress(const UnixSocketAddress & other);
    UnixSocketAddress & operator=(const UnixSocketAddress & other);

    UnixSocketAddress(UnixSocketAddress && other) = default;
    UnixSocketAddress & operator=(UnixSocketAddress && other) = default;
    ~UnixSocketAddress() = default;

    bool operator<(UnixSocketAddress & other){
        return m_address < other.m_address;
    }

    bool fromString(std::string_view addr);
    bool setAddress(UnixAddress addr);
    void setAddress(const sockaddr &sock_addr);

    std::string toString() const;
    void getAddress(address_type & out_addr) const;
    void getAddress(sockaddr & out_addr) const;
    inline bool empty() const { return m_address.pathname.empty(); }
    //UnixAddress getDomainSpecificAddress() const {return m_address};

    inline static size_t getAddressLength() {
        static constexpr auto size{sizeof(sockaddr_un)};
        return size;
    }

    friend std::ostream & operator<<(std::ostream & os, const UnixSocketAddress & addr)
    {
        os << "UnixSocketAddress: address = " << addr.m_address;
        return os;
    }


private:
    UnixAddress m_address;
    std::unique_ptr<address_type> m_p_unx_addr;
};

} //unx
} //infra

#endif // UNIXSOCKETADDRESS_H
