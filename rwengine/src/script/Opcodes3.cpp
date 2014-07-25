#include <script/Opcodes3.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <ai/PlayerController.hpp>


#include <glm/gtx/string_cast.hpp>
#include <iostream>

#define OPC(code, name, params, func) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) func } },
#define OPC_COND(code, name, params, func) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { t->conditionResult = ([=]() {func})(); } } },
#define OPC_UNIMPLEMENTED_CRITICAL(code, name, params) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { throw UnimplementedOpcode(code, *p); } } },
#define OPC_UNIMPLEMENTED_MSG(code, name, params) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { std::cout << #code << " " << #name << " unimplemented" << std::endl; } } },

SCMMicrocodeTable ops_global = {
	OPC( 0x0001, "Wait", 1,	{
		t->wakeCounter = p->at(0).integer;
		if( t->wakeCounter == 0 ) {
			t->wakeCounter = -1;
		}
	})

	OPC( 0x0002, "Jump", 1, {
		t->programCounter = p->at(0).integer;
	})

	OPC( 0x0004, "Set Global Integer", 2, {
		*p->at(0).globalInteger = p->at(1).integer;
	})

	OPC( 0x0005, "Set Global Float", 2, {
		*p->at(0).globalReal = p->at(1).real;
	})

	OPC( 0x0015, "Divide Global by Float", 2, {
		*p->at(0).globalReal /= p->at(1).real;
	})

	OPC_COND( 0x0038, "Global Int Equal to Int", 2, {
		return *p->at(0).globalInteger == p->at(1).integer;
	})

	OPC_COND( 0x0039, "Local Int Equal to Int", 2, {
		std::cout << *p->at(0).globalInteger << " == " << p->at(1).integer << std::endl;
		return *p->at(0).globalInteger == p->at(1).integer;
	})

	OPC( 0x004F, "Start New Thread", -1, {
		std::cout << "Starting thread at " << p->at(0).integer << std::endl;
		m->startThread(p->at(0).integer);
	})

	OPC( 0x004D, "Jump if false", 1, {
		if( ! t->conditionResult ) {
			t->programCounter = p->at(0).integer;
		}
	})

	OPC( 0x004E, "End Thread", 0, {
		// ensure the thread is immediately yeilded
		t->wakeCounter = -1;
		t->finished = true;
	})

	OPC( 0x0051, "Return", 0, {
		std::cout << "Return <= " << t->calls.top() << std::endl;
		t->programCounter = t->calls.top();
		t->calls.pop();
	})

	OPC( 0x0084, "Set Global Int To Global", 2, {
		*p->at(0).globalInteger = *p->at(1).globalInteger;
	})

	OPC( 0x0086, "Set Global Float To Global", 2, {
		*p->at(0).globalReal = *p->at(1).globalReal;
	})

	OPC( 0x00D6, "If", 1, {
		auto n = p->at(0).integer;
		if( n <= 7 ) {
			t->conditionCount = n+1;
			t->conditionMask = 0xFF;
		}
		else {
			t->conditionCount = n-7;
			t->conditionMask = 0x00;
		}
	})

	OPC( 0x00D7, "Start Mission Thread", 1, {
		std::cout << "Starting Mission Thread at " << p->at(0).integer << std::endl;
		m->startThread(p->at(0).integer, true);
	})

	OPC( 0x02CD, "Call", 2, {
		std::cout << "Call => " << std::hex << p->at(0).integer << std::endl;
		t->calls.push(t->programCounter);
		t->programCounter = p->at(0).integer;
	})

	OPC( 0x03A4, "Name Thread", 1, {
		t->name = p->at(0).string;
	})

	OPC( 0x0417, "Start Mission", 1, {
		std::cout << "Starting mission no. " << p->at(0).integer << std::endl;
		auto offset = m->getFile()->getMissionOffsets()[p->at(0).integer];
		m->startThread(offset, true);
	})
};

