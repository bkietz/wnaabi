#include <wnaabi/pretty_function.hpp>
#include <iostream>

struct foo;

namespace bar
{
struct baz;

namespace
{
struct quux;

template <typename>
struct a_tpl;
}

template <typename>
struct tpl;
}

using wnaabi::string_literal;
using wnaabi::to_string_literal;
using wnaabi::pretty_function::strlen;
using wnaabi::pretty_function::c_str;

struct cruft_crib;
constexpr auto cruft_suffix =
  string_literal<sizeof("!!!cruft_suffix=")-1>("!!!cruft_suffix=")
  + to_string_literal<char, wnaabi::pretty_function::suffix_length, 10>()
  + string_literal<sizeof("!!!")-1>("!!!");
constexpr auto cruft_strlen =
  string_literal<sizeof("!!!cruft_strlen=")-1>("!!!cruft_strlen=")
  + to_string_literal<char, strlen<cruft_crib>(), 10>()
  + string_literal<sizeof("!!!")-1>("!!!");
constexpr auto cruft_c_str = wnaabi::pretty_function::c_str<cruft_crib>();

constexpr auto name_0 = wnaabi::get_typename<foo>::value;
constexpr auto name_1 = wnaabi::get_typename<bar::baz>::value;
constexpr auto name_2 = wnaabi::get_typename<bar::quux>::value;
constexpr auto name_3 = wnaabi::get_templatename<bar::tpl, int>::value;
constexpr auto name_4 = wnaabi::get_templatename<bar::a_tpl, foo>::value;

int main()
{
  std::cout << cruft_c_str << std::endl;
  std::cout << cruft_strlen << std::endl;
  std::cout << cruft_suffix << std::endl;
  std::cout << name_0 << std::endl;
  std::cout << name_1 << std::endl;
  std::cout << name_2 << std::endl;
  std::cout << name_3 << std::endl;
  std::cout << name_4 << std::endl;
}
