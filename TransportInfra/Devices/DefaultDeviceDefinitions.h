#ifndef DEFAULT_DEVICE_DEFINITIONS_H
#define DEFAULT_DEVICE_DEFINITIONS_H
#include "DeviceFactory.hpp"
#include "FileDevice.hpp"

namespace infra
{

namespace defaults
{
    using IPV4TcpSocketDevice    = typename Device2Type<ipv4_strm_tag>::template device_type<UnixResourceHandler>;
    using IPV4UdpSocketDevice    = typename Device2Type<ipv4_dgram_tag>::template device_type<UnixResourceHandler>;
    using IPV6TcpSocketDevice    = typename Device2Type<ipv6_strm_tag>::template device_type<UnixResourceHandler>;
    using IPV6UdpSocketDevice    = typename Device2Type<ipv6_dgram_tag>::template device_type<UnixResourceHandler>;
    using UnixStreamSocketDevice = typename Device2Type<unx_strm_tag>::template device_type<UnixResourceHandler>;
    using UnixDgramSocketDevice  = typename Device2Type<unx_dgram_tag>::template device_type<UnixResourceHandler>;
    using ReadFifoDevice         = typename Device2Type<read_fifo_tag>::template device_type<UnixResourceHandler>;
    using WriteFifoDevice        = typename Device2Type<write_fifo_tag>::template device_type<UnixResourceHandler>;
    using UnixFileDevice         = FileDevice<UnixResourceHandler>;
}//defaults

} //infra

#endif
