#pragma once

#include <boost/noncopyable.hpp>
#include <functional>

namespace rgdk
{

class scope_exit : boost::noncopyable
{
public:

   template <typename Action>
   scope_exit(Action&& action) : m_action(action) { }
   ~scope_exit() { m_action(); }

private:
   std::function<void()> m_action;
};

} // namespace rgdk
