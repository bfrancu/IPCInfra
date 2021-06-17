#ifndef SERVER_CALLBACK_POLICY_HPP
#define SERVER_CALLBACK_POLICY_HPP

#include <sys/types.h>
#include <iostream>

#include "Host.hpp"
#include "utilities.hpp"
#include "Devices/DeviceDefinitions.h"
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
                           std::enable_if_t<IsPassiveConnectableDevice<Device>::value && traits::is_endpoint_storage_v<Storage>>>
{
   using DeviceAddress = typename Device::address_type;
   using RawDevice = typename UnpackHost<Device>::ClientT;
   using Handle = typename handler_traits<RawDevice>::handle_type;
   using ClientEndpoint = typename Storage::endpoint_t;
   using ClientKey = typename Storage::key_t;

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

   //TODO Find a way to expose the client descriptor to generate a client key from it
   // some friend function dependency injection - Maybe?
   // https://accu.org/journals/overload/28/156/harrison_2776/
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
   ssize_t send(Args&&... args)
   {
   }

    ~ServerCallbackPolicy() = default;

protected:
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
       handle_access_key.template retrieve<RawDevice>(device, accessor);

       auto key = ClientKeyGenerator::generateKey(client_handle);
       if (meta::traits::default_value<Handle>::value == client_handle)
       {
           p_endpoint->disconnect();
           return false;
       }

       events_array events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
       if (p_endpoint->listenerSubscribe(events))
       {
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

       auto & endpoint_ptr_ref = storage.getEndpointFor(key);
       endpoint_ptr_ref->registerDisconnectionCallback([this, &client_key = std::as_const(key)](){
                   ProcessClientDisconnection(client_key); });

       endpoint_ptr_ref->registerInputCallback([this, &client_key = std::as_const(key)](std::string_view data){
                   ProcessClientInput(client_key, data); });

       this->asDerived().template onClientConnected<Storage>(key);
   }

   bool ProcessInputEvent() {
       std::cout << "ServerCallbackPolicy::ProcessInputEvent()\n";
       return true;
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
       this->asDerived().template OnClientDisconnected<Storage>(key);
       auto & storage = this->asDerived().getStorage();
       storage.erase(key);
   }

protected:

    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }
};


}//infra
#endif //SERVER_CALLBACK_POLICY_HPP
