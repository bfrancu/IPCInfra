#ifndef ACCESSCONTEXTHIERARCHY_HPP
#define ACCESSCONTEXTHIERARCHY_HPP
#include "typelist.hpp"

namespace infra
{

template<typename Client,
         template<typename... > typename... Plugins>
struct AccessContextHierarchy;

template<typename Client>
struct AccessContextHierarchy<Client> : public Client
{
    using ConcretePluginsTList = meta::tl::typelist<>;

   template<typename... Args>
   AccessContextHierarchy(Args&&... args) :
        Client(std::forward<Args>(args)...)
   {}
};

template<typename Client,
         template<typename... > typename Plugin,
         template<typename... > typename... Plugins>
struct AccessContextHierarchy<Client,
                              Plugin,
                              Plugins...> : public Plugin<AccessContextHierarchy<Client, Plugin, Plugins...>, Client>,
                                            public AccessContextHierarchy<Client, Plugins...>
{
    friend class Plugin<AccessContextHierarchy<Client, Plugin, Plugins...>, Client>;
    using ConcretePluginsTList = meta::tl::push_front_t<typename AccessContextHierarchy<Client, Plugins...>::ConcretePluginsTList,
                                                        Plugin<AccessContextHierarchy<Client, Plugin, Plugins...>, Client>>;

    template<typename... Args>
    AccessContextHierarchy(Args&&... args) :
        Plugin<AccessContextHierarchy<Client, Plugin, Plugins...>, Client>(),
        AccessContextHierarchy<Client, Plugins...>(std::forward<Args>(args)...)
    {}
};

}//infra

#endif // ACCESSCONTEXTHIERARCHY_HPP
