#pragma once
#include <cstdint>
#include <wnaabi/pretty_function.hpp>
#include <wnaabi/string_literal.hpp>
#include <wnaabi/string_view.hpp>
#include <wnaabi/utility.hpp>

namespace wnaabi
{
namespace
{

///
/// a class containing static type information
/// this is either a string containing the name of T
/// or a rope of tokens which make up the name of T.
/// Visitors can be applied to the tokens in this rope
/// using type_info<T>::name_tokens
template <typename T, typename Enable = void>
struct type_info;

namespace typename_tokens
{
template <std::size_t N>
constexpr string_literal<N - 1> make_token(char const (&arr)[N])
{
  return arr;
}

using pointer = char_sequence<char, '*'>;
using l_reference = char_sequence<char, '&'>;
using r_reference = char_sequence<char, '&', '&'>;
using const_qualifier = char_sequence<char, ' ', 'c', 'o', 'n', 's', 't'>;
using volatile_qualifier =
  char_sequence<char, ' ', 'v', 'o', 'l', 'a', 't', 'i', 'l', 'e'>;
using l_bracket = char_sequence<char, '['>;
using r_bracket = char_sequence<char, ']'>;
using l_angle = char_sequence<char, '<'>;
using r_angle = char_sequence<char, '>'>;
using l_paren = char_sequence<char, '('>;
using r_paren = char_sequence<char, ')'>;
}
// namespace typename_tokens

///
/// Apply a visitor to a rope of tokens.
/// Takes elements by value since I expect to use this with empty/small objects.
///
/// TODO maybe this should be called curry_tokens or so?
template <typename F>
constexpr F each(F &&f)
{
  return std::forward<F>(f);
}

///
/// visitors must visit char_sequence directly
template <typename F, char... c, typename... T>
constexpr auto each(F &&f, char_sequence<char, c...> const &h, T... t)
  -> decltype(each(std::forward<F>(f)(h), t...))
{
  return each(std::forward<F>(f)(h), t...);
}

///
/// visitors must visit type_info directly
template <typename F, typename H, typename... T>
constexpr auto each(F &&f, type_info<H> const &h, T... t)
  -> decltype(each(std::forward<F>(f)(h), t...))
{
  return each(std::forward<F>(f)(h), t...);
}

namespace constexpr_visitors
{

///
/// CRTP mixin for visiting type_info tokens using type_info::name_tokens
/// visitors which don't want to do clever things with individual type_info
/// can inherit this
template <typename Visitor>
struct recurse_into_type_info_tokens
{
  using derived_type = Visitor;
  using self_type = recurse_into_type_info_tokens;

  constexpr derived_type const &cast() const
  {
    static_assert(std::is_base_of<self_type, derived_type>::value,
                  "CRTP instantiation error. your_type must inherit "
                  "recurse_into_type_info_tokens<your_type>");
    return static_cast<derived_type const &>(*this);
  }

  template <typename T>
  constexpr auto operator()(type_info<T>) const
    -> decltype(type_info<T>::name_tokens(cast()))
  {
    return type_info<T>::name_tokens(cast());
  }
};

template <std::size_t Size = 0>
struct size_t : std::integral_constant<std::size_t, Size>,
                recurse_into_type_info_tokens<size_t<Size>>
{
  using recurse_into_type_info_tokens<size_t<Size>>::operator();

  template <char... c>
  constexpr size_t<Size + sizeof...(c)>
  operator()(char_sequence<char, c...>) const
  {
    return {};
  }
};

constexpr size_t<> size = {};

template <typename Chars = char_sequence<char>>
struct sequence_t : Chars, recurse_into_type_info_tokens<sequence_t<Chars>>
{
  template <typename Token>
  using concat = sequence_t<sequence_cast<concat_sequence<Chars, Token>>>;

