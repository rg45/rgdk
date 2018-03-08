#pragma once

#include <rgdk/type_traits/replace_type.hpp>

template <typename T>
using void_type = replace_type<T, void>;

template <typename T>
using void_type_t = typename void_type<T>::type;

