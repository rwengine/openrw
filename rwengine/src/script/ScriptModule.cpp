#include <script/ScriptModule.hpp>
#include <script/ScriptMachine.hpp>

void ScriptModule::bind(ScriptFunctionID id, ScriptFunction func, bool conditional, int args, const std::string& name, const std::string& desc)
{
	functions.insert(
		{ id,
			{
				func,
				args,
				conditional,
				name,
				desc
			}
		}
				);
}

bool ScriptModule::findOpcode(ScriptFunctionID id, ScriptFunctionMeta** out)
{
	auto it = functions.find(id);
	if( it == functions.end() )
	{
		return false;
	}
	*out = &functions[id];
	return true;
}

template<> ScriptFunction conditional_facade<bool>(bool(*f)(const ScriptArguments&))
{ return [=](const ScriptArguments& a) { return a.getThread()->conditionResult = f(a); }; }

template<> bool is_conditional<bool>(bool(*)(const ScriptArguments&)) { return true; }
