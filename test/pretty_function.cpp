#include <algorithm>
#include <gtest/gtest.h>
#include <regex>
#include <wnaabi/pretty_function.hpp>

// this is nifty but where does it go?
template <typename T>
std::string string_with_prefix_suffix_dividers()
{
  constexpr auto c_str = wnaabi::pretty_function::c_str<T>();

  constexpr auto length = wnaabi::pretty_function::strlen<T>();

  constexpr auto typename_length =
    wnaabi::pretty_function::typename_length<T>();

  constexpr auto prefix = wnaabi::pretty_function::prefix_length
                          + wnaabi::pretty_function::class_offset<T>();

  constexpr auto suffix = wnaabi::pretty_function::suffix_length;

  std::string str;
  str.reserve(length + 2);
  str.append(c_str, c_str + prefix);
  str.append(1, '^');
  str.insert(c_str + prefix, c_str + prefix + typename_length);
  return str;
}

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

// Some of these are redundant due to static_asserts which must already
// have compiled for the test to run. Oh well.
TEST(PrettyFunction, CompileTimeStringAssumptions)
{
  constexpr auto void_c_str_b = wnaabi::pretty_function::c_str<void>();
  auto void_c_str_size = std::strlen(void_c_str_b);
  constexpr auto void_strlen = wnaabi::pretty_function::strlen<void>();

  // strlen<void> is correct
  EXPECT_EQ(void_c_str_size, void_strlen);
  auto void_c_str_e = void_c_str_b + void_c_str_size;

  auto void_b = "void";
  auto void_e = void_b + 4;
  auto void_found = std::search(void_c_str_b, void_c_str_e, void_b, void_e);

  // "void" should appear in c_str<void>
  EXPECT_NE(void_found, void_c_str_e);
  constexpr auto prefix_length_hint =
    wnaabi::pretty_function::prefix_length_hint;
  constexpr auto prefix_length = wnaabi::pretty_function::prefix_length;
  constexpr auto suffix_length = wnaabi::pretty_function::suffix_length;
  // prefix_length_hint is appropriate
  EXPECT_GT(prefix_length, prefix_length_hint);
  EXPECT_EQ(void_found - void_c_str_b, prefix_length);
  EXPECT_EQ(void_c_str_e - (void_found + 4), suffix_length);

  // void is a good crib since 'v' appears nowhere before "void" in c_str<void>
  auto v_found = std::find(void_c_str_b, void_c_str_e, 'v');
  EXPECT_EQ(v_found, void_found) << void_c_str_b;

  // namespaced types are scoped with "::" and no whitespace
  constexpr auto baz_c_str_b = wnaabi::pretty_function::c_str<bar::baz>();
  constexpr auto baz_c_str_e =
    baz_c_str_b + wnaabi::pretty_function::strlen<bar::baz>();
  auto baz_b = "bar::baz";
  auto baz_e = baz_b + 8;
  auto baz_found = std::search(baz_c_str_b, baz_c_str_e, baz_b, baz_e);
  EXPECT_NE(baz_found, baz_c_str_e);

  // '<' occurs exactly after the templatename
  constexpr auto tpl_c_str_b = wnaabi::pretty_function::c_str<bar::tpl<int>>();
  constexpr auto tpl_c_str_e =
    tpl_c_str_b + wnaabi::pretty_function::strlen<bar::tpl<int>>();
  auto tpl_b = "bar::tpl";
  auto tpl_e = tpl_b + 8;
  auto tpl_found = std::search(tpl_c_str_b, tpl_c_str_e, tpl_b, tpl_e);
  EXPECT_NE(tpl_found, tpl_c_str_e);
  auto angle_found = std::find(tpl_found, tpl_c_str_e, '<');
  EXPECT_EQ(angle_found - tpl_found, 8);

  // typename_literal<bar::quux> is exactly "bar::(.*)::quux"
  std::regex quux_exp{"^bar::(.*)::quux$"};
  constexpr auto quux_lit =
    wnaabi::pretty_function::typename_literal<bar::quux>();
  auto quux_str = quux_lit.str();
  std::smatch quux_results;
  auto quux_matched = std::regex_match(quux_str, quux_results, quux_exp);
  EXPECT_TRUE(quux_matched);

  // anonymous_scope_literal is longer than "anonymous::"
  EXPECT_GE(static_cast<std::size_t>(quux_results[1].length()),
            std::strlen("anonymous::"));

  using wnaabi::pretty_function::typename_length;
  static_assert(typename_length<int>() == 3, "typename_length<int>");
  static_assert(typename_length<void>() == 4, "typename_length<void>");
  static_assert(typename_length<float>() == 5, "typename_length<float>");
}

