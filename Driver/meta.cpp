#include <cstdint>
#include <memory>
#include <iostream>
#include <type_traits>
#include <variant>
#include "function_traits.hpp"
#include "pointer_traits.hpp"
#include "template_typelist.hpp"
#include "typelist.hpp"
#include "non_typelist.hpp"
#include "runtime_dispatcher.hpp"
#include "traits_utils.hpp"
#include "Traits/device_constraints.hpp"

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

using intersect_ttlist1 = template_typelist<TwoTypesHolder, FourTypesHolder, ThreeTypesHolder>;
using intersect_ttlist2 = template_typelist<FiveTypesHolder, ThreeTypesHolder, TwoTypesHolder>;
using intersection_ttlist12 = template_typelist<TwoTypesHolder, ThreeTypesHolder>;


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
static_assert(std::is_same_v<intersect_t<intersect_ttlist1, intersect_ttlist2>, intersection_ttlist12>);
} //ttl

namespace tl
{
using extra_integers_tlist = typelist<bool, int, signed char, int, long, int, long long>;
using integrals_tlist_sans_int = typelist<bool, signed char, long, long long>;
using floating_tlist = typelist<float, double>;
using floating_tuple = std::tuple<float, double>;
struct test_struct { using test = int; };
using concat_tlist = typelist<bool, signed char, long, long long, float, double>;

using filtered_tlist = filter_t<std::is_floating_point, concat_tlist, true>; 
using no_dup_int_tlist = typelist<bool, int, signed char, long, long long>;
using custom_tlist =typelist<bool, int, char, test_struct, std::string>;
DEFINE_GET_MEMBER_TYPE_BY_INDEX(test);

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
static_assert(std::is_same_v<to_variant_t<floating_tlist>, std::variant<float, double>>);
static_assert(size_v<concat_tlist> == 6);
static_assert(contains_v<concat_tlist, signed char>);
static_assert(contains_v<floating_tuple, double>);
static_assert(std::is_same_v<nth_element_t<custom_tlist, 3>, test_struct>);
static_assert(std::is_same_v<typename get_test_type_by_index<custom_tlist, 3>::type, int>);

template<auto V, typename... Ts>
struct test_holder{};

template<auto V, typename... Ts>
struct test_generator
{
    using type = test_holder<V, Ts...>;
};

using tags_set = ntl::non_typelist<1, 2, 3, 4>;

static_assert(std::is_same_v<typename test_generator<1, double>::type, test_holder<1, double>>);

using generated_tlist = typelist<test_holder<1, double>,
                                 test_holder<2, double>,
                                 test_holder<3, double>,
                                 test_holder<4, double>>;

using result_tlist = typename generate_typelist<tags_set, test_generator, double>::type;
static_assert (std::is_same_v<result_tlist, generated_tlist>);

} //tl

namespace ntl
{

non_typelist<1, 2,3> ts;
static_assert(is_empty_v<non_typelist<>>);
static_assert(size_v<non_typelist<1, 2, 3>> == 3);
static_assert(size_v<non_typelist<1>> == 1);
static_assert(front_v<non_typelist<3250, 56111, 3877>> == 3250);
static_assert(back_v<non_typelist<3250, 56111, 3877>> == 3877);
static_assert(std::is_same_v<push_front_t<non_typelist<1, 2, 3>, 8>, non_typelist<8, 1, 2, 3>>);
static_assert(std::is_same_v<push_back_t<non_typelist<1, 2, 3>, 8>, non_typelist<1, 2, 3, 8>>);
static_assert(nth_element_v<non_typelist<0, 1, 2, 19, 8>, 4> == 8);
static_assert (nth_element_v<non_typelist<1>, 0> == 1);
static_assert (nth_element_v<non_typelist<1, 2>, 1> == 2);

}//ntl

} //meta

namespace traits
{
static_assert(std::is_same_v<int, select_if_t<std::true_type, int, double>>);
static_assert(std::is_same_v<double, select_if_t<std::false_type, int, double>>);

struct Base {};
struct Derived : Base {};

static_assert(are_related_v<Base, Derived>);
static_assert(are_related_v<Base, Base>);
static_assert(!are_related_v<Derived, Base>);
static_assert(!are_related_v<Base, Derived, int>);
}

