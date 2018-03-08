#pragma once

template <typename T, typename U>
struct replace_type { using type = U; };

template <typename T, typename U>
using replace_type_t = typename replace_type<T, U>::type;



