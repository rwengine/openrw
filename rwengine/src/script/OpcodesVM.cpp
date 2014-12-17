#include <script/OpcodesVM.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>

#include <engine/GameWorld.hpp>

#include <iostream>

SCMThread::pc_t localizeLabel(SCMThread* t, int label)
{
	// Negative jump addresses indicate a jump relative to the start of the thread.
	return (label < 0) ? (t->baseAddress + (-label)) : label;
}

VM_OPCODE_DEF( 0x0001 )
{
	t->wakeCounter = p->at(0).integer;
	if( t->wakeCounter == 0 ) {
		t->wakeCounter = -1;
	}
}

VM_OPCODE_DEF( 0x0002 )
{
	t->programCounter = localizeLabel(t, p->at(0).integer);
}

VM_OPCODE_DEF( 0x0004 )
{
	*p->at(0).globalInteger = p->at(1).integer;
}

VM_OPCODE_DEF( 0x0005 )
{
	*p->at(0).globalReal = p->at(1).real;
}
VM_OPCODE_DEF( 0x0006 )
{
	*p->at(0).globalInteger = p->at(1).integer;
}

VM_OPCODE_DEF( 0x0008 )
{
	*p->at(0).globalInteger += p->at(1).integer;
}
VM_OPCODE_DEF( 0x0009 )
{
	*p->at(0).globalReal += p->at(1).real;
}

VM_OPCODE_DEF( 0x000C )
{
	*p->at(0).globalInteger -= p->at(1).integer;
}
VM_OPCODE_DEF( 0x000D )
{
	*p->at(0).globalReal -= p->at(1).real;
}

VM_OPCODE_DEF( 0x0015 )
{
	*p->at(0).globalReal /= p->at(1).real;
}

VM_OPCODE_DEF( 0x0018 ) {
	t->conditionResult = *p->at(0).globalInteger > p->at(1).integer;
}
VM_OPCODE_DEF( 0x0019 ) {
	t->conditionResult =  *p->at(0).globalInteger > p->at(1).integer;
}

VM_OPCODE_DEF( 0x001A ) {
	t->conditionResult =  p->at(0).integer > *p->at(1).globalInteger;
}

VM_OPCODE_DEF( 0x001B ) {
	t->conditionResult =  p->at(0).integer > *p->at(1).globalInteger;
}

VM_OPCODE_DEF( 0x0020 ) {
	t->conditionResult =  *p->at(0).globalReal > p->at(1).real;
}

VM_OPCODE_DEF( 0x0028 ) {
	t->conditionResult =  *p->at(0).globalInteger >= p->at(1).integer;
}
VM_OPCODE_DEF( 0x0029 )
{
	t->conditionResult = *p->at(0).globalInteger >= p->at(1).integer;
}


VM_OPCODE_DEF( 0x002A ) {
	t->conditionResult =  p->at(0).integer >= *p->at(1).globalInteger;
}

VM_OPCODE_DEF( 0x0038 ) {
	t->conditionResult =  *p->at(0).globalInteger == p->at(1).integer;
}

VM_OPCODE_DEF( 0x0039 ) {
	t->conditionResult =  *p->at(0).globalInteger == p->at(1).integer;
}

VM_OPCODE_DEF( 0x004F )
{
	std::cout << t->name << " spawning thread at " << p->at(0).integer << std::endl;
	m->startThread(p->at(0).integer);
}

VM_OPCODE_DEF( 0x004D )
{
	if( ! t->conditionResult ) {
		t->programCounter = localizeLabel(t, p->at(0).integer);
	}
}

VM_OPCODE_DEF( 0x004E )
{
	// ensure the thread is immediately yeilded
	t->wakeCounter = -1;
	t->finished = true;
}

VM_OPCODE_DEF( 0x0050 )
{
	t->calls.push(t->programCounter);
	t->programCounter = localizeLabel(t, p->at(0).integer);
}

VM_OPCODE_DEF( 0x0051 )
{
	t->programCounter = t->calls.top();
	t->calls.pop();
}

VM_OPCODE_DEF( 0x0060 )
{
	*p->at(0).globalInteger -= *p->at(1).globalInteger;
}

VM_OPCODE_DEF( 0x0061 )
{
	*p->at(0).globalReal -= *p->at(1).globalReal;
}

VM_OPCODE_DEF( 0x0084 )
{
	*p->at(0).globalInteger = *p->at(1).globalInteger;
}

VM_OPCODE_DEF( 0x0086 )
{
	*p->at(0).globalReal = *p->at(1).globalReal;
}

VM_OPCODE_DEF( 0x00D6 )
{
	auto n = p->at(0).integer;
	if( n <= 7 ) {
		t->conditionCount = n+1;
		t->conditionMask = 0xFF;
		t->conditionAND = true;
	}
	else {
		t->conditionCount = n-19;
		t->conditionMask = 0x00;
		t->conditionAND = false;
	}
}

VM_OPCODE_DEF( 0x00D7 )
{
	std::cout << "Starting Mission Thread at " << p->at(0).integer << std::endl;
	m->startThread(p->at(0).integer, true);
}

VM_OPCODE_DEF( 0x00D8 )
{
	std::cout << "Ended: " << t->name << std::endl;
	
	for( auto& o : m->getWorld()->state.missionObjects )
	{
		m->getWorld()->destroyObjectQueued(o);
	}
	
	m->getWorld()->state.missionObjects.clear();
	
	*m->getWorld()->state.scriptOnMissionFlag = 0;
}

VM_OPCODE_DEF( 0x02CD )
{
	t->calls.push(t->programCounter);
	t->programCounter = p->at(0).integer;
}

VM_OPCODE_DEF( 0x03A4 )
{
	t->name = p->at(0).string;
	std::cout << "Thread renamed: " << t->name << std::endl;
}

VM_OPCODE_DEF( 0x0417 )
{
	std::cout << "Starting mission no. " << p->at(0).integer << std::endl;
	auto offset = m->getFile()->getMissionOffsets()[p->at(0).integer];
	m->startThread(offset, true);
}

namespace Opcodes {

VM::VM()
{
	VM_OPCODE_DEC( 0x0001, 1, "Wait" );
	VM_OPCODE_DEC( 0x0002, 1, "Jump" );

	VM_OPCODE_DEC( 0x0004, 2, "Set Global Integer" );
	VM_OPCODE_DEC( 0x0005, 2, "Set Global Float" );
	VM_OPCODE_DEC( 0x0006, 2, "Set Local Int" );

	VM_OPCODE_DEC( 0x0008, 2, "Increment Global Int" );
	VM_OPCODE_DEC( 0x0009, 2, "Increment Global Float" );
	VM_OPCODE_DEC( 0x000C, 2, "Decrement Global Int" );
	VM_OPCODE_DEC( 0x000D, 2, "Decrement Global Float" );

	VM_OPCODE_DEC( 0x0015, 2, "Divide Global by Float" );

	VM_OPCODE_DEC( 0x0018, 2, "Global Int Greater than Int" );
	VM_OPCODE_DEC( 0x0019, 2, "Local Int Greater than Int" );

	VM_OPCODE_DEC( 0x001A, 2, "Int Greater Than Global Int" );
	VM_OPCODE_DEC( 0x001B, 2, "Int Greater Than Var Int" );
	
	VM_OPCODE_DEC( 0x0020, 2, "Global Float Greather than Float" );
	
	VM_OPCODE_DEC( 0x0028, 2, "Global Int >= Int" );
	VM_OPCODE_DEC( 0x0029, 2, "Local Int >= Int" );
	
	VM_OPCODE_DEC( 0x002A, 2, "Int >= Global Int" );
	
	VM_OPCODE_DEC( 0x0038, 2, "Global Int Equal to Int" );

	VM_OPCODE_DEC( 0x0039, 2, "Local Int Equal to Int" );

	VM_OPCODE_DEC( 0x004F, -1, "Start New Thread" );

	VM_OPCODE_DEC( 0x004D, 1, "Jump if false" );

	VM_OPCODE_DEC( 0x004E, 0, "End Thread" );

	VM_OPCODE_DEC( 0x0050, 1, "Gosub" );

	VM_OPCODE_DEC( 0x0051, 0, "Return" );

	VM_OPCODE_DEC( 0x0060, 2, "Decrement Global Integer by Global Integer" );
	VM_OPCODE_DEC( 0x0061, 2, "Decrement Global Float by Global Float" );

	VM_OPCODE_DEC( 0x0084, 2, "Set Global Int To Global" );

	VM_OPCODE_DEC( 0x0086, 2, "Set Global Float To Global" );

	VM_OPCODE_DEC( 0x00D6, 1, "If" );

	VM_OPCODE_DEC( 0x00D7, 1, "Start Mission Thread" );

	VM_OPCODE_DEC( 0x00D8, 0, "Set Mission Finished" );

	VM_OPCODE_DEC( 0x02CD, 2, "Call" );

	VM_OPCODE_DEC( 0x03A4, 1, "Name Thread" );

	VM_OPCODE_DEC( 0x0417, 1, "Start Mission" );
}

}
