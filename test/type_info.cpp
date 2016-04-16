#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <wnaabi/type_info.hpp>

auto visitor = wnaabi::runtime_visitors::stringify_t{};

TEST(TypeInfo, FundamentalTypenames)
{
  using wnaabi::type_info;
  EXPECT_EQ(type_info<std::uint8_t>::name_tokens(visitor).str, "uint8_t");
  EXPECT_EQ(type_info<std::uint16_t>::name_tokens(visitor).str, "uint16_t");
  EXPECT_EQ(type_info<std::uint32_t>::name_tokens(visitor).str, "uint32_t");
  EXPECT_EQ(type_info<std::uint64_t>::name_tokens(visitor).str, "uint64_t");

  EXPECT_EQ(type_info<std::int8_t>::name_tokens(visitor).str, "int8_t");
  EXPECT_EQ(type_info<std::int16_t>::name_tokens(visitor).str, "int16_t");
  EXPECT_EQ(type_info<std::int32_t>::name_tokens(visitor).str, "int32_t");
  EXPECT_EQ(type_info<std::int64_t>::name_tokens(visitor).str, "int64_t");

  EXPECT_EQ(type_info<float>::name_tokens(visitor).str, "float32_t");
  EXPECT_EQ(type_info<double>::name_tokens(visitor).str, "float64_t");

  EXPECT_EQ(type_info<char>::name_tokens(visitor).str, "char8_t");
  EXPECT_EQ(type_info<char16_t>::name_tokens(visitor).str, "char16_t");
  EXPECT_EQ(type_info<char32_t>::name_tokens(visitor).str, "char32_t");
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

TEST(TypeInfo, ClassNames)
{
  using wnaabi::type_info;
  EXPECT_EQ(type_info<foo>::name_tokens(visitor).str, "foo");
  EXPECT_EQ(type_info<bar::baz>::name_tokens(visitor).str, "bar::baz");
  EXPECT_EQ(type_info<bar::quux>::name_tokens(visitor).str, "bar::quux");
}

TEST(TypeInfo, PointerTypenames)
{
  using wnaabi::type_info;
  EXPECT_EQ(type_info<foo *>::name_tokens(visitor).str, "foo*");
  EXPECT_EQ(type_info<std::int32_t *>::name_tokens(visitor).str, "int32_t*");
  EXPECT_EQ(type_info<bar::baz &>::name_tokens(visitor).str, "bar::baz&");
  EXPECT_EQ(type_info<std::int32_t &>::name_tokens(visitor).str, "int32_t&");
  EXPECT_EQ(type_info<bar::quux &&>::name_tokens(visitor).str, "bar::quux&&");
  EXPECT_EQ(type_info<std::int32_t &&>::name_tokens(visitor).str, "int32_t&&");
}

TEST(TypeInfo, QualifiedTypenames)
{
  using wnaabi::type_info;
  EXPECT_EQ(type_info<const foo>::name_tokens(visitor).str, "foo const");
  EXPECT_EQ(type_info<std::int32_t volatile>::name_tokens(visitor).str, "int32_t volatile");
  EXPECT_EQ(type_info<volatile bar::baz const>::name_tokens(visitor).str, "bar::baz const volatile");
}

TEST(TypeInfo, ArrayTypenames)
{
  using wnaabi::type_info;
  EXPECT_EQ(type_info<foo[34]>::name_tokens(visitor).str, "foo[34]");
  EXPECT_EQ(type_info<std::int32_t[]>::name_tokens(visitor).str, "int32_t[]");
  EXPECT_EQ(type_info<bar::quux * [0xFF]>::name_tokens(visitor).str, "bar::quux*[255]");
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
