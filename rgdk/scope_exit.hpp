#pragma once

#include <functional>

namespace rgdk
{

class scope_exit
{
public:

   template <typename Action>
   scope_exit(Action&& action) : m_action(action) { }
   ~scope_exit() { m_action(); }

   scope_exit(const scope_exit&) = delete;
   scope_exit& operator=(const scope_exit&) = delete;

private:
   std::function<void()> m_action;
};

} // namespace rgdk
