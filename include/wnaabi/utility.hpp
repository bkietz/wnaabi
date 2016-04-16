#pragma once

#include <type_traits>
#include <utility>

namespace wnaabi
{
namespace
{

template <typename T>
struct identity
{
  using type = T;
  constexpr identity() = default;
};
template <typename, typename R>
using replace = R;
template <bool Test, typename R>
using replace_enable = replace<typename std::enable_if<Test>::type, R>;

template <bool, typename, typename>
struct if_else_impl;
template <typename Then, typename Else>
struct if_else_impl<true, Then, Else>
{
  using type = Then;
};
template <typename Then, typename Else>
struct if_else_impl<false, Then, Else>
{
  using type = Else;
};

///
/// if the first argument is true, same as second argument
/// otherwise same as the third argument
template <bool Test, typename Then, typename Else>
using if_else = typename if_else_impl<Test, Then, Else>::type;

///
/// concept sequence: has a member type named sequence_type
template <typename Seq>
struct is_sequence
{
  template <typename S>
  static std::false_type test(...);
  template <typename S>
  static std::true_type test(identity<typename S::sequence_type> *);
  static constexpr bool value = decltype(test<Seq>(nullptr))::value;
};

template <typename Seq>
struct sequence_cast_checked
{
  static_assert(is_sequence<Seq>::value,
                "attempted sequence_cast on type which is not a sequence");
  using sequence_type = typename Seq::sequence_type;
};

template <typename Seq>
using sequence_cast = typename sequence_cast_checked<Seq>::sequence_type;

///
/// sequence of std::size_t
template <typename Int, Int...>
struct integer_sequence
{
  using sequence_type = integer_sequence;
};

///
/// sequence of std::size_t
template <std::size_t... Indices>
using index_sequence = integer_sequence<std::size_t, Indices...>;

///
/// sequence of types
template <typename...>
struct type_sequence
{
  using sequence_type = type_sequence;
};

static_assert(is_sequence<type_sequence<int, float, double>>::value, "WTF");
static_assert(is_sequence<index_sequence<1, 5, 2>>::value, "WTF");
static_assert(!is_sequence<int>::value, "WTF");

///
/// compile time sequence of characters.
/// The more natural definition would be
///
///    template <typename Char, Char... chars>
///    using char_sequence = integer_sequence<Char, chars...>;
///
/// but this causes MSVC to emit C1001 errors on usage with filters
template <typename Char, Char... chars>
using char_sequence = type_sequence<std::integral_constant<Char, chars>...>;

///
/// Removes elements from Seq
/// type_sequence<T...>       -> type_sequence<>
/// integer_sequence<I, i...> -> integer_sequence<I>
template <typename Seq>
struct empty_sequence : empty_sequence<sequence_cast<Seq>>
{
};

template <typename... T>
struct empty_sequence<type_sequence<T...>> : type_sequence<>
{
};

template <typename Int, Int... ints>
struct empty_sequence<integer_sequence<Int, ints...>> : integer_sequence<Int>
{
};

///
/// concatenates two sequences
template <typename L, typename R>
struct concat_sequence : concat_sequence<sequence_cast<L>, sequence_cast<R>>
{
};

template <typename Int, Int... I, Int... J>
struct concat_sequence<integer_sequence<Int, I...>, integer_sequence<Int, J...>>
  : integer_sequence<Int, I..., J...>
{
};

template <typename... I, typename... J>
struct concat_sequence<type_sequence<I...>, type_sequence<J...>>
  : type_sequence<I..., J...>
{
};

static_assert(std::is_base_of<index_sequence<0, 3, 2, 1, 4>,
                              concat_sequence<index_sequence<0, 3, 2>,
                                              index_sequence<1, 4>>>::value,
              "concat_sequence failure");

template <typename It>
constexpr It midpoint(It b, It e)
{
  return b + (e - b) / 2;
}

///
/// an integer_sequence containing the integers in [Min, Max)
template <typename Int, Int Min, Int Max>
struct integer_range
  : if_else<Min == Max, integer_sequence<Int>,
            if_else<Min + 1 == Max, integer_sequence<Int, Min>,
                    concat_sequence<integer_range<Int, Min, midpoint(Min, Max)>,
                                    integer_range<Int, midpoint(Min, Max),
                                                  Max>>>>
{
};

template <std::size_t Min, std::size_t Max>
using index_range = integer_range<std::size_t, Min, Max>;

static_assert(
  std::is_base_of<index_sequence<0, 1, 2, 3>, index_range<0, 4>>::value,
  "index_range failure");
static_assert(std::is_base_of<index_sequence<5>, index_range<5, 6>>::value,
              "index_range failure");
static_assert(std::is_base_of<index_sequence<>, index_range<8, 8>>::value,
              "index_range failure");
using large_range =
  index_range<0, (1 << 10)>; // if this doesn't compile, we're in trouble

///
/// characters are distinct from other integers!
/// 3.9.1 Fundamental types [basic.fundamental]
template <typename Ch>
struct is_character : std::false_type
{
};
template <>
struct is_character<char> : std::true_type
{
};
template <>
struct is_character<wchar_t> : std::true_type
{
};
template <>
struct is_character<char16_t> : std::true_type
{
};
template <>
struct is_character<char32_t> : std::true_type
{
};
template <typename T>
using is_unqualified =
  std::integral_constant<bool,
                         std::is_const<T>::value || std::is_volatile<T>::value>;
template <typename T>
using is_bool = std::is_same<T, bool>;
template <typename I>
using is_integral =
  std::integral_constant<bool,
                         std::is_integral<I>::value && !is_character<I>::value
                           && !is_bool<I>::value && !is_unqualified<I>::value>;

template <typename I>
using is_signed_integral =
  std::integral_constant<bool,
                         is_integral<I>::value && std::is_signed<I>::value>;

template <typename I>
using is_unsigned_integral =
  std::integral_constant<bool,
                         is_integral<I>::value && std::is_unsigned<I>::value>;

template <typename Tag, typename Num>
struct is_numeric : std::false_type
{
};

template <typename Num>
struct is_numeric<unsigned, Num> : is_unsigned_integral<Num>
{
};

template <typename Num>
struct is_numeric<signed, Num> : is_signed_integral<Num>
{
};

template <typename Num>
struct is_numeric<float, Num> : std::is_floating_point<Num>
{
};

template <typename Num>
struct is_numeric<char, Num> : is_character<Num>
{
};

template <typename Tag, typename Num, typename R=void>
using enable_if_numeric = typename std::enable_if<is_numeric<Tag, Num>::value, R>::type;

///
/// braindead find impl
template <typename It, typename N>
constexpr std::size_t find_offset(It b_hay, N const &needle)
{
  return *b_hay == needle ? 0 : 1 + find_offset(b_hay + 1, needle);
}
}
// namespace
}
// namespace wnaabi
