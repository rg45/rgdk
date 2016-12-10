#pragma once

#include <sstream>
#include <string>

namespace rgdk
{
namespace type_info
{
namespace detail
{

template <typename T>
class type_name_extractor
{
public:

   type_name_extractor() { m_output << __FUNCSIG__; }

   std::string operator()() const
   {
      auto&& str = m_output.str();
      auto begin = str.find('<') + 1;
      auto end = str.rfind('>');
      return str.substr(begin, end - begin);
   }

private:
   std::ostringstream m_output;
};

} // namespace detail

template <typename T>
std::string type_name()
{
   return detail::type_name_extractor<T>()();
};

} // namespace type_info

using type_info::type_name;

} // namespace rgdk