  template <typename Token>
  constexpr concat<Token> operator()(Token) const
  {
    return {};
  }
};

constexpr sequence_t<> sequence = {};

template <std::size_t Hash = 0>
struct hash_t : std::integral_constant<std::size_t, Hash>,
                recurse_into_type_info_tokens<hash_t<Hash>>
{
  using recurse_into_type_info_tokens<hash_t<Hash>>::operator();

  template <char H, char... T>
  static constexpr std::size_t combine(std::size_t v,
                                       char_sequence<char, H, T...> const &)
  {
    return combine((v * 33) ^ static_cast<std::uint8_t>(H),
                   char_sequence<char, T...>{});
  }

  static constexpr std::size_t combine(std::size_t v, ...) { return v; }

  template <char... c>
  constexpr hash_t<combine(Hash, std::declval<char_sequence<char, c...>>())>
  operator()(char_sequence<char, c...>) const
  {
    return {};
  }
};
constexpr hash_t<> hash = {};
}
// namespace constexpr_visitors

namespace runtime_visitors
{

using constexpr_visitors::recurse_into_type_info_tokens;

template <typename Ostream>
struct stream_out_t : recurse_into_type_info_tokens<stream_out_t<Ostream>>
{
  Ostream &os_;
  stream_out_t(Ostream &os) : os_(os) {}

  using recurse_into_type_info_tokens<stream_out_t<Ostream>>::operator();

  template <char... c>
  stream_out_t operator()(char_sequence<char, c...> const &)
  {
    os_ << to_string_literal(char_sequence<char, c...>{});
    return *this;
  }
};

template <typename Ostream>
stream_out_t<Ostream> stream_out(Ostream &os)
{
  return stream_out_t<Ostream>(os);
}

struct stringify_t : recurse_into_type_info_tokens<stringify_t>
{
  std::string str;

  using recurse_into_type_info_tokens<stringify_t>::operator();

  template <char... c>
  stringify_t operator()(char_sequence<char, c...> const &)
  {
    str += to_string_literal(char_sequence<char, c...>{}).str();
    return std::move(*this);
  }
};
}
// namespace runtime_visitors

///
/// The default for type_info is to simply use
/// pretty_function::get_typename<T> as the only token,
/// however specializations of type_info will mean
/// this definition only applies to class types
template <typename T, typename Enable>
struct type_info
{
  static_assert(std::is_class<T>::value,
                "type_info is not specialized for this non-class type");

  using name_sequence = get_typename<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, name_sequence{}))
  {
    return each(vis, name_sequence{});
  }
};

///
/// specializations to intercept numeric types and force them into
/// fixed width format: (uint|int|float|char)${BITS}_t

///
/// unsigned integers
template <typename T>
struct type_info<T, enable_if_numeric<unsigned, T>>
{
  using name_type = decltype(typename_tokens::make_token("uint")
                             + to_string_literal<char, sizeof(T) * 8, 10>()
                             + typename_tokens::make_token("_t"));
  static constexpr name_type name_ =
    typename_tokens::make_token("uint")
    + to_string_literal<char, sizeof(T) * 8, 10>()
    + typename_tokens::make_token("_t");

  using name_sequence = to_char_sequence<name_type, name_>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, name_sequence{}))
  {
    return each(vis, name_sequence{});
  }
};

template <typename T>
constexpr typename type_info<T, enable_if_numeric<unsigned, T>>::name_type
  type_info<T, enable_if_numeric<unsigned, T>>::name_;

///
/// signed integers
template <typename T>
struct type_info<T, enable_if_numeric<signed, T>>
{
  using name_type = decltype(typename_tokens::make_token("int")
                             + to_string_literal<char, sizeof(T) * 8, 10>()
                             + typename_tokens::make_token("_t"));
  static constexpr name_type name_ =
    typename_tokens::make_token("int")
    + to_string_literal<char, sizeof(T) * 8, 10>()
    + typename_tokens::make_token("_t");

  using name_sequence = to_char_sequence<name_type, name_>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, name_sequence{}))
  {
    return each(vis, name_sequence{});
  }
};

template <typename T>
constexpr typename type_info<T, enable_if_numeric<signed, T>>::name_type
  type_info<T, enable_if_numeric<signed, T>>::name_;

///
/// floating point numbers
template <typename T>
struct type_info<T, enable_if_numeric<float, T>>
{
  using name_type = decltype(typename_tokens::make_token("float")
                             + to_string_literal<char, sizeof(T) * 8, 10>()
                             + typename_tokens::make_token("_t"));
  static constexpr name_type name_ =
    typename_tokens::make_token("float")
    + to_string_literal<char, sizeof(T) * 8, 10>()
    + typename_tokens::make_token("_t");

