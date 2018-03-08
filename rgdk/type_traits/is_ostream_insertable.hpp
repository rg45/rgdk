#pragma once

#include <rgdk/type_traits/void_type.hpp>

#include <ostream>
#include <type_traits>
#include <utility>

template <typename, typename = void>
struct is_ostream_insertable : std::false_type {};

template <typename T>
struct is_ostream_insertable<T, void_type_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>>
   : std::true_type {};

template <typename T>
constexpr bool is_ostream_insertable_v = is_ostream_insertable<T>::value;
