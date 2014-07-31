#pragma once
#ifndef _OPCODESVM_HPP_
#define _OPCODESVM_HPP_
#include <script/ScriptTypes.hpp>

namespace Opcodes {
struct VM {
	static VM& get() {
		static VM vm;
		return vm;
	}

	VM();

	SCMMicrocodeTable codes;
};
}

#endif
