#pragma once

#include <rgdk/type_traits/is_ostream_insertable.hpp>

#include <sstream>
#include <string>
#include <utility>

namespace rgdk
{

inline const std::string& to_string(const std::string& s) { return s; }

inline std::string&& to_string(std::string&& s) { return std::move(s); }

template <typename T>
std::enable_if_t<is_ostream_insertable_v<T>, std::string> to_string(T&& t)
{
   std::ostringstream output;
   output << std::forward<T>(t);
   return output.str();
}

} // namespace rgdk