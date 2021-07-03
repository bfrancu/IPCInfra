#ifndef SERVER_CALLBACK_POLICY_HPP
#define SERVER_CALLBACK_POLICY_HPP

#include <sys/types.h>
#include <iostream>

#include "Host.hpp"
#include "utilities.hpp"
#include "Devices/DeviceDefinitions.h"
#include "TransportDefinitions.h"
#include "Traits/fifo_traits.hpp"
#include "Traits/utilities_traits.hpp"
#include "Traits/storage_traits.hpp"
#include "Devices/AccessibleHandleBase.h"
#include "default_traits.hpp"
#include "Reactor/EventTypes.h"

namespace infra
{

// to be used with EndpointStorage associtive methods
struct BasicDescriptorKeyGenerator
{
    static int generateKey(int descriptor) { return descriptor; }
};

template<typename Derived, typename Device, typename Storage,
         typename ClientKeyGenerator = BasicDescriptorKeyGenerator, typename = void>
class ServerCallbackPolicy
{};

template<typename Derived, typename Device, typename Storage, typename ClientKeyGenerator>
class ServerCallbackPolicy<Derived, Device, Storage, ClientKeyGenerator,
                           std::enable_if_t<IsPassiveConnectableDevice<Device>::value &&
                                            traits::is_endpoint_storage_v<Storage>
                                            && IsNamedPipeDeviceT<Device>::value
                                            //&& def::has_member_read<Device>::value
                                            >>
{
   using DeviceAddress = typename Device::address_type;
   using RawDevice = typename UnpackHost<Device>::ClientT;
   using Handle = typename handler_traits<RawDevice>::handle_type;
   using ClientEndpoint = typename Storage::endpoint_t;
   using ClientKey = typename Storage::key_t;
   using ClientEndpointDevice = typename ClientEndpoint::Device;

public:
   bool bind(const DeviceAddress & addr, bool non_blocking = true)
   {
       std::cout << "ServerCallbackPolicy::bind()\n";
       Device & device = this->asDerived().getDevice();
       bool result = device.bind(addr, non_blocking);
       if (result){
           this->asDerived().setState(EConnectionState::E_BINDED);
       }
       return result;
   }

   bool listen(int backlog = 50)
   {
       std::cout << "ServerCallbackPolicy::listen()\n";
       Device & device = this->asDerived().getDevice();
       bool result = device.listen(backlog);
       if (result){
           this->asDerived().setState(EConnectionState::E_LISTENING);
       }
       return result;
   }

   bool accept(bool non_blocking) {
       std::cout << "ServerCallbackPolicy::accept()\n";
       Device & device = this->asDerived().getDevice();
       device.accept(non_blocking);
       updateDescriptorKey();

       return ProcessInputEvent();
   }

   template<typename... Args>
   ssize_t broadcast(Args&&... args)
   {
       return send(std::forward<Args>(args)...);
   }

   template<typename... Args>
   ssize_t send(Args&&... args)
   {
       return sendImpl(this->asDerived().getDevice(), std::forward<Args>(args)...);
   }

protected:
    ssize_t sendImpl(...){ return -1; }

    template<typename Dev, typename... Args,
             typename = decltype(std::declval<std::decay_t<Dev>>().send(std::declval<Args&&>()...))>
    std::enable_if_t<IsSendableDeviceT<Dev>::value, ssize_t>
    sendImpl(Dev & device, Args&&... args)
    {
        return device.send(std::forward<Args>(args)...);
    }

    template<typename Dev, typename... Args,
             typename = void, typename = decltype(std::declval<std::decay_t<Dev>>().write(std::declval<Args&&>()...))>
    std::enable_if_t<(IsWritableDeviceT<Dev>::value && !IsSendableDeviceT<Dev>::value), ssize_t>
    sendImpl(Dev & device, Args&&... args)
    {
        return device.write(std::forward<Args>(args)...);
    }

   bool ProcessInputEvent() {
       if (meta::traits::default_value<ClientKey>::value == m_descriptor_key)
       {
           updateDescriptorKey();
       }

       std::cout << "ServerCallbackPolicy::ProcessInputEvent()\n";
       Device & device = this->asDerived().getDevice();
       ssize_t size_read = readImpl(device);

       if (size_read > 0)
       {
           //std::cout << "ServerCallbackPolicy::ProcessInputEvent(): read from device: " << m_local_buffer << "\n";
           this->asDerived().template onClientInputAvailable<Storage>(m_descriptor_key, m_local_buffer);
           m_local_buffer.clear();
       }

       return true;
   }

   ssize_t readImpl(...) {
       std::cout << "ServerCallbackPolicy::readImpl() no read() in the Device Interface. Make sure to provide an IO Policy for the Pipe device\n";
       return -1; }

   template<typename Dev, typename = std::enable_if_t<def::has_member_read<Dev>::value>>
   ssize_t readImpl(Dev & device)
   {
       std::cout << "ServerCallbackPolicy::readImpl(Device & device)\n";
       return device.read(m_local_buffer);
   }

   bool ProcessOutputEvent() { return true; }

   bool ProcessHangupEvent() {
       std::cout << "ServerCallbackPolicy::ProcessHangupEvent()\n";
       return true;
   }

   bool ProcessDisconnectionEvent() { return true; }
   bool ProcessErrorEvent() { return true; }
protected:
    ~ServerCallbackPolicy() = default;
    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }

    void updateDescriptorKey()
    {
       Handle local_handle{meta::traits::default_value<Handle>::value};
       auto accessor = [&local_handle] (Handle h) { local_handle = h; };
       AccessKey<decltype(accessor)> handle_access_key;
       handle_access_key.retrieve(this->asDerived().getDevice(), accessor);

       m_descriptor_key = ClientKeyGenerator::generateKey(local_handle);
    }

private:
    ClientKey m_descriptor_key{meta::traits::default_value<ClientKey>::value};
    std::string m_local_buffer{};
};

template<typename Derived, typename Device, typename Storage, typename ClientKeyGenerator>
class ServerCallbackPolicy<Derived, Device, Storage, ClientKeyGenerator,
                           std::enable_if_t<IsPassiveConnectableDevice<Device>::value &&
                                            traits::is_endpoint_storage_v<Storage>
                                            && !IsNamedPipeDeviceT<Device>::value
                                            >>
{
   using DeviceAddress = typename Device::address_type;
   using RawDevice = typename UnpackHost<Device>::ClientT;
   using Handle = typename handler_traits<RawDevice>::handle_type;
   using ClientEndpoint = typename Storage::endpoint_t;
   using ClientKey = typename Storage::key_t;
   using ClientEndpointDevice = typename ClientEndpoint::Device;

public:
   bool bind(const DeviceAddress & addr, bool reusable = true)
   {
       std::cout << "ServerCallbackPolicy::bind()\n";
       Device & device = this->asDerived().getDevice();
       bool result = device.bind(addr, reusable);
       if (result){
           this->asDerived().setState(EConnectionState::E_BINDED);
       }
       return result;
   }

   bool listen(int backlog = 50)
   {
       std::cout << "ServerCallbackPolicy::listen()\n";
       Device & device = this->asDerived().getDevice();
       bool result = device.listen(backlog);
       if (result){
           this->asDerived().setState(EConnectionState::E_LISTENING);
       }
       return result;
   }

   bool accept(bool non_blocking)
   {
       std::cout << "ServerCallbackPolicy::accept()\n";
       auto optional_res = acceptImpl(non_blocking);
       if (!optional_res.has_value())
       {
           std::cerr << "ServerCallbackPolicy::accept() no value returned by accept\n";
           return false;
       }

       return setupClientEndpoint((std::move(optional_res)).value());
   }

   template<typename... Args>
   ssize_t broadcast(Args&&... args)
   {
       ssize_t ret{0};
       auto & storage = this->asDerived().getStorage();
       storage.forEach([&ret, this, &args...](auto & p_endpoint){
           ret += sendImpl(p_endpoint->GetDevice(), std::forward<Args>(args)...);
       });
       return ret;
   }

   template<typename... Args>
   ssize_t send(const ClientKey  & key, Args&&... args)
   {
       auto & storage = this->asDerived().getStorage();
       auto endpoint_ptr_ref = storage.getEndpointFor(key);
       if (!endpoint_ptr_ref.has_value())
       {
           return 0;
       }

       auto & p_endpoint = endpoint_ptr_ref.value().get();
       return sendImpl(p_endpoint->getDevice(), std::forward<Args>(args)...);
   }

protected:
    ssize_t sendImpl(...){ std::cout << "wrong sendImpl()\n"; return -1; }

    template<typename Dev, typename... Args,
             typename = decltype(std::declval<std::decay_t<Dev>>().send(std::declval<Args&&>()...))>
    std::enable_if_t<IsSendableDeviceT<Dev>::value, ssize_t>
    sendImpl(Dev & device, Args&&... args)
    {
        return device.send(std::forward<Args>(args)...);
    }

    template<typename Dev, typename... Args,
             typename = void,
             typename = decltype(std::declval<std::decay_t<Dev>>().write(std::declval<Args&&>()...))>
    std::enable_if_t<(IsWritableDeviceT<Dev>::value && !IsSendableDeviceT<Dev>::value), ssize_t>
    sendImpl(Dev & device, Args&&... args)
    {
        return device.write(std::forward<Args>(args)...);
    }

   std::optional<RawDevice> acceptImpl(bool non_blocking)
   {
       std::cout << "ServerCallbackPolicy::acceptImpl()\n";
       Device & device = this->asDerived().getDevice();
       return device.accept(non_blocking);
   }

   bool setupClientEndpoint(RawDevice && device)
   {
       std::cout << "ServerCallbackPolicy::setupClientEndpoint()\n";
       auto & listener = this->asDerived().getListener();
       auto p_endpoint = std::make_unique<ClientEndpoint>(EConnectionState::E_CONNECTED, listener);
       p_endpoint->setDevice(std::move(device));

       Handle client_handle{meta::traits::default_value<Handle>::value};
       auto accessor = [&client_handle] (Handle h) { client_handle = h; };
       AccessKey<decltype(accessor)> handle_access_key;
       handle_access_key.retrieve(p_endpoint->getDevice(), accessor);

       auto key = ClientKeyGenerator::generateKey(client_handle);
       std::cout << "ServerCallbackPolicy::setupClientEndpoint() client key: " << key << "; handle: " << client_handle << "\n";
       if (meta::traits::default_value<Handle>::value == client_handle)
       {
           p_endpoint->disconnect();
           return false;
       }

       events_array events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
       if (p_endpoint->listenerSubscribe(events))
       {
           std::cout << "ServerCallbackPolicy::setupClientEndpoint() client endpoint registered to Listener\n";
           onClientConnected(key, std::move(p_endpoint));
           return true;
       }
       return false;
   }

   void onClientConnected(const ClientKey & key, std::unique_ptr<ClientEndpoint> p_endpoint)
   {
       std::cout << "ServerCallbackPolicy::onClientConnected()\n";
       auto & storage = this->asDerived().getStorage();
       if (!storage.store(key, std::move(p_endpoint)))
       {
           std::cerr << "ServerCallbackPolicy::onClientConnected() couldn't store endpoint by key: " << key << "\n";
           return;
       }

       auto endpoint_ptr_ref = storage.getEndpointFor(key);
       if (!endpoint_ptr_ref.has_value())
       {
           return;
       }

       auto & endpoint_ptr = endpoint_ptr_ref.value().get();
       endpoint_ptr->registerDisconnectionCallback([this, key](){
                   ProcessClientDisconnection(key); });

       endpoint_ptr->registerInputCallback([this, key](std::string_view data){
                   ProcessClientInput(key, data); });

       this->asDerived().template onClientConnected<Storage>(key);
   }

   bool ProcessInputEvent() {
       std::cout << "ServerCallbackPolicy::ProcessInputEvent()\n";
       bool non_blocking{true};
       return accept(non_blocking);
   }
   
   bool ProcessOutputEvent() { return true; }

   bool ProcessHangupEvent() {
       std::cout << "ServerCallbackPolicy::ProcessHangupEvent()\n";
       return true;
   }

   bool ProcessDisconnectionEvent() { return true; }
   bool ProcessErrorEvent() { return true; }

   void ProcessClientInput(const ClientKey & key, std::string_view data)
   {
       std::cout << "ServerCallbackPolicy::ProcessClientInput()\n";
       this->asDerived().template onClientInputAvailable<Storage>(key, data);
   }

   void ProcessClientDisconnection(const ClientKey & key)
   {
       std::cout << "ServerCallbackPolicy::ProcessClientDisconnection()\n";
       this->asDerived().template onClientDisconnected<Storage>(key);
       auto & storage = this->asDerived().getStorage();
       storage.erase(key);
   }

protected:
    ~ServerCallbackPolicy() = default;
    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }
};


}//infra
#endif //SERVER_CALLBACK_POLICY_HPP
