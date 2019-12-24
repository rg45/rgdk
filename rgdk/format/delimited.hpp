#pragma once

#include <rgdk/string/to_string.hpp>

#include <functional>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace rgdk
{

template <typename D, typename T, size_t...I>
class delimited
{
public:

   template <typename...U>
   delimited(D&& delimiter, U&&...u)
   : m_delimiter(std::forward<D>(delimiter))
   , m_tuple(std::forward<U>(u)...) {}

   friend const D& delimiter(const delimited& d) { return d.m_delimiter; }
   friend D&& delimiter(delimited&& d) { return std::move(d.m_delimiter); }

   template <size_t J>
   friend const std::tuple_element_t<J, T>& get(const delimited& d) {
      return std::get<J>(d.m_tuple); }

   template <size_t J>
   friend std::tuple_element_t<J, T>&& get(delimited&& d) {
      return std::move(std::get<J>(d.m_tuple)); }

private:
   D m_delimiter;
   T m_tuple;
};

template <typename, typename, typename> struct delimited_provider;
template <typename D, typename T, size_t...I>
struct delimited_provider<D, T, std::index_sequence<I...>> {
   using type = delimited<D, T, I...>; };

template <typename D, typename...T>
using delimited_t = typename delimited_provider<D, std::tuple<T...>, std::make_index_sequence<sizeof...(T)>>::type;

template <typename D, typename T, size_t...I>
std::ostream& operator << (std::ostream& output, const delimited<D, T, 0, I...>& d) {
   output << get<0>(d);
   std::initializer_list<int>{(output << delimiter(d) << get<I>(d), 0)...};
   return output; }

template <typename D, typename...T>
delimited_t<D, T...> make_delimited(D&& delimiter, T&&...t) {
   return { std::forward<D>(delimiter), std::forward<T>(t)... }; }

template <typename D, typename...T>
std::string make_delimited_string(D&& delimiter, T&&...t) {
   return to_string(make_delimited(std::forward<D>(delimiter), std::forward<T>(t)...)); }

} // namespace rgdk