  using name_sequence = to_char_sequence<name_type, name_>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, name_sequence{}))
  {
    return each(vis, name_sequence{});
  }
};

template <typename T>
constexpr typename type_info<T, enable_if_numeric<float, T>>::name_type
  type_info<T, enable_if_numeric<float, T>>::name_;

///
/// characters
template <typename T>
struct type_info<T, enable_if_numeric<char, T>>
{
  using name_type = decltype(typename_tokens::make_token("char")
                             + to_string_literal<char, sizeof(T) * 8, 10>()
                             + typename_tokens::make_token("_t"));
  static constexpr name_type name_ =
    typename_tokens::make_token("char")
    + to_string_literal<char, sizeof(T) * 8, 10>()
    + typename_tokens::make_token("_t");

  using name_sequence = to_char_sequence<name_type, name_>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, name_sequence{}))
  {
    return each(vis, name_sequence{});
  }
};

template <typename T>
constexpr typename type_info<T, enable_if_numeric<char, T>>::name_type
  type_info<T, enable_if_numeric<char, T>>::name_;

///
/// pointer types
template <typename T>
struct type_info<T *>
{
  using pointed = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, pointed{}, typename_tokens::pointer{}))
  {
    return each(vis, pointed{}, typename_tokens::pointer{});
  }
};

///
/// lvalue references
template <typename T>
struct type_info<T &>
{
  using pointed = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, pointed{}, typename_tokens::l_reference{}))
  {
    return each(vis, pointed{}, typename_tokens::l_reference{});
  }
};

///
/// rvalue references
template <typename T>
struct type_info<T &&>
{
  using pointed = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, pointed{}, typename_tokens::r_reference{}))
  {
    return each(vis, pointed{}, typename_tokens::r_reference{});
  }
};

///
/// array types
/// FIXME this is going to break hard for arrays of complicated things like
/// function pointers
template <typename T, std::size_t N>
struct type_info<T[N]>
{
  using pointed = type_info<T>;

  using extent_type = decltype(to_string_literal<char, N, 10>());
  static constexpr extent_type extent = to_string_literal<char, N, 10>();
  using extent_sequence = to_char_sequence<extent_type, extent>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, pointed{}, typename_tokens::l_bracket{},
                     extent_sequence{}, typename_tokens::r_bracket{}))
  {
    return each(vis, pointed{}, typename_tokens::l_bracket{}, extent_sequence{},
                typename_tokens::r_bracket{});
  }
};

template <typename T, std::size_t N>
constexpr typename type_info<T[N]>::extent_type type_info<T[N]>::extent;

template <typename T>
struct type_info<T[]>
{
  using pointed = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, pointed{}, typename_tokens::l_bracket{},
                     typename_tokens::r_bracket{}))
  {
    return each(vis, pointed{}, typename_tokens::l_bracket{},
                typename_tokens::r_bracket{});
  }
};

///
/// const types
template <typename T>
struct type_info<T const>
{
  using unqualified = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, unqualified{}, typename_tokens::const_qualifier{}))
  {
    return each(vis, unqualified{}, typename_tokens::const_qualifier{});
  }
};

///
/// volatile types
template <typename T>
struct type_info<T volatile>
{
  using unqualified = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, unqualified{}, typename_tokens::volatile_qualifier{}))
  {
    return each(vis, unqualified{}, typename_tokens::volatile_qualifier{});
  }
};

///
/// const volatile types
/// (specialized separately to guarantee that "const" is visited before
/// "volatile")
template <typename T>
struct type_info<T const volatile>
{
  using unqualified = type_info<T>;

  template <typename Visitor>
  constexpr static auto name_tokens(Visitor vis)
    -> decltype(each(vis, unqualified{}, typename_tokens::const_qualifier{},
                     typename_tokens::volatile_qualifier{}))
  {
    return each(vis, unqualified{}, typename_tokens::const_qualifier{},
                typename_tokens::volatile_qualifier{});
  }
};

// TODO:
// function types
// template types
// member data pointer types
// member function pointer types
// array of pointer to function returning member pointer function taking
//     arguments that are templates instantiated against array of array of...
}
// namespace
}
// namespace wnaabi
