#ifndef EXPORTERPOLICY_HPP
#define EXPORTERPOLICY_HPP
#include "template_typelist.hpp"

#include "TransportDefinitions.h"
#include "AccessContextHierarchy.hpp"
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
    template<typename Host, typename Endpoint, typename = void>
    class Policy{};

    template<typename Host, typename Endpoint>
    //class Policy<Host, Endpoint,  std::enable_if_t<ExposesDevice<Endpoint>::value>> : public Policies<Host, Endpoint>...
    class Policy<Host, Endpoint,  std::enable_if_t<ExposesDevice<Endpoint>::value>>
        : public AccessContextHierarchy<ProxyDevice<typename Endpoint::Device>, Policies...>

    {
        using Device = typename Endpoint::Device;
        using ProxyDeviceBase = ProxyDevice<Device>;
        using handle_type = typename Device::handle_type;

    public:
        bool init()
        {
            //std::cout << "ExporterPolicy::init() id: " << id << "\n";
            Device & endpoint_device = (static_cast<Host&>(*this)).getDevice();
            (static_cast<ProxyDeviceBase&>(*this).setBaseReference(endpoint_device));
            //m_proxy_dev.setBaseReference(endpoint_device);
            return true;
        }

    protected:
        handle_type getHandle(){
            //std::cout << "ExporterPolicy::getHandle() id: " << id << "\n";
            //return m_proxy_dev.getHandle();
            static_cast<const ProxyDeviceBase&>(*this).getHandle();
        } 

    //private:
        //ProxyDevice<Device> m_proxy_dev;
        //int id{getRandomNumberInRange(0, 5000)};
    };
};

}//infra
#endif
