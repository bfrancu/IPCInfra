#include <cstdint>
#include <iostream>
#include <type_traits>
#include "template_typelist.hpp"
#include "typelist.hpp"
#include "runtime_dispatcher.hpp"

#include "meta.h"

namespace infra
{
namespace meta
{
namespace ttl
{

template<typename> struct SingleTypeHolder{};
template<typename, typename> struct TwoTypesHolder{};
template<typename, typename, typename> struct ThreeTypesHolder{};
template<typename, typename, typename, typename> struct FourTypesHolder{};
template<typename, typename, typename, typename, typename> struct FiveTypesHolder{};

using first_ttlist = template_typelist<TwoTypesHolder, ThreeTypesHolder>;
using second_ttlist = template_typelist<FourTypesHolder, FiveTypesHolder>;
using third_ttlist = template_typelist<ThreeTypesHolder>;
using fourth_ttlist = template_typelist<ThreeTypesHolder, SingleTypeHolder>;
using fifth_ttlist = template_typelist<TwoTypesHolder, FourTypesHolder, TwoTypesHolder, FiveTypesHolder>;
using sixth_ttlist = template_typelist<SingleTypeHolder>;
using concat_ttlist = template_typelist<TwoTypesHolder, ThreeTypesHolder, FourTypesHolder, FiveTypesHolder, SingleTypeHolder>;
using duplicates_ttlist = template_typelist<SingleTypeHolder, TwoTypesHolder, SingleTypeHolder, ThreeTypesHolder, ThreeTypesHolder, TwoTypesHolder,
                                            SingleTypeHolder, FourTypesHolder, FiveTypesHolder, FourTypesHolder, TwoTypesHolder, FiveTypesHolder>;
using singles_ttlist = template_typelist<SingleTypeHolder, TwoTypesHolder, ThreeTypesHolder, FourTypesHolder, FiveTypesHolder>;
using first_tuple = std::tuple<pack<TwoTypesHolder>, pack<ThreeTypesHolder>>;

template<template<typename...> typename Holder>
struct IsTwoTypesHolder : std::is_same<pack<TwoTypesHolder>, pack<Holder>>
{};

using filtered_ttlist = filter_t<IsTwoTypesHolder, fifth_ttlist, false>;

static_assert(is_empty<template_typelist<>>::value);
static_assert(std::is_same_v<front_tt<template_typelist<TwoTypesHolder>, int, double>, TwoTypesHolder<int, double>>);
static_assert(std::is_same_v<pop_front_t<first_ttlist>, template_typelist<ThreeTypesHolder>>);
static_assert(std::is_same_v<push_front_t<third_ttlist, TwoTypesHolder>, first_ttlist>);
static_assert(std::is_same_v<push_back_t<third_ttlist, SingleTypeHolder>, fourth_ttlist>);
static_assert(std::is_same_v<erase_rec_t<fifth_ttlist, TwoTypesHolder>, second_ttlist>);
static_assert(std::is_same_v<to_tuple_t<first_ttlist>, first_tuple>);
static_assert(std::is_same_v<to_ttlist_t<first_tuple>, first_ttlist>);
static_assert(std::is_same_v<concat_t<first_ttlist, second_ttlist, sixth_ttlist>, concat_ttlist>);
static_assert(std::is_same_v<filtered_ttlist, second_ttlist>);
static_assert(std::is_same_v<erase_t<fifth_ttlist, TwoTypesHolder>, second_ttlist>);
static_assert(std::is_same_v<remove_duplicates_t<duplicates_ttlist>, singles_ttlist>);
static_assert(std::is_same_v<nth_element_tt<singles_ttlist, 0, unsigned int>, SingleTypeHolder<unsigned int>>);
static_assert(std::is_same_v<nth_element_tt<singles_ttlist, 2, char, long, unsigned int>, ThreeTypesHolder<char, long, unsigned int>>);
static_assert(std::is_same_v<typename nth_element<singles_ttlist, 3>::packed_type, pack<FourTypesHolder>>); 
static_assert(any_of_v<fifth_ttlist, IsTwoTypesHolder> == true);
static_assert(any_of_v<fourth_ttlist, IsTwoTypesHolder> == false);
static_assert(get_index_by_type_v<concat_ttlist, FourTypesHolder> == 2);
static_assert(size_v<fifth_ttlist> == 4);
} //ttl

namespace tl
{
using extra_integers_tlist = typelist<bool, int, signed char, int, long, int, long long>;
using integrals_tlist_sans_int = typelist<bool, signed char, long, long long>;
using floating_tlist = typelist<float, double>;
using floating_tuple = std::tuple<float, double>;
using concat_tlist = typelist<bool, signed char, long, long long, float, double>;

using filtered_tlist = filter_t<std::is_floating_point, concat_tlist, true>; 
using no_dup_int_tlist = typelist<bool, int, signed char, long, long long>;

static_assert(is_empty_v<typelist<>>);
static_assert(std::is_same_v<front_t<typelist<int, double, char>>, int>);
static_assert(std::is_same_v<pop_front_t<typelist<char, float, unsigned int>>, typelist<float, unsigned int>>);
static_assert(std::is_same_v<push_front_t<typelist<char, float, int>, double>, typelist<double, char, float, int>>);
static_assert(std::is_same_v<push_back_t<typelist<float, long>, char>, typelist<float, long, char>>);
static_assert(std::is_same_v<erase_rec_t<extra_integers_tlist, int>, integrals_tlist_sans_int>);
static_assert(std::is_same_v<to_tuple_t<floating_tlist>, floating_tuple>);
static_assert(std::is_same_v<to_tlist_t<floating_tuple>, floating_tlist>);
static_assert(std::is_same_v<concat_t<integrals_tlist_sans_int, floating_tlist>, concat_tlist>);
static_assert(std::is_same_v<filtered_tlist, floating_tlist>);
static_assert(std::is_same_v<erase_t<extra_integers_tlist, int>, integrals_tlist_sans_int>);
static_assert(std::is_same_v<remove_duplicates_t<extra_integers_tlist>, no_dup_int_tlist>);
static_assert(std::is_same_v<nth_element_t<concat_tlist, 3>, long long>);
static_assert(size_v<concat_tlist> == 6);
} //tl

namespace dispatch
{

struct Tester
{
    int value{0};

