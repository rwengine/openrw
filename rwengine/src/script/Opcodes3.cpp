#include <script/Opcodes3.hpp>
#include <script/ScriptMachine.hpp>

SCMMicrocodeTable ops_global = {
	{ 0x0002,
	  {
		  "Jump",
		  1,
		  [](ScriptMachine*, SCMThread* t, SCMParams* p)
		  {
			  t->programCounter = p->at(0).integer;
		  }
	  }
	},

	{ 0x0004,
	  {
		  "Set Global Integer",
		  2,
		  [](ScriptMachine*, SCMThread*, SCMParams* p)
		  {
			*p->at(0).globalInteger = p->at(1).integer;
		  }
	  }
	},

	{ 0x0005,
	  {
		  "Set Global Float",
		  2,
		  [](ScriptMachine*, SCMThread*, SCMParams* p)
		  {
			*p->at(0).globalReal = p->at(1).real;
		  }
	  }
	},

	{ 0x0015,
	  {
		  "Divide Global by Float",
		  2,
		  [](ScriptMachine*, SCMThread*, SCMParams* p)
		  {
			*p->at(0).globalReal /= p->at(1).real;
		  }
	  }
	},

	{ 0x0086,
	  {
		  "Set Global Float To Global",
		  2,
		  [](ScriptMachine*, SCMThread*, SCMParams* p)
		  {
			*p->at(0).globalReal = *p->at(1).globalReal;
		  }
	  }
	},

	{ 0x03A4,
	  {
		  "Name Thread",
		  1,
		  [](ScriptMachine*, SCMThread* t, SCMParams* p)
		  {
			  t->name = p->at(0).string;
		  }
	  }
	},

};

#define SCM_FUNC(c) [](ScriptMachine* m, SCMThread* t, SCMParams* p) c

#include <iostream>

SCMMicrocodeTable ops_game = {

	{ 0x042C,
	  { "Set Total Missions", 1,
		SCM_FUNC({
			std::cout << "Total Missions: " << p->at(0).integer << std::endl;
		})
	  }
	}
};



Opcodes3::Opcodes3()
{
	codes.insert(ops_global.begin(), ops_global.end());
	codes.insert(ops_game.begin(), ops_game.end());
}
