#pragma once

namespace wnaabi
{
namespace
{

///
/// completely braindead string_view implementation, does only a few things I
/// need at compile time (for now)
/// For the official proposal of a standard type like this:
/// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3921.html
template <typename Char>
class basic_string_view
{
public:
  using char_type = Char const;

  constexpr basic_string_view(char_type *b, char_type *e)
    : data_(b), size_(e - b)
  {
  }
  constexpr basic_string_view(char_type *b, std::size_t s) : data_(b), size_(s)
  {
  }

  constexpr std::size_t size() const { return size_; }
  constexpr char_type *begin() const { return data_; }
  constexpr char_type *end() const { return begin() + size_; }
  constexpr char_type &operator[](std::size_t i) const { return data_[i]; }

private:
  char_type *data_;
  std::size_t size_;
};

using string_view = basic_string_view<char>;

template <typename Char>
std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &os,
                                     basic_string_view<Char> const &s)
{
  os.write(s.data(), s.size());
  return os;
}
}
// namespace
}
// namespace wnaabi