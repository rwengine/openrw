#pragma once
#ifndef _SCRIPTDISASSEMBLY_HPP_
#define _SCRIPTDISASSEMBLY_HPP_
#include <script/ScriptTypes.hpp>

class SCMFile;

/**
 * Extracts instruction level information from a SCM file
 */
class ScriptDisassembly
{
public:

	/**
	 * Information about a single call to a single opcode and
	 * it's parameters
	 */
	struct InstructionInfo
	{
		SCMOpcode opcode;
		SCMParams parameters;
	};

	ScriptDisassembly(SCMOpcodes* codes, SCMFile* scm);

	/**
	 * Execute the disassembly routine.
	 * 
	 * If there is an error during disassembly, an exeption will be
	 * thrown
	 */
	void disassemble(SCMAddress startAddress);

	std::map<SCMAddress, InstructionInfo>& getInstructions() { return instructions; }

private:

	SCMOpcodes* codes;
	SCMFile* scm;

	std::map<SCMAddress, InstructionInfo> instructions;
};

#endif