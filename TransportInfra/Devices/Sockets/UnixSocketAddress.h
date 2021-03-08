#ifndef UNIXSOCKETADDRESS_H
#define UNIXSOCKETADDRESS_H
#include <sys/un.h>
#include <sys/socket.h>

#include <string>
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

    bool setAddress(UnixAddress addr);
    void setAddress(const sockaddr &sock_addr);
    void getAddress(address_type & out_addr) const;
    void getAddress(sockaddr & out_addr) const;
    inline bool empty() const { return m_address.pathname.empty(); }
    //UnixAddress getDomainSpecificAddress() const {return m_address};

    inline static size_t getAddressLength() {
        static constexpr auto size{sizeof(sockaddr_un)};
        return size;
    }

private:
    UnixAddress m_address;
    std::unique_ptr<address_type> m_p_unx_addr;
};

} //unx
} //infra

#endif // UNIXSOCKETADDRESS_H