SCMMicrocodeTable ops_game = {
	OPC( 0x0053, "Create Player", 5, {
		auto id	= p->at(0).integer;
		glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

		auto pc = m->getWorld()->createPedestrian(1, {-1000.f, -990.f, 13.f});
		m->getWorld()->state.player = new PlayerController(pc);

		*p->at(4).handle = m->getWorld()->state.player;
	})

	OPC_COND( 0x00DE, "Is Player In Model", 2, {
		auto& vdata = m->getWorld()->vehicleTypes[p->at(1).integer];
		auto controller = (CharacterController*)(*p->at(0).handle);
		auto character = controller->getCharacter();
		auto vehicle = character->getCurrentVehicle();
		if ( vehicle ) {
			return vehicle->model && vdata->modelName == vehicle->model->name;
		}
		return false;
	})

	OPC_COND( 0x00E1, "Is Button Pressed", 2, {
		/// @todo implement
		return false;
	})

	OPC( 0x0111, "Set Dead or Arrested", 1, {
		*m->getWorld()->state.scriptOnMissionFlag = p->at(0).integer;
	})

	OPC_UNIMPLEMENTED_MSG( 0x0118, "Is Character Dead", 1)

	OPC_UNIMPLEMENTED_MSG( 0x014B, "Create Car Generator", 13 )

	// 0 -> disable, 1-100 -> number, 101+ -> always
	OPC_UNIMPLEMENTED_MSG( 0x014C, "Set Car Generator count", 2)

	OPC_UNIMPLEMENTED_MSG( 0x0152, "Set zone car info", 17)

	OPC_UNIMPLEMENTED_MSG( 0x015C, "Set zone ped info", 11)

	OPC_UNIMPLEMENTED_MSG( 0x0164, "Disable Radar Blip", 1)

	OPC( 0x0169, "Set Fade Colour", 3, {
		m->getWorld()->state.fadeColour.r = p->at(0).integer;
		m->getWorld()->state.fadeColour.g = p->at(1).integer;
		m->getWorld()->state.fadeColour.b = p->at(2).integer;
	})

	OPC( 0x016A, "Fade Screen", 2, {
		m->getWorld()->state.fadeIn = !!p->at(0).integer;
		m->getWorld()->state.fadeTime = p->at(1).integer / 1000.f;
		m->getWorld()->state.fadeEnabled = true;
	})

	OPC( 0x0171, "Set Player Heading", 2, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		auto hdg = p->at(1).real;
		controller->getCharacter()->setRotation(glm::quat(hdg, glm::vec3(0.f, 0.f, 1.f)));
	})

	OPC( 0x0177, "Set Object heading", 2, {
		auto inst = (InstanceObject*)(*p->at(0).handle);
		auto hdg = p->at(1).real;
		inst->setRotation(glm::quat(hdg, glm::vec3(0.f, 0.f, 1.f)));
	})

	OPC( 0x0180, "Link ONMISSION Flag", 1, {
		m->getWorld()->state.scriptOnMissionFlag = (unsigned int*)p->at(0).globalInteger;
	})
	OPC_UNIMPLEMENTED_MSG( 0x0181, "Link Character Mission Flag", 2 )
	OPC_UNIMPLEMENTED_MSG( 0x0182, "Unknown Character Opcode", 2 )

	OPC_UNIMPLEMENTED_MSG( 0x018D, "Create soundscape", 5)

	OPC( 0x01B4, "Set Player Input Enabled", 2, {
		auto controller = static_cast<PlayerController*>(*p->at(0).handle);
		controller->setInputEnabled(!!p->at(1).integer);
	})

	OPC( 0x01C7, "Don't remove object", 1, {
		auto inst = (InstanceObject*)(*p->at(0).handle);
		std::cout << "Unable to pin object " << inst << ". Object pinning unimplimented" << std::endl;
	})

	OPC_UNIMPLEMENTED_MSG( 0x01E7, "Enable Roads", 6)
	OPC_UNIMPLEMENTED_MSG( 0x01E8, "Disable Roads", 6)

	OPC( 0x01F0, "Set Max Wanted Level", 1, {
		m->getWorld()->state.maxWantedLevel = p->at(0).integer;
	})

	// This does nothing for us.
	OPC( 0x01F5, "Get Player Character", 2, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		*p->at(1).handle = controller;
	})

	/// @todo http://gtag.gtagaming.com/opcode-database/opcode/0213/
	OPC_UNIMPLEMENTED_MSG( 0x0213, "Create pickup", 6)
	OPC_COND( 0x0214, "Has Pickup been collected", 1, {
		std::cout << "0x0214 Unimplemented" << std::endl;
		return false;
	})

	OPC( 0x0219, "Create Garage", 8, {
		glm::vec3 min(p->at(0).real, p->at(1).real, p->at(2).real);
		glm::vec3 max(p->at(3).real, p->at(4).real, p->at(5).real);

		/// @todo http://www.gtamodding.com/index.php?title=Garage#GTA_III
		int garageType = p->at(6).integer;
		auto garageHandle = p->at(7).handle;

		std::cout << "Garages Unimplemented. type " << garageType << std::endl;
	})

	OPC_UNIMPLEMENTED_MSG( 0x022B, "Disable ped paths", 6)

	OPC_UNIMPLEMENTED_MSG( 0x0236, "Set Gang Car", 2 )
	OPC_UNIMPLEMENTED_MSG( 0x0237, "Set Gang Weapons", 3 )

	OPC_UNIMPLEMENTED_MSG( 0x023C, "Load Special Character", 2)

	OPC_UNIMPLEMENTED_MSG( 0x024A, "Get Phone Near", 3)

	OPC_COND( 0x0248, "Is Model Loaded", 1, {
		/// @todo this will need changing when model loading is overhauled.
		//auto model = m->getFile()->getModels()[p->at(0).integer];
		//if( model == "" ) return true; // ??
		return false;
	})

	/// @todo http://www.gtamodding.com/index.php?title=0256 (e.g. check if dead or busted)
	OPC_COND( 0x0256, "Is Player Playing", 1, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		return controller != nullptr;
	})

	OPC( 0x0293, "Get Controller Mode", 1, {
		*p->at(0).globalInteger	= 0;
	})

	OPC( 0x029B, "Create Object no offset", 5, {
		auto id = p->at(0).integer;
		auto& object = m->getWorld()->objectTypes[id];
		glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

		auto inst = m->getWorld()->createInstance(object->ID, position);

		/// @todo this will fall over due to 64-bit pointers.
		*p->at(4).handle = inst;
	})

	OPC_UNIMPLEMENTED_MSG( 0x02A7, "Add Radar Contact Blip", 5)
	OPC_UNIMPLEMENTED_MSG( 0x02A8, "Add Radar Blip", 5)

	OPC_UNIMPLEMENTED_MSG( 0x02DE, "Is Player In Taxi", 1)

	OPC_UNIMPLEMENTED_MSG( 0x02EC, "Create Hidden Package", 3 )

	OPC( 0x02ED, "Set Total Hidden Packages", 1, {
		m->getWorld()->state.numHiddenPackages = p->at(0).integer;
	})

	/// @todo parse HIER section for this opcode
	OPC_UNIMPLEMENTED_MSG( 0x02F3, "Load Special Model", 2)

	OPC_UNIMPLEMENTED_MSG( 0x02FB, "Create Crusher Crane", 10)

	OPC( 0x030D, "Set Max Progress", 1, {
		m->getWorld()->state.maxProgress = p->at(0).integer;
	})

	OPC( 0x0314, "Set Total Unique Jumps", 1, {
		m->getWorld()->state.numUniqueJumps = p->at(0).integer;
	})

	OPC_UNIMPLEMENTED_MSG( 0x0324, "Set zone ped group", 3)
	OPC_UNIMPLEMENTED_MSG( 0x0325, "Create Car Fire", 2)

	OPC_UNIMPLEMENTED_MSG( 0x032B, "Create Weapon Pickup", 7)

	OPC_UNIMPLEMENTED_MSG( 0x0336, "Set Player Visible", 2)

	OPC_UNIMPLEMENTED_MSG( 0x0352, "Set Character Model", 2)

	OPC_UNIMPLEMENTED_MSG( 0x035D, "Set Object Targetable", 1)

	OPC_UNIMPLEMENTED_MSG( 0x0363, "Set Closest Object Visibility", 6)

	OPC_UNIMPLEMENTED_MSG( 0x0368, "Create ev Crane", 10)

	OPC_UNIMPLEMENTED_MSG( 0x0399, "Disable ped paths in angled cube", 7)

	OPC_UNIMPLEMENTED_MSG( 0x03AD, "Set Garbage Enabled", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03AF, "Set Map Streaming Enabled", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03B6, "Change Nearest Instance Model", 6)

	OPC_UNIMPLEMENTED_MSG( 0x03BB, "Set Garage Door to Rotate", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03BF, "Set Pedestrians Ignoring Player", 2)

	OPC_UNIMPLEMENTED_MSG( 0x03DA, "Set Garage Camera Follows Player", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03E5, "Display Help Text", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03EF, "Make Player Safe For Cutscene", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03F1, "Set Ped Hostility", 2 )
	OPC_UNIMPLEMENTED_MSG( 0x03F2, "Clear Ped Hostility", 2 )

	OPC_UNIMPLEMENTED_MSG( 0x03F7, "Load Collision", 1 )

	OPC( 0x0408, "Set Total Rampage Missions", 1, {
		m->getWorld()->state.numRampages = p->at(0).integer;
	})

	/// @todo http://gtag.gtagaming.com/opcode-database/opcode/0426/
	OPC_UNIMPLEMENTED_MSG( 0x0426, "Create Save Cars Between Levels cube", 6)

	OPC( 0x042C, "Set Total Missions", 1, {
		m->getWorld()->state.numMissions = p->at(0).integer;
	})

	OPC( 0x043D, "Set Is Intro Playing", 1, {
		m->getWorld()->state.isIntroPlaying = !!p->at(0).integer;
	})
};



Opcodes3::Opcodes3()
{
	codes.insert(ops_global.begin(), ops_global.end());
	codes.insert(ops_game.begin(), ops_game.end());
}
