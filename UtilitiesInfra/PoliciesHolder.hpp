#ifndef POLICIESHOLDER_HPP
#define POLICIESHOLDER_HPP
#include "Host.hpp"

namespace infra
{
   template<template<typename... > typename... Policies>
   struct PoliciesHolder
   {
       //https://stackoverflow.com/questions/45288396/c-11-templates-alias-for-a-parameter-pack
       template<typename Client> 
       using AssembledClientT = Host<Client, Policies...>;
   };

   template<template<typename... > typename... Policies>
   struct TemplateTypeList
   {};

   template<typename TList, typename Client>
   struct AssembleClient;

   template<template<typename...> typename... Policies,
            typename Client>
   struct AssembleClient<TemplateTypeList<Policies...>, Client>
   {
       using AssembledClientT = Host<Client, Policies...>;
   };
} //infra
#endif
