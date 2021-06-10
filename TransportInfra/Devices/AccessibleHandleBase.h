#ifndef ACCESSIBLEHANDLEBASE_H
#define ACCESSIBLEHANDLEBASE_H
namespace infra
{

template<typename Derived>
class AccessibleHandleBase;

template<template<typename, typename...> typename Derived,
         typename ResourceHandler, typename... Ts>
class AccessibleHandleBase<Derived<ResourceHandler, Ts...>>
{
    using derived_type = Derived<ResourceHandler, Ts...>;

    class HandleKey
    {
        template<typename T> friend class AccessKey;
        HandleKey() {}
        HandleKey(const HandleKey &) {}
    };

public:
    inline decltype (auto) getHandleRestricted(HandleKey) const {
        return static_cast<const derived_type &>(*this).getHandle();
    }
};

template<typename AccessorFunc>
class AccessKey
{
public:
    template<typename Device, typename = decltype(&Device::getHandleRestricted)>
    static void retrieve(const Device & dev, AccessorFunc & accessor)
    {
        auto handle = dev.getHandleRestricted({});
        accessor(handle);
    }

    template<typename>
    static void retrieve(...)
    {}

};

}//infra

#endif // ACCESSIBLEHANDLEBASE_H
