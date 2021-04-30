#ifndef EXPORTERPOLICY_HPP
#define EXPORTERPOLICY_HPP
#include "template_typelist.hpp"

#include "TransportDefinitions.h"
#include "Devices/ProxyDevice.hpp"

namespace infra
{

template<typename, typename>
class Ancestor{};

template<typename TTList>
struct Exporter;


template<typename T,
         typename HasDeviceType = traits::select_if_t<def::has_type_Device<T>,
                                                      std::true_type,
                                                      std::false_type>>
struct ExposesDevice;

template<typename T>
struct ExposesDevice<T, std::false_type> : std::false_type
{};

template<typename T>
struct ExposesDevice<T, std::true_type> : def::has_member_getDevice<T>
{};


template<template <typename...> typename... Policies>
struct Exporter<meta::ttl::template_typelist<Policies...>>
{
    template<typename Host, typename Derived, typename = void>
    class Policy{};

    /*
    template<typename Host, typename Derived>
    class Policy<Host, Derived, 
                 std::enable_if_t<ExposesDevice<Derived>::value>> : public Policies<Host, Derived>...
    {
        friend class GenericDeviceAccess;
        friend class NamedPipeDeviceAccess;
        friend class SocketDeviceAccess;

        using Device = typename Derived::Device;
        using handle_type = typename Device::handle_type;

    protected:
        void init()
        {
            Device & endpoint_device = static_cast<Derived&>(*this).getDevice();
            m_proxy_dev->setBaseReference(endpoint_device);
        }

        handle_type getHandle(){
            return m_proxy_dev.getHandle();
        } 

    private:
        ProxyDevice<Device> m_proxy_dev;
    };
    */
};

}//infra
#endif