TEST(PrettyFunction, UserDefinedTypenameLiterals)
{
  using wnaabi::pretty_function::typename_literal;
  EXPECT_EQ(typename_literal<foo>().str(), "foo");
  EXPECT_EQ(typename_literal<bar::baz>().str(), "bar::baz");

  using wnaabi::pretty_function::templatename_literal;
  EXPECT_EQ((templatename_literal<bar::tpl, int>()).str(), "bar::tpl");

  using wnaabi::pretty_function::anonymous_scope_literal;
  EXPECT_EQ(typename_literal<bar::quux>().str(),
            ("bar::" + anonymous_scope_literal.str() + "quux"));

  EXPECT_EQ((templatename_literal<bar::a_tpl, bar::quux>()).str(),
            ("bar::" + anonymous_scope_literal.str() + "a_tpl"));
}

template <typename L, typename R>
using same_seq =
  std::is_same<wnaabi::sequence_cast<L>, wnaabi::sequence_cast<R>>;

TEST(PrettyFunction, UserDefinedTypenameSequences)
{
  using wnaabi::char_sequence;
  using wnaabi::pretty_function::typename_sequence;
  static_assert(
    same_seq<typename_sequence<foo>, char_sequence<char, 'f', 'o', 'o'>>::value,
    "foo");
  static_assert(same_seq<typename_sequence<bar::baz>,
                         char_sequence<char, 'b', 'a', 'r', ':', ':', 'b', 'a',
                                       'z'>>::value,
                "bar::baz");

  using wnaabi::pretty_function::templatename_sequence;
  static_assert(same_seq<templatename_sequence<bar::tpl, int>,
                         char_sequence<char, 'b', 'a', 'r', ':', ':', 't', 'p',
                                       'l'>>::value,
                "bar::tpl");

  using wnaabi::concat_sequence;
  using wnaabi::pretty_function::anonymous_scope_sequence;
  using bar_anonymous =
    concat_sequence<char_sequence<char, 'b', 'a', 'r', ':', ':'>,
                    anonymous_scope_sequence>;
  static_assert(
    same_seq<typename_sequence<bar::quux>,
             concat_sequence<bar_anonymous,
                             char_sequence<char, 'q', 'u', 'u', 'x'>>>::value,
    "bar::quux");

  // intellisense sometimes flags this static_assert, but MSVC passes it
  static_assert(
    same_seq<templatename_sequence<bar::a_tpl, bar::quux>,
             concat_sequence<bar_anonymous, char_sequence<char, 'a', '_', 't',
                                                          'p', 'l'>>>::value,
    "bar::a_tpl");

  // wnaabi::get_typename should strip anonymous scoping
  EXPECT_EQ(wnaabi::get_typename<bar::quux>::value.str(), "bar::quux");

  // wnaabi::get_templatename should strip anonymous scoping
  EXPECT_EQ((wnaabi::get_templatename<bar::a_tpl, float>::value.str()),
            "bar::a_tpl");
}

struct
  super_long_identifier_super_long_identifier_super_long_identifier_super_long_identifier;

TEST(PrettyFunction, StressTesting)
{
  // this is mostly here to make sure the compiler doesn't break
  // FEAR THE C1001 ERROR
  auto sl =
    wnaabi::
      get_typename<super_long_identifier_super_long_identifier_super_long_identifier_super_long_identifier>::
        value.str();
  EXPECT_EQ(sl,
            "super_long_identifier_super_long_identifier_super_long_identifier_"
            "super_long_identifier");
  constexpr auto other = wnaabi::string_literal<4>({"abc"});
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
