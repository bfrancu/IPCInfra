#include <stdint.h>
#include <cstring>

#include <algorithm>
#include <iostream>

#include "UnixSocketAddress.h"

namespace
{
constexpr uint8_t UNIX_SOCKET_PATH_MAX_LEN{108};
}

namespace infra
{

namespace unx
{

void UnixAddress::setPathname(std::string_view path)
{
    pathname.assign(path.data(), path.size());
    std::cout << "UnixAddress::setPathname: " << pathname << "\n";
}
UnixSocketAddress::UnixSocketAddress() :
    m_address{},
    m_p_unx_addr{std::make_unique<sockaddr_un>()}
{
    memset(m_p_unx_addr.get(), 0, getAddressLength());
    m_p_unx_addr->sun_family = PF_UNIX;
}

UnixSocketAddress::UnixSocketAddress(const UnixSocketAddress &other) :
    m_address{other.m_address},
    m_p_unx_addr{std::make_unique<sockaddr_un>()}
{
    memset(m_p_unx_addr.get(), 0, getAddressLength());
    m_p_unx_addr->sun_family = PF_UNIX;
    std::copy(other.m_p_unx_addr->sun_path,
              other.m_p_unx_addr->sun_path + UNIX_SOCKET_PATH_MAX_LEN,
              m_p_unx_addr->sun_path);
}

UnixSocketAddress &UnixSocketAddress::operator=(const UnixSocketAddress &other)
{
    if (this == &other){
        return *this;
    }

    m_address = other.m_address;
    memset(m_p_unx_addr->sun_path, 0, UNIX_SOCKET_PATH_MAX_LEN);
    auto other_unix_path_len = strnlen(other.m_p_unx_addr->sun_path,
                                 static_cast<unsigned long>(UNIX_SOCKET_PATH_MAX_LEN));

    std::copy(other.m_p_unx_addr->sun_path,
              other.m_p_unx_addr->sun_path + other_unix_path_len,
              m_p_unx_addr->sun_path);

    return *this;
}

bool UnixSocketAddress::fromString(std::string_view addr)
{
    UnixAddress unx_addr;
    unx_addr.setPathname(addr);
    return setAddress(unx_addr);
}

bool UnixSocketAddress::setAddress(UnixAddress addr)
{
    std::cout << "UnixSocketAddress::setAddress() : " << addr << "\n";
    if (addr.pathname.empty()) return false;

    m_address = std::move(addr);
    memset(m_p_unx_addr->sun_path, 0, UNIX_SOCKET_PATH_MAX_LEN);
    std::copy(m_address.pathname.begin(), m_address.pathname.end(), m_p_unx_addr->sun_path);
    return true;
}

void UnixSocketAddress::setAddress(const sockaddr &sock_addr)
{
    const sockaddr_un *p_remote_unix_sock_addr = reinterpret_cast<const sockaddr_un*>(&sock_addr);
    memset(m_p_unx_addr->sun_path, 0, UNIX_SOCKET_PATH_MAX_LEN);
    auto unix_path_len = strnlen(p_remote_unix_sock_addr->sun_path, static_cast<unsigned long>(UNIX_SOCKET_PATH_MAX_LEN));

    std::copy(p_remote_unix_sock_addr->sun_path,
              p_remote_unix_sock_addr->sun_path + unix_path_len,
              m_p_unx_addr->sun_path);
    m_address.pathname = m_p_unx_addr->sun_path;
}

std::string UnixSocketAddress::toString() const
{
    return m_address.pathname;
}

void UnixSocketAddress::getAddress(UnixSocketAddress::address_type &out_addr) const
{
    std::cout << "UnixSocketAddress::getAddress(ddress_type &)\n";
    //memset(&out_addr, 0, getAddressLength());
    out_addr.sun_family = PF_UNIX;
    std::copy(m_p_unx_addr->sun_path,
              m_p_unx_addr->sun_path + UNIX_SOCKET_PATH_MAX_LEN,
              out_addr.sun_path);
}

void UnixSocketAddress::getAddress(sockaddr &out_addr) const
{
    std::cout << "UnixSocketAddress::getAddress(sockaddr &)\n";
    sockaddr_un * p_out_un_addr = reinterpret_cast<sockaddr_un*>(&out_addr);
    p_out_un_addr->sun_family = PF_UNIX;
    //auto unix_path_len = strnlen(m_p_unx_addr->sun_path,
    //                             static_cast<unsigned long>(UNIX_SOCKET_PATH_MAX_LEN));

    std::copy(m_p_unx_addr->sun_path,
              m_p_unx_addr->sun_path + UNIX_SOCKET_PATH_MAX_LEN,
              p_out_un_addr->sun_path);
    std::cout << "UnixSocketAddress::getAddress(sockaddr &) end\n";
}

} //unx
} //infra
