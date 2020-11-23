#ifndef TRAITS_UTILS_HPP
#define TRAITS_UTILS_HPP

#define DEFINE_HAS_TYPE(MemType)                                      \
  template<typename T, typename = void>                               \
  struct HasTypeT_##MemType                                           \ 
    : std::false_type {};                                             \ 
                                                                      \
  template<typename T>                                                \
  struct HasTypeT_##MemType<T, std::void_t<typename T::MemType>>      \
   : std::true_type {} // ; intentionally skipped          

#define DEFINE_HAS_MEMBER(Member)                                     \ 
  template<typename T, typename = void>                               \
  struct HasMemberT_##Member                                          \
    : std::false_type {};                                             \ 
                                                                      \
  template<typename T>                                                \ 
  struct HasMemberT_##Member<T, std::void_t<decltype(&T::Member)>>    \
    : std::true_type {} // ; intentionally skipped 

#endif
