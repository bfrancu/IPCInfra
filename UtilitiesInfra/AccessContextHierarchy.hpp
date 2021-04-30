#ifndef ACCESSCONTEXTHIERARCHY_HPP
#define ACCESSCONTEXTHIERARCHY_HPP

template<typename Client,
         template<typename... > typename... Plugins>
struct AccessContextHierarchy;

template<typename Client>
struct AccessContextHierarchy<Client> : public Client
{};

template<typename Client,
         template<typename... > typename Plugin,
         template<typename... > typename... Plugins>
struct AccessContextHierarchy<Client,
                              Plugin,
                              Plugins...> : public Plugin<AccessContextHierarchy<Client, Plugin, Plugins...>, Client>,
                                             public AccessContextHierarchy<Client, Plugins...>
{
    friend class Plugin<AccessContextHierarchy<Client, Plugin, Plugins...>, Client>;
};

#endif // ACCESSCONTEXTHIERARCHY_HPP