    Tester() = default;
    Tester(const Tester &) { std::cout << "Tester(const Tester&)\n"; }

    double test(double d) const{
        std::cout << "double Tester::test(double) instance specific value: " << value << "\n";
        return d; }

    void test(const char *pbuf){
        std::cout << "void Tester::test(const char *) " << pbuf << "\n";
    }

    static uint8_t test() {
        std::cout << "static uint8_t Tester::test()\n";
        return 0;}
};

struct Tester1
{
    Tester1() {std::cout << "Tester1::Tester1()\n";}
    std::string test(int i){
        std::cout << "std::string Tester1::test(int)\n";
        return std::string{};
    }
};

struct Tester2
{
    long long test(double b = 1.0) {
        std::cout << "long long Tester2::test(double)\n";
        return b; 
    }
};

struct Tester3
{
    static uint16_t test() { 
        std::cout << "uint16_t Tester3::test()\n";
        return 0; 
    }
};

struct Tester4
{
    void test() && { std::cout << "void Tester4::test() && \n"; }
};

enum TesterEnum : std::size_t
{
    T_DEFAULT = 0,
    T_1,
    T_2,
    T_3,
    T_4
};

using test_tlist = tl::typelist<Tester, Tester1, Tester2, Tester3, Tester4>;
//using test_tlist = tl::typelist<Tester>;
static_assert(std::is_same_v<Tester, tl::nth_element_t<test_tlist, 0>>);

struct select_test_overload
{
    template<typename T, typename... Args, 
             typename = decltype(std::declval<T>().test(std::declval<Args&&>()...)) >
    static decltype(auto) call(T && object, Args&&... args) {
        return std::forward<T>(object).test(std::forward<Args>(args)...);
    }

