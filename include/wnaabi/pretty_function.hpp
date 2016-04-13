#pragma once

#include <iostream>
#include <string>
#include <wnaabi/filter.hpp>
#include <wnaabi/string_literal.hpp>
#include <wnaabi/utility.hpp>

namespace wnaabi
{
namespace
{
namespace pretty_function
{

template <typename T>
inline constexpr char const *c_str()
{
#ifdef _WIN32
  return __FUNCSIG__;
#else  // _WIN32
  return __PRETTY_FUNCTION__;
#endif // _WIN32
}

template <typename T>
inline constexpr std::size_t strlen_uncorrected()
{
#ifdef _WIN32
  return sizeof(__FUNCSIG__);
#else  // _WIN32
  return sizeof(__PRETTY_FUNCTION__);
#endif // _WIN32
}

// to save some constexpr work, I assume that __PRETTY_FUNCTION__'s prefix
// will at least include this many characters
constexpr auto prefix_length_hint =
  sizeof("char const *wnaabi::anonymous::pretty_function::c_str") - 1;

constexpr auto strlen_correction =
  strlen_uncorrected<void>()
  - (find_offset(c_str<void>() + prefix_length_hint, '\0')
     + prefix_length_hint);
static_assert(strlen_correction
                == strlen_uncorrected<int>()
                     - (find_offset(c_str<int>() + prefix_length_hint, '\0')
                        + prefix_length_hint),
              "strlen_correction is inconsistent!");

template <typename T>
inline constexpr std::size_t strlen()
{
  return strlen_uncorrected<T>() - strlen_correction;
}

// use void to figure out the lengths of __PRETTY_FUNCTION__'s prefix and suffix
// (whatever the prefix may be, I'm fairly sure it will not include the
// character 'v')
constexpr auto prefix_length =
  find_offset(c_str<void>() + prefix_length_hint, 'v') + prefix_length_hint;
constexpr auto suffix_length =
  strlen<void>() - (prefix_length + sizeof("void") - 1);

template <typename T>
constexpr inline std::size_t class_offset()
{
  // MSVC refers to class types using "struct St" and "class Cl"
  // this is not incorrect, but is unnecessarily cluttered
  // and inconsistent with other compilers
  return std::is_class<T>::value && strlen<T>() >= 7
             //&& (c_str<T>() + prefix_length)[0] == 's'
             //&& (c_str<T>() + prefix_length)[1] == 't'
             //&& (c_str<T>() + prefix_length)[2] == 'r'
             //&& (c_str<T>() + prefix_length)[3] == 'u'
             //&& (c_str<T>() + prefix_length)[4] == 'c'
             //&& (c_str<T>() + prefix_length)[5] == 't'
             && (c_str<T>() + prefix_length)[6] == ' '
           ? 7
           : std::is_class<T>::value && strlen<T>() >= 6
                 //&& (c_str<T>() + prefix_length)[0] == 'c'
                 //&& (c_str<T>() + prefix_length)[1] == 'l'
                 //&& (c_str<T>() + prefix_length)[2] == 'a'
                 //&& (c_str<T>() + prefix_length)[3] == 's'
                 //&& (c_str<T>() + prefix_length)[4] == 's'
                 && (c_str<T>() + prefix_length)[5] == ' '
               ? 6
               : 0;
}

template <typename T>
inline constexpr std::size_t typename_length()
{
  return strlen<T>() - prefix_length - suffix_length - class_offset<T>();
}

template <typename T>
inline constexpr string_literal<typename_length<T>()> typename_literal()
{
  return c_str<T>() + prefix_length + class_offset<T>();
}

template <typename T>
struct typename_sequence
{
  // nested in a struct because MSVC complains about internal linkage of
  // template arguments otherwise
  static constexpr auto value = typename_literal<T>();
  using sequence_type = sequence_cast<to_char_sequence<decltype(value), value>>;
};
///
/// this assume that the character '<' will first occur at the
/// end of a template name
/// NB: this means that types nested inside template types are right out
template <template <typename...> class T, typename... A>
inline constexpr std::size_t templatename_length()
{
  using type = T<A...>;
  return find_offset(c_str<type>() + prefix_length + class_offset<type>(), '<');
}

template <template <typename...> class T, typename... A>
inline constexpr string_literal<templatename_length<T, A...>()>
templatename_literal()
{
  using type = T<A...>;
  return c_str<type>() + prefix_length + class_offset<type>();
}

template <template <typename...> class T, typename... A>
struct templatename_sequence
{
  // nested in a struct because MSVC complains about internal linkage of
  // template arguments otherwise
  static constexpr auto value = templatename_literal<T, A...>();
  using sequence_type = sequence_cast<to_char_sequence<decltype(value), value>>;
};

///
/// retrieve the string used to indicate residence in an anonymous namespace
struct crib;

constexpr std::size_t anonymous_scope_length =
  typename_length<crib>() - (sizeof("wnaabi::") - 1)
  - (sizeof("pretty_function::crib") - 1);

constexpr string_literal<anonymous_scope_length> anonymous_scope_literal =
  string_literal<anonymous_scope_length>(c_str<crib>() + prefix_length
                                         + class_offset<crib>()
                                         + sizeof("wnaabi::") - 1);

struct anonymous_scope_sequence
{
  // nested in a struct because MSVC complains about internal linkage of
  // template arguments otherwise
  static constexpr auto value = anonymous_scope_literal;
  using sequence_type = sequence_cast<to_char_sequence<decltype(value), value>>;
};
}
// namespace pretty_function

// TODO this should go in another header file
template <typename T>
struct get_typename
  : apply_filter<remove_matching_filter<pretty_function::
                                          anonymous_scope_sequence>,
                 pretty_function::typename_sequence<T>>
{
  static constexpr auto value =
    to_string_literal(sequence_cast<get_typename>{});
};

// TODO this should go in another header file
template <template <typename...> class T, typename... A>
struct get_templatename
  : apply_filter<remove_matching_filter<pretty_function::
                                          anonymous_scope_sequence>,
                 pretty_function::templatename_sequence<T, A...>>
{
  static constexpr auto value =
    to_string_literal(sequence_cast<get_templatename>{});
};
}
// namespace
}
// namespace wnaabi
