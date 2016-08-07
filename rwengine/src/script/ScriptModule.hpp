#ifndef _SCRIPTMODULE_HPP_
#define _SCRIPTMODULE_HPP_
#include <script/ScriptTypes.hpp>
#include "ScriptMachine.hpp"

#include <string>

namespace script_bind
{
	template <class T>
	struct arg_traits
	{
		static constexpr size_t arg_size = 1;
	};

	template <>
	struct arg_traits<ScriptVec2>
	{
		static constexpr size_t arg_size = 2;
	};

	template <>
	struct arg_traits<ScriptVec3>
	{
		static constexpr size_t arg_size = 3;
	};

	template <>
	struct arg_traits<ScriptRGB>
	{
		static constexpr size_t arg_size = 3;
	};

	template <>
	struct arg_traits<ScriptRGBA>
	{
		static constexpr size_t arg_size = 4;
	};

	template <class T, unsigned arg>
	struct unpack
	{
		static T convert(const ScriptArguments& args)
		{
			return args.getParameter<T>(arg);
		}
	};

	template <unsigned arg>
	struct unpack<const ScriptArguments&, arg>
	{
		static const ScriptArguments& convert(const ScriptArguments& args)
		{
			return args;
		}
	};

	template <class T, unsigned arg>
	struct unpack<T&, arg>
	{
		static T& convert(const ScriptArguments& args)
		{
			return args.getParameterRef<T>(arg);
		}
	};

	// unpack ScriptObjectType<T>& as non-ref so object is copied
	// by the caller.
	template <class T, unsigned arg>
	struct unpack<ScriptObjectType<T>&, arg>
	{
		static ScriptObjectType<T> convert(const ScriptArguments& args)
		{
			return args.getScriptObject<T>(arg);
		}
	};
	template <class T, unsigned arg>
	struct unpack<ScriptObjectType<T>, arg>
	{
		static ScriptObjectType<T> convert(const ScriptArguments& args)
		{
			return args.getScriptObject<T>(arg);
		}
	};

	template <unsigned depth, unsigned script_arg, class Tret, class...Targs>
	struct call_unpacked
	{
		// This isn't instanciated, just for reference
		template <class Tfunc, class...Tprev>
		Tret call(Tfunc func, const ScriptArguments& args, Tprev&&...prev);
	};

	template<unsigned script_arg, class Tret, class...Targs>
	struct call_unpacked<0, script_arg, Tret, Targs...>
	{
		template <class Tfunc, class...Tprev>
		static Tret call(Tfunc func, const ScriptArguments&, Tprev&&...prev)
		{
			return func(prev...);
		}
	};

	template<unsigned depth, unsigned script_arg, class Tret, class Targ, class...Targs>
	struct call_unpacked<depth, script_arg, Tret, Targ, Targs...>
	{
		template <class Tfunc, class...Tprev>
		static Tret call(Tfunc func, const ScriptArguments& args, Tprev&&...prev)
		{
			constexpr size_t next_arg = script_arg + arg_traits<Targ>::arg_size;
			return call_unpacked<depth-1, next_arg, Tret, Targs...>::call(func, args, prev..., unpack<Targ, script_arg-1>::convert(args));
		}
	};

	/**
	 * Template to unpack ScriptArguments into real types
	 */
	template <class Tret, class...Targs>
	struct binder
	{
		using func_type = Tret(*const)(Targs...);

		template <class Tfunc>
		static void call(Tfunc func, const ScriptArguments& args)
		{
			call_unpacked<sizeof...(Targs), 0, Tret, Targs...>::call(func, args);
		}
	};

	template <class...Targs>
	struct binder<bool, Targs...>
	{

		template <class Tfunc>
		static void call(Tfunc func, const ScriptArguments& args)
		{
			args.getThread()->conditionResult = call_unpacked<sizeof...(Targs), 0, bool, Targs...>::call(func, args);
		}
	};

	template <>
	struct binder<void, const ScriptArguments&>
	{
		template <class Tfunc>
		static void call(Tfunc func, const ScriptArguments& args)
		{
			func(args);
		}
	};

	template <>
	struct binder<bool, const ScriptArguments&>
	{
		template <class Tfunc>
		static void call(Tfunc func, const ScriptArguments& args)
		{
			args.getThread()->conditionResult = func(args);
		}
	};

	template <class Tret, class...Targs>
	void do_unpacked_call(Tret(*const& func)(Targs...), const ScriptArguments& args)
	{
		script_bind::binder<Tret, Targs...>::call(func, args);
	}
}

/**
 * Interface for a collection of functions that can be exported to a game script interface.
 *
 * For example a collection of functions that control the time of day, or create objects would
 * be the collected within one ScriptModule with a sensible name like "Environment" or "Objects"
 */
class ScriptModule
{

public:
	ScriptModule(const std::string& name) : name(name) { }

	const std::string& getName() const { return name; }

	void bind(ScriptFunctionID id,
			  ScriptFunction func,
			  bool conditional,
			  int args,
			  const std::string& name,
			  const std::string& desc
	);

	template <class Tfunc>
	void bind(ScriptFunctionID id,
			  int argc,
			  Tfunc function)
	{
		functions.insert({id, {
							  [=](const ScriptArguments& args) {
								script_bind::do_unpacked_call(function, args);
							  },
							  argc,
							  false,
							  "opcode",
							  ""
						  }});
	}

	bool findOpcode(ScriptFunctionID id, ScriptFunctionMeta** out);

private:
	const std::string name;
	std::map<ScriptFunctionID, ScriptFunctionMeta> functions;
};

template <>
void ScriptModule::bind<void(const ScriptArguments &)>(ScriptFunctionID id, int args,
                                                       void (*function)(const ScriptArguments &));

template<class Tret> ScriptFunction conditional_facade(Tret(*f)(const ScriptArguments&)) { return f; }
template<> ScriptFunction conditional_facade<bool>(bool(*f)(const ScriptArguments&));

template<class Tret> bool is_conditional(Tret(*)(const ScriptArguments&)) { return false; }
template<> bool is_conditional<bool>(bool(*f)(const ScriptArguments&));

// Macro to automatically use function name.
#define bindFunction(id, func, argc, desc) \
	bind(id, argc, func)
#define bindUnimplemented(id, func, argc, desc) \
	bind(id, 0, false, argc, #func, desc)

#endif
