#ifndef _SCRIPTMODULE_HPP_
#define _SCRIPTMODULE_HPP_
#include <script/ScriptTypes.hpp>
#include "ScriptMachine.hpp"

#include <string>

/**
 * Interface for a collection of functions that can be exported to a game script interface.
 *
 * For example a collection of functions that control the time of day, or create objects would
 * be the collected within one ScriptModule with a sensible name like "Environment" or "Objects"
 */
class ScriptModule
{
public:
  ScriptModule(const std::string& name) : name(name) {}

  const std::string& getName() const { return name; }

  void bind(ScriptFunctionID id, ScriptFunction func, bool conditional, int args,
            const std::string& name, const std::string& desc);

  bool findOpcode(ScriptFunctionID id, ScriptFunctionMeta** out);

private:
  const std::string name;
  std::map<ScriptFunctionID, ScriptFunctionMeta> functions;
};

template <class Tret>
ScriptFunction conditional_facade(Tret (*f)(const ScriptArguments&))
{
  return f;
}
template <>
ScriptFunction conditional_facade<bool>(bool (*f)(const ScriptArguments&));

template <class Tret>
bool is_conditional(Tret (*)(const ScriptArguments&))
{
  return false;
}
template <>
bool is_conditional<bool>(bool (*f)(const ScriptArguments&));

// Macro to automatically use function name.
#define bindFunction(id, func, argc, desc) \
  bind(id, conditional_facade(func), is_conditional(func), argc, #func, desc)
#define bindUnimplemented(id, func, argc, desc) bind(id, 0, false, argc, #func, desc)

#endif