namespace meta
{

namespace dispatch
{

struct ConstructableObject
{
    ConstructableObject(int identifier)
    {
        id = identifier;
        std::cout << "ConstructableObject() id: " << id <<  "\n";
    }

    ~ConstructableObject()
    {
        std::cout << "~ConstructableObject() id: " << id << "\n";
    }

    ConstructableObject(const ConstructableObject & other) :
        id(other.id)
    {
        std::cout << "ConstructableObject(const ConstructableObject &) id: " << id << "\n ";
    }

    ConstructableObject(ConstructableObject && other) :
        id(other.id)
    {
        std::cout << "ConstructableObject(ConstructableObject &&) id: " << id << "\n ";
    }

    ConstructableObject & operator=(const ConstructableObject & other)
    {
        id = other.id;
        std::cout << "operator=(const ConstructableObject &) id: " << id << "\n ";
        return *this;
    }

    ConstructableObject & operator=(ConstructableObject && other)
    {
        id = other.id;
        std::cout << "operator=(ConstructableObject &&) id: " << id << "\n ";
        return *this;
    }
    
    int id{-1};
};

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
        value = i;
        std::cout << "std::string Tester1::test(int)\n";
        std::string ret{"this should go right into the variant"};
        return ret;
    }
    int value{0};
};

struct Tester2
{
    long long test(double b = 1.0) {
        value = b;
        std::cout << "long long Tester2::test(double)\n";
        return b; 
    }
    double value{0};
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

struct Tester5
{
    bool test(const ConstructableObject & obj)
    {
        std::cout << "void Tester5::test(const ConstructableObject &)\n";
        result = obj.id;
        return true;
    }
    int result{-1};
};

struct Tester6
{
    bool test(ConstructableObject && obj)
    {
        std::cout << "void Tester6::test(ConstructableObject &&)\n";
        result = obj.id;
        return false;
    }
    int result{-1};
};

enum TesterEnum : std::size_t
{
    T_DEFAULT = 0,
    T_1,
    T_2,
    T_3,
    T_4,
    T_5,
    T_6
};

using test_tlist = tl::typelist<Tester, Tester1, Tester2, Tester3, Tester4, Tester5, Tester6>;
//using test_tlist = tl::typelist<Tester>;
static_assert(std::is_same_v<Tester, tl::nth_element_t<test_tlist, 0>>);

DEFINE_HAS_MEMBER1(test);
DEFINE_RETURN_TYPES_FROM_MEMBER(test);
DEFINE_DISPATCH_TO_MEMBER(test);

template<typename TList>
using return_types_t = return_types_from_test_t<TList>;//typename return_types<TList>::type;


template<typename TList>
using return_variant_type = tl::to_variant_t<tl::push_front_t<tl::remove_duplicates_t<
                                             tl::erase_t<return_types_t<tl::filter_t<HasMemberT_test, TList, true>>, void>
                                             >, std::monostate>>;

template<typename TList = test_tlist>
class test_dispatcher1
{

private:
    using return_variant = tl::to_variant_t<tl::push_front_t<tl::remove_duplicates_t<
                                                                 tl::erase_t<return_types_t<tl::filter_t<HasMemberT_test, TList, true>>, void>
                                                                 >, std::monostate>>;
    struct select_test_overload
    {
        template<typename T, typename... Args,
                 typename = decltype(std::declval<T>().test(std::declval<Args&&>()...)) >
        static void call(T && object, return_variant & result, Args&&... args) {
            using return_type = std::decay_t<decltype(std::declval<T>().test(std::declval<Args&&>()...))>;
            if constexpr (tl::contains_v<return_variant, return_type>) {
                result = std::forward<T>(object).test(std::forward<Args>(args)...);
            }
            else {
                std::forward<T>(object).test(std::forward<Args>(args)...);
            }
        }

        template<typename T, typename... Args,
                 typename = decltype(std::declval<T>().test(std::declval<Args&&>()...)) >
        static decltype(auto) call(T && object, Args&&... args) {
            return std::forward<T>(object).test(std::forward<Args>(args)...);
        }