    template<typename...>
    static void call(...) {}
};
 
/*
template<typename... Args>
void call_test(std::size_t tag, Args&&... args)
{
    auto cb = [](auto tester) {
        using tester_t = std::decay_t<decltype(tester)>;
        delegate_call<&tester_t::test>::call(tester);
    };
    dispatch<test_tlist>(tag, cb);
}
*/

template<typename... Args>
void call_test(std::size_t tag, Args&&... args)
{
    auto cb = [&args...](auto && tester){
        select_test_overload::call(std::move(tester), std::forward<Args>(args)...);
    };
    dispatch<test_tlist>(tag, cb);
}

template<bool IsConst, typename... Args>
void call_test(std::size_t tag, object_wrapper<IsConst> wrapper, Args&&... args)
{
    auto cb = [&args...](auto & tester){
        select_test_overload::call(tester, std::forward<Args>(args)...);
    };
    dispatch<test_tlist>(tag, cb, wrapper);
}

void dispatch_main()
{
    Tester tst;
    tst.value = 199;
    Tester *p_test = new Tester{};
    p_test->value = 556;
    //auto callable = [](auto && tester) { std::forward<decltype(tester)>(tester).test(); };
    object_wrapper<true> apply_const;
    object_wrapper<false> apply_non_const;
    apply_const.object = reinterpret_cast<const void*>(p_test);
    apply_non_const.object = reinterpret_cast<void*>(p_test);
    auto & const_ref = *(reinterpret_cast<cast_object_t<Tester, object_wrapper<true>>>(apply_const.object));
    auto & const_ref2 = extractValue<Tester>(apply_const);
    auto & non_const_ref = extractValue<Tester>(apply_non_const);
    //select_overload::call(non_const_ref, "gogogo");
    const char *p_msg{"heihei"};
    auto callable = [&p_msg](auto & tester) { tester.test(2.0); };
    //callable(non_const_ref);
    //callable(const_ref);

    //const_ref.test("hihi");

    static_assert(std::is_same_v<decltype(const_ref), const Tester&>);
    static_assert(std::is_same_v<decltype(const_ref2), const Tester&>);
    static_assert(std::is_same_v<decltype(non_const_ref), Tester&>);
    static_assert(std::is_same_v<std::add_const_t<Tester>*, const Tester *>);
    static_assert(std::is_same_v<cast_object_t<Tester, object_wrapper<true>>, const Tester*>);
    static_assert(std::is_same_v<cast_object_t<Tester, object_wrapper<false>>, Tester*>);
    //static_assert(std::is_same_v<const Tester &, >);
    //dispatch_helper<test_tlist, 0>::dispatch(0, callable);
    ///dispatch_helper<test_tlist, 0>::dispatch(0, callable, apply_const);
    //dispatch_helper<test_tlist, 0>::dispatch(0, callable, apply_non_const);
    ///dispatch<test_tlist>(TesterEnum::T_DEFAULT, callable);
    //dispatch<test_tlist>(0, callable, apply_const);
    //dispatch<test_tlist>(0, callable, apply_non_const);

    //call_test(TesterEnum::T_DEFAULT);
    //call_test(TesterEnum::T_DEFAULT, 3.2);
    //call_test(TesterEnum::T_DEFAULT, apply_const);
    //call_test(TesterEnum::T_DEFAULT, apply_const, 5.3);
    //call_test(TesterEnum::T_DEFAULT, apply_non_const, 2.0);
    call_test(TesterEnum::T_DEFAULT, apply_non_const, "bau");
    call_test(TesterEnum::T_1, 2);
    call_test(TesterEnum::T_2);
    call_test(TesterEnum::T_1, "ccc");
    call_test(TesterEnum::T_3);
    call_test(TesterEnum::T_4);
    call_test(TesterEnum::T_4, 2);
    //delegate_call<&Tester3::test>::call(Tester3{});
    //delegate_call<&Tester::test>::call(tst, 2.0);
    //delegate_call2<p_test> delegator;

    //select_overload::call(tst, 2.0);
    //select_overload::call(tst);
    delete p_test;
}

} //dispatch

} //meta
} //infra

