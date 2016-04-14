#pragma once

#include <string>
#include <wnaabi/utility.hpp>

namespace wnaabi
{
namespace
{

///
/// tag struct for basic_string_literal constructor
constexpr struct fill_arg_t
{
} fill_arg{};

///
/// basic_string_literal holds an array of N characters at compile time.
/// Since it copies characters instead of just wrapping a pointer,
/// the compiler can optimize out any string literals it was initialize from.
/// For the official proposal of a standard type like this:
/// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4236.html
template <typename Char, std::size_t N>
class basic_string_literal
{
public:
  using char_type = Char const;
  using raw_type = char_type[N];

  ///
  /// copy construction is allowed
  constexpr basic_string_literal(basic_string_literal const &) = default;

  ///
  /// copies the first N characters from data
  constexpr basic_string_literal(char_type *data)
    : basic_string_literal{data, index_range<0, N>()}
  {
  }

  ///
  /// minor hack to allow ~aggregate initialization of basic_string_literals
  struct initializer_list
  {
    raw_type elements;
  };

  ///
  /// allow constructions like string_literal<3>({ 'a', 'b', 'c' })
  /// and string_literal<4>({ "abc" })
  constexpr basic_string_literal(initializer_list const &raw)
    : basic_string_literal{raw.elements, index_range<0, N>()}
  {
  }

  ///
  /// fill the entire basic_string_literal with copies of e
  constexpr basic_string_literal(fill_arg_t const &, char_type e)
    : basic_string_literal{fill_arg, e, index_range<0, N>()}
  {
  }

  ///
  /// by default, a basic_string_literal is zeroed out
  constexpr basic_string_literal()
    : basic_string_literal{fill_arg, static_cast<char_type>(0),
                           index_range<0, N>()}
  {
  }

  ///
  /// operator+ is overloaded to provide concatenation, as with std::string
  template <std::size_t M>
  constexpr basic_string_literal<char_type, N + M>
  operator+(basic_string_literal<char_type, M> const &other) const
  {
    return basic_string_literal<char_type, N + M>(*this, other);
  }

  ///
  /// accessors
  constexpr raw_type &data() const { return data_; }
  constexpr std::size_t size() const { return N; }
  constexpr char_type *begin() const { return data_; }
  constexpr char_type *end() const { return begin() + N; }
  constexpr char_type &operator[](std::size_t i) const { return data_[i]; }

  std::string str() const { return std::string(data_, size()); }

private:
  template <std::size_t... I>
  constexpr basic_string_literal(char_type *data, index_sequence<I...> &&)
    : data_{data[I]...}
  {
  }

  template <std::size_t... I, std::size_t... J>
  constexpr basic_string_literal(char_type *i_data, index_sequence<I...> &&,
                                 char_type *j_data, index_sequence<J...> &&)
    : data_{i_data[I]..., j_data[J]...}
  {
  }

  template <std::size_t N1, std::size_t N2>
  constexpr basic_string_literal(basic_string_literal<char_type, N1> const &n1,
                                 basic_string_literal<char_type, N2> const &n2)
    : basic_string_literal{n1.begin(), index_range<0, N1>(), n2.begin(),
                           index_range<0, N2>()}
  {
  }

  constexpr char_type ignore_index(std::size_t, char_type t) const { return t; }

  template <std::size_t... I>
  constexpr basic_string_literal(fill_arg_t const &, char_type e,
                                 index_sequence<I...> &&)
    : data_{ignore_index(I, e)...}
  {
  }

  raw_type data_;
};

template <std::size_t N>
using string_literal = basic_string_literal<char, N>;

template <typename Char, std::size_t N>
inline std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &os,
                                     basic_string_literal<Char, N> const &s)
{
  os.write(s.data(), s.size());
  return os;
}

constexpr std::size_t num_digits(std::size_t n, std::size_t base)
{
  return n == 0 ? 0 : num_digits(n / base, base) + 1;
}

constexpr std::size_t get_digit(std::size_t n, std::size_t i,
                                       std::size_t base)
{
  return i == 0 ? n % base : get_digit(n / base, i - 1, base);
}

// TODO is this the correct way to derive Char'0'?
template <typename Char>
constexpr Char get_digit_char(std::size_t n, std::size_t i,
                                     std::size_t base)
{
  return static_cast<Char>(get_digit(n, i, base)) + static_cast<Char>('0');
}

static_assert(num_digits(0b1, 2) == 1, "num_digits failure");
static_assert(num_digits(0b10, 2) == 2, "num_digits failure");
static_assert(num_digits(0b100, 2) == 3, "num_digits failure");
static_assert(num_digits(0b111, 2) == 3, "num_digits failure");
static_assert(num_digits(0b1000, 2) == 4, "num_digits failure");
static_assert(num_digits(16, 10) == 2, "num_digits failure");
static_assert(num_digits(32, 10) == 2, "num_digits failure");
static_assert(num_digits(64, 10) == 2, "num_digits failure");
static_assert(num_digits(128, 10) == 3, "num_digits failure");

static_assert(get_digit(128, 0, 10) == 8, "get_digit failure");
static_assert(get_digit(128, 1, 10) == 2, "get_digit failure");
static_assert(get_digit(128, 2, 10) == 1, "get_digit failure");
static_assert(get_digit(0b1101, 0, 2), "get_digit failure");
static_assert(!get_digit(0b1101, 1, 2), "get_digit failure");
static_assert(get_digit(0b1101, 2, 2), "get_digit failure");
static_assert(get_digit(0b1101, 3, 2), "get_digit failure");

template <typename Char, std::size_t N, std::size_t Base, std::size_t... I>
constexpr basic_string_literal<Char, num_digits(N, Base)>
to_string_literal(index_sequence<I...> const &)
{
  return basic_string_literal<Char, num_digits(N, Base)>(
    {{get_digit_char<Char>(N, Base, I)...}});
}

template <typename Char, std::size_t N, std::size_t Base>
constexpr basic_string_literal<Char, num_digits(N, Base)>
to_string_literal()
{
  return to_string_literal<Char, N, Base>(
    index_range<0, num_digits(N, Base)>{});
}

template <typename Char, Char... chars>
constexpr basic_string_literal<Char, sizeof...(chars)>
to_string_literal(char_sequence<Char, chars...> const &)
{
  return basic_string_literal<Char, sizeof...(chars)>({chars...});
}

template <typename Lit, typename Indices>
struct to_char_sequence_impl
  : to_char_sequence_impl<Lit, sequence_cast<Indices>>
{
};

template <typename Lit, std::size_t... I>
struct to_char_sequence_impl<Lit, index_sequence<I...>>
{
  using char_type = typename std::remove_const<typename Lit::char_type>::type;

  template <Lit const &lit>
  using from = char_sequence<char_type, lit[I]...>;
};

template <typename Lit, Lit const &lit>
struct to_char_sequence
  : to_char_sequence_impl<Lit, index_range<0, lit.size()>>::template from<lit>
{
};
}
// namespace
}
// namespace wnaabi