        template<typename...>
        static void call(...) {}
    };

public:
    template<typename... Args>
    static return_variant call(std::size_t tag, Args&&... args)
    {
        return_variant result;
        auto cb = [&result, &args...](auto && tester){
            std::cout << "inside lambda1\n";
            select_test_overload::call(std::move(tester), result, std::forward<Args>(args)...);
        };
        dispatch<TList>(tag, cb);
        return result;
    }

    template<bool IsConst, typename... Args>
    static return_variant call(std::size_t tag, object_wrapper<IsConst> wrapper, Args&&... args)
    {
        return_variant result;
        auto cb = [&result, &args...](auto & tester){
            std::cout << "inside lambda 2\n";
            select_test_overload::call(tester, result, std::forward<Args>(args)...);
        };
        dispatch<TList>(tag, cb, wrapper);
        return result;
    }
};

template<typename NonTList>
struct pop_front;

template<typename T,
         template<T... > typename NonTList,
         T... elements>
struct pop_front<NonTList<elements...>>
{};

void dispatch_main()
{
    Tester tst;
    tst.value = 199;
    auto p_test = std::make_unique<Tester1>();
    object_wrapper<true> apply_const = wrap_const(*p_test);
    object_wrapper<false> apply_non_const = wrap(*p_test);
    auto & const_ref = *(reinterpret_cast<cast_object_t<Tester1, object_wrapper<true>>>(apply_const.object));
    auto & const_ref2 = extractValue<Tester1>(apply_const);
    auto & non_const_ref = extractValue<Tester1>(apply_non_const);

    static_assert(std::is_same_v<decltype(const_ref), const Tester1&>);
    static_assert(std::is_same_v<decltype(const_ref2), const Tester1&>);
    static_assert(std::is_same_v<decltype(non_const_ref), Tester1&>);
    static_assert(std::is_same_v<std::add_const_t<Tester1>*, const Tester1 *>);
    static_assert(std::is_same_v<cast_object_t<Tester, object_wrapper<true>>, const Tester*>);
    static_assert(std::is_same_v<cast_object_t<Tester, object_wrapper<false>>, Tester*>);

    return_variant_type<test_tlist> res;
    res = test_dispatcher1<test_tlist>::call(TesterEnum::T_1, apply_non_const, 2);

    if (std::holds_alternative<std::string>(res))
    {
        std::cout << "the result: " << std::get<std::string>(res) << "\n";
        std::cout << "tester1 value: " << p_test->value << "\n";
    }

    Tester2 t2;
    res = test_dispatcher1<test_tlist>::call(TesterEnum::T_2, wrap(t2), 4);
    if (std::holds_alternative<long long>(res))
    {
        std::cout << "the second result: " << std::get<long long>(res) << "\n";
        std::cout << "tester2 value: " << t2.value << "\n";
    }

    //tl::to_variant_t<tl::typelist<int, double>> my_Var;
    ConstructableObject my_obj(366);
    Tester5 t5;
    res = test_dispatcher1<test_tlist>::call(TesterEnum::T_5, wrap(t5), my_obj);
    if (std::holds_alternative<bool>(res))
    {
        std::cout << "the third result: " << std::get<bool>(res) << "\n";
        std::cout << "tester5 value: " << t5.result << "\n";
    }
    std::cout << "-----test 4 starts here----\n";

    ConstructableObject your_obj(588);
    Tester6 t6;
    res = test_dispatcher<test_tlist>::call(TesterEnum::T_6, wrap(t6), std::move(your_obj));
    if (std::holds_alternative<bool>(res))
    {
        std::cout << "the fourth result: " << std::get<bool>(res) << "\n";
        std::cout << "tester6 value: " << t6.result << "\n";
    }

}

} //dispatch

namespace traits
{
struct Base{};
struct Derived : Base {};
struct NonRelated{};

static_assert(std::is_base_of_v<Base, Derived>);
static_assert(!std::is_base_of_v<Derived, Base>);
static_assert(!std::is_base_of_v<Base, NonRelated>);
static_assert(are_related_v<Derived, Base>);
static_assert(are_related_v<Base, Derived>);
static_assert(!are_related_v<Base, NonRelated>);
static_assert(!are_related_v<Derived, NonRelated>);


}//traits

} //meta
} //infra

