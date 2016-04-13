#pragma once

#include <wnaabi/utility.hpp>

namespace wnaabi
{
namespace
{

template <typename Remaining, typename Consumed>
struct cursor
{
};

struct noop_type_filter
{
  template <typename Current, typename... Remaining, typename... Consumed>
  auto operator()(cursor<type_sequence<Current, Remaining...>,
                         type_sequence<Consumed...>> const &) const
    -> cursor<type_sequence<Remaining...>, type_sequence<Consumed..., Current>>;
};

struct noop_integer_filter
{
  template <typename Int, Int Current, Int... Remaining, Int... Consumed>
  auto operator()(cursor<integer_sequence<Int, Current, Remaining...>,
                         integer_sequence<Int, Consumed...>> const &) const
    -> cursor<integer_sequence<Int, Remaining...>,
              integer_sequence<Int, Consumed..., Current>>;
};

template <typename Pattern>
struct remove_matching_filter : remove_matching_filter<sequence_cast<Pattern>>
{
};

template <typename Int, Int... Pattern>
struct remove_matching_filter<integer_sequence<Int, Pattern...>>
  : noop_integer_filter
{
  using noop_integer_filter::operator();

  template <Int... Remaining, Int... Consumed>
  auto operator()(cursor<integer_sequence<Int, Pattern..., Remaining...>,
                         integer_sequence<Int, Consumed...>> const &) const
    -> cursor<integer_sequence<Int, Remaining...>,
              integer_sequence<Int, Consumed...>>;
};

template <typename... Pattern>
struct remove_matching_filter<type_sequence<Pattern...>> : noop_type_filter
{
  using noop_type_filter::operator();

  template <typename... Remaining, typename... Consumed>
  auto operator()(cursor<type_sequence<Pattern..., Remaining...>,
                         type_sequence<Consumed...>> const &) const
    -> cursor<type_sequence<Remaining...>, type_sequence<Consumed...>>;
};

template <typename Filter, typename In>
struct apply_filter
  : apply_filter<Filter,
                 cursor<sequence_cast<In>, sequence_cast<empty_sequence<In>>>>
{
};

template <typename Filter, typename In, typename Consumed>
struct apply_filter<Filter, cursor<In, Consumed>>
  : apply_filter<Filter, decltype(std::declval<Filter>()(
                           std::declval<cursor<In, Consumed>>()))>
{
};

template <typename Filter, typename Consumed>
struct apply_filter<Filter,
                    cursor<sequence_cast<empty_sequence<Consumed>>, Consumed>>
  : Consumed
{
};
}
// namespace
}
// namespace wnaabi
