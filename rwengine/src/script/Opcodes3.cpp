#include <script/Opcodes3.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <render/Model.hpp>
#include <engine/Animator.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <objects/CutsceneObject.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>

#define OPC(code, name, params, func) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) func } },
#define OPC_COND(code, name, params, func) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { t->conditionResult = ([=]() {func})(); } } },
#define OPC_UNIMPLEMENTED_CRITICAL(code, name, params) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { throw UnimplementedOpcode(code, *p); } } },
#define OPC_UNIMPLEMENTED_MSG(code, name, params) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { std::cout << #code << " " << #name << " unimplemented" << std::endl; } } },

SCMThread::pc_t	localizeLabel(SCMThread* t, int label)
{
	// Negative jump addresses indicate a jump relative to the start of the thread.
	return (label < 0) ? (t->baseAddress + (-label)) : label;
}

SCMMicrocodeTable ops_global = {
	OPC( 0x0001, "Wait", 1, {
		t->wakeCounter = p->at(0).integer;
		if( t->wakeCounter == 0 ) {
			t->wakeCounter = -1;
		}
	})

	OPC( 0x0002, "Jump", 1, {
		t->programCounter = localizeLabel(t, p->at(0).integer);
	})

	OPC( 0x0004, "Set Global Integer", 2, {
		*p->at(0).globalInteger = p->at(1).integer;
	})

	OPC( 0x0005, "Set Global Float", 2, {
		*p->at(0).globalReal = p->at(1).real;
	})
	OPC( 0x0006, "Set Local Int", 2, {
		*p->at(0).globalInteger = p->at(1).integer;
	})

	OPC( 0x0008, "Increment Global Int", 2, {
		*p->at(0).globalInteger += p->at(1).integer;
	})
	OPC( 0x0009, "Increment Global Float", 2, {
		*p->at(0).globalReal += p->at(1).real;
	})

	OPC( 0x000C, "Decrement Global Int", 2, {
		*p->at(0).globalInteger -= p->at(1).integer;
	})
	OPC( 0x000D, "Decrement Global Float", 2, {
		*p->at(0).globalReal -= p->at(1).real;
	})

	OPC( 0x0015, "Divide Global by Float", 2, {
		*p->at(0).globalReal /= p->at(1).real;
	})

	OPC_COND( 0x0018, "Global Int Greater than Int", 2, {
		return *p->at(0).globalInteger > p->at(1).integer;
	})
	OPC_COND( 0x0019, "Local Int Greater than Int", 2, {
		return *p->at(0).globalInteger > p->at(1).integer;
	})

	OPC_COND( 0x001A, "Int Greater Than Global Int", 2, {
		return p->at(0).integer > *p->at(1).globalInteger;
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
			t->programCounter = localizeLabel(t, p->at(0).integer);
		}
	})

	OPC( 0x004E, "End Thread", 0, {
		// ensure the thread is immediately yeilded
		t->wakeCounter = -1;
		t->finished = true;
	})

	OPC( 0x0050, "Gosub", 1, {
		t->calls.push(t->programCounter);
		t->programCounter = localizeLabel(t, p->at(0).integer);
	})

	OPC( 0x0051, "Return", 0, {
		t->programCounter = t->calls.top();
		t->calls.pop();
	})

	OPC( 0x0061, "Decrement Global Float by Global Float", 2, {
		*p->at(0).globalReal -= *p->at(1).globalReal;
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

	OPC( 0x00D8, "Set Mission Finished", 0, {
		std::cout << "Ended: " << t->name << std::endl;
		*m->getWorld()->state.scriptOnMissionFlag = 0;
	})

	OPC( 0x02CD, "Call", 2, {
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

glm::vec3 spawnMagic( 0.f, 0.f, 1.f );

SCMMicrocodeTable ops_game = {
	OPC( 0x0053, "Create Player", 5, {
		auto id	= p->at(0).integer;
		glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

		if( position.z < -99.f ) {
			position = m->getWorld()->getGroundAtPosition(position);
		}

		auto pc = m->getWorld()->createPedestrian(1, position + spawnMagic);
		m->getWorld()->state.player = new PlayerController(pc);

		*p->at(4).handle = m->getWorld()->state.player;
	})

	OPC( 0x0055, "Set Player Position", 4, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real + 1.f);
		controller->getCharacter()->setPosition(position + spawnMagic);
	})
	OPC_COND( 0x0056, "Is In Area", 6, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		glm::vec2 min(p->at(1).real, p->at(2).real);
		glm::vec2 max(p->at(3).real, p->at(4).real);
		bool drawMarker = !!p->at(5).integer;
		auto player = controller->getCharacter()->getPosition();
		if( player.x > min.x && player.y > min.y && player.x < max.x && player.y < max.y ) {
			return true;
		}
		return false;
	})

	OPC( 0x009A, "Create Character", 6, {
		auto type = p->at(0).integer;
		auto id	= p->at(1).integer;
		glm::vec3 position(p->at(2).real, p->at(3).real, p->at(4).real);

		if( type == 21 ) {

		}
		if( position.z < -99.f ) {
			position = m->getWorld()->getGroundAtPosition(position);
		}

		auto character = m->getWorld()->createPedestrian(id, position + spawnMagic);
		auto controller = new DefaultAIController(character);

		*p->at(5).handle = controller;
	})

	OPC( 0x00A5, "Create Vehicle", 5, {
		auto id	= p->at(0).integer;
		glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

		auto vehicle = m->getWorld()->createVehicle(id, position + spawnMagic);

		*p->at(4).handle = vehicle;
	})

	OPC( 0x00BC, "Print Message Now", 3, {
		std::string str(p->at(0).string);
		str = m->getWorld()->gameData.texts.text(str);
		m->getWorld()->state.osTextString = str;
		m->getWorld()->state.osTextTime = p->at(1).integer / 1000.f;
		m->getWorld()->state.osTextStart= m->getWorld()->gameTime;
		m->getWorld()->state.osTextStyle = p->at(2).integer;
	})

	OPC( 0x00BE, "Clear Message Prints", 0, {
		m->getWorld()->state.osTextTime = 0.f;
		m->getWorld()->state.osTextStart= 0.f;
	})

	OPC( 0x00C0, "Set Time Of Day", 2, {
		m->getWorld()->state.hour = p->at(0).integer;
		m->getWorld()->state.minute = p->at(1).integer;
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

	OPC_COND( 0x00E0, "Is Player In Any Vehicle", 1, {
		auto controller = static_cast<CharacterController*>(*p->at(0).handle);

		auto vehicle = controller->getCharacter()->getCurrentVehicle();
		return vehicle != nullptr;
	})

	OPC_COND( 0x00E1, "Is Button Pressed", 2, {
		/// @todo implement
		return false;
	})

	OPC_COND( 0x0100, "Is Character near point in car", 8, {
		auto controller = static_cast<CharacterController*>(*p->at(0).handle);
		glm::vec3 center(p->at(1).real, p->at(2).real, p->at(3).real);
		glm::vec3 size(p->at(4).real, p->at(5).real, p->at(6).real);
		bool unkown	= !!p->at(7).integer;

		auto vehicle = controller->getCharacter()->getCurrentVehicle();
		if( vehicle ) {
			auto distance = center - controller->getCharacter()->getPosition();
			distance /= size;
			if( glm::length( distance ) < 1.f ) return true;
		}

		return false;
	})

	OPC( 0x0111, "Set Dead or Arrested", 1, {
		*m->getWorld()->state.scriptOnMissionFlag = p->at(0).integer;
	})

	OPC_COND( 0x0118, "Is Character Dead", 1, {
		auto controller = static_cast<CharacterController*>(*p->at(0).handle);
		return !controller->getCharacter()->isAlive();
	})

	OPC_COND( 0x0121, "Is Player In Zone", 2, {
		auto controller = static_cast<CharacterController*>(*p->at(0).handle);
		std::string zname(p->at(1).string);

		auto zfind = m->getWorld()->zones.find(zname);
		if( zfind != m->getWorld()->zones.end() ) {
			auto player = controller->getCharacter()->getPosition();
			auto& min = zfind->second.min;
			auto& max = zfind->second.max;
			if( player.x > min.x && player.y > min.y && player.z > min.z &&
				player.x < max.x && player.y < max.y && player.z < max.z ) {
				std::cout << "Player is in zone! " << zfind->second.name << std::endl;
			}
		}

		return false;
	})

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
		m->getWorld()->state.fadeTime = p->at(0).integer / 1000.f;
		m->getWorld()->state.fadeOut = !!p->at(1).integer;
		m->getWorld()->state.fadeStart = m->getWorld()->gameTime;
		std::cout << "Fade " << p->at(0).integer << " " << p->at(1).integer << std::endl;
	})
	OPC_COND( 0x016B, "Is Screen Fading", 0, {
		return m->getWorld()->gameTime <
			m->getWorld()->state.fadeStart + m->getWorld()->state.fadeTime;
	})

	OPC( 0x0171, "Set Player Heading", 2, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		controller->getCharacter()->setHeading(p->at(1).real);
	})

	OPC( 0x0173, "Set Character Heading", 2, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		controller->getCharacter()->setHeading(p->at(1).real);
	})

	OPC( 0x0175, "Set Vehicle heading", 2, {
		auto vehicle = (VehicleObject*)(*p->at(0).handle);
		vehicle->setHeading(p->at(1).real);
	})

	OPC( 0x0177, "Set Object heading", 2, {
		auto inst = (InstanceObject*)(*p->at(0).handle);
		inst->setHeading(p->at(1).real);
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

	OPC( 0x01B6, "Set Weather Now", 1, {
		m->getWorld()->state.currentWeather = p->at(0).integer;
	})


	OPC_UNIMPLEMENTED_MSG( 0x01BE, "Turn Character To Face Point", 4)

	OPC( 0x01C7, "Don't remove object", 1, {
		auto inst = (InstanceObject*)(*p->at(0).handle);
		std::cout << "Unable to pin object " << inst << ". Object pinning unimplimented" << std::endl;
	})

	OPC_UNIMPLEMENTED_MSG( 0x01E7, "Enable Roads", 6)
	OPC_UNIMPLEMENTED_MSG( 0x01E8, "Disable Roads", 6)

	OPC_UNIMPLEMENTED_MSG( 0x01ED, "Clear Character Threat Search", 1)

	OPC( 0x01F0, "Set Max Wanted Level", 1, {
		m->getWorld()->state.maxWantedLevel = p->at(0).integer;
	})

	// This does nothing for us.
	OPC( 0x01F5, "Get Player Character", 2, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		*p->at(1).handle = controller;
	})

	OPC_COND( 0x0204, "Is Char near Car in Car 2D", 5, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		auto vehicle = (VehicleObject*)(*p->at(1).handle);
		glm::vec2 radius(p->at(2).real, p->at(3).real);
		bool drawMarker = !!p->at(4).integer;

		auto charVehicle = controller->getCharacter()->getCurrentVehicle();
		if( charVehicle ) {
			auto dist = charVehicle->getPosition() - vehicle->getPosition();
			if( dist.x <= radius.x && dist.y <= radius.y ) {
				return true;
			}
		}

		return false;
	})

	/// @todo http://gtag.gtagaming.com/opcode-database/opcode/0213/
	OPC_UNIMPLEMENTED_MSG( 0x0213, "Create pickup", 6)
	OPC_COND( 0x0214, "Has Pickup been collected", 1, {
		/// @todo implement pls
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

	OPC( 0x0229, "Set Vehicle Colours", 3, {
		auto vehicle = (VehicleObject*)(*p->at(0).handle);

		auto& colours = m->getWorld()->gameData.vehicleColours;
		vehicle->colourPrimary = colours[p->at(1).integer];
		vehicle->colourSecondary = colours[p->at(2).integer];
	})


	OPC_UNIMPLEMENTED_MSG( 0x022B, "Disable ped paths", 6)

	OPC_UNIMPLEMENTED_MSG( 0x022D, "Set Character Always Face Player", 2)

	OPC_UNIMPLEMENTED_MSG( 0x0236, "Set Gang Car", 2 )
	OPC_UNIMPLEMENTED_MSG( 0x0237, "Set Gang Weapons", 3 )

	OPC( 0x023C, "Load Special Character", 2, {
		m->getWorld()->loadSpecialCharacter(p->at(0).integer, p->at(1).string);
	})
	OPC_COND( 0x023D, "Is Special Character Loaded", 1, {
		auto chfind = m->getWorld()->pedestrianTypes.find(p->at(0).integer);
		if( chfind != m->getWorld()->pedestrianTypes.end() ) {
			auto modelfind = m->getWorld()->gameData.models.find(chfind->second->modelName);
			if( modelfind != m->getWorld()->gameData.models.end() && modelfind->second->model != nullptr ) {
				return true;
			}
		}

		return false;
	})

	OPC( 0x0244, "Set Cutscene Offset", 3, {
		glm::vec3 position(p->at(0).real, p->at(1).real, p->at(2).real);
		if( m->getWorld()->state.currentCutscene ) {
			m->getWorld()->state.currentCutscene->meta.sceneOffset = position;
		}
	})
	OPC_UNIMPLEMENTED_MSG( 0x0245, "Set Character Animation Group", 2)

	OPC_UNIMPLEMENTED_MSG( 0x0247, "Request Model Loaded", 1)

	OPC_UNIMPLEMENTED_MSG( 0x024A, "Get Phone Near", 3)

	OPC_COND( 0x0248, "Is Model Loaded", 1, {
		/// @todo this will need changing when model loading is overhauled.
		if( p->at(0).integer == 0 ) {
			/// @todo Figure out if this really does mean the player.
			return true;
		}
		//auto model = m->getFile()->getModels()[p->at(0).integer];
		//if( model == "" ) return true; // ??
		return true;
	})
	OPC_UNIMPLEMENTED_MSG( 0x0249, "Mark Model As Unneeded", 1)

	OPC_UNIMPLEMENTED_MSG( 0x0250, "Create Light", 6)

	/// @todo http://www.gtamodding.com/index.php?title=0256 (e.g. check if dead or busted)
	OPC_COND( 0x0256, "Is Player Playing", 1, {
		auto controller = (CharacterController*)(*p->at(0).handle);
		return controller != nullptr;
	})

	OPC( 0x0293, "Get Controller Mode", 1, {
		*p->at(0).globalInteger	= 0;
	})

	OPC_UNIMPLEMENTED_MSG( 0x0296, "Unload Special Character", 1)

	OPC( 0x029B, "Create Object no offset", 5, {
		auto id = p->at(0).integer;
		auto& object = m->getWorld()->objectTypes[id];
		glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

		auto inst = m->getWorld()->createInstance(object->ID, position);

		*p->at(4).handle = inst;
	})

	OPC_UNIMPLEMENTED_MSG( 0x02A7, "Add Radar Contact Blip", 5)
	OPC_UNIMPLEMENTED_MSG( 0x02A8, "Add Radar Blip", 5)

	OPC_COND( 0x02DE, "Is Player In Taxi", 1, {
		auto controller = static_cast<CharacterController*>(*p->at(0).handle);

		auto vehicle = controller->getCharacter()->getCurrentVehicle();
		return vehicle && (vehicle->vehicle->classType & VehicleData::TAXI) == VehicleData::TAXI;
	})

	OPC( 0x02E4, "Load Cutscene Data", 1, {
		m->getWorld()->loadCutscene(p->at(0).string);
		m->getWorld()->state.cutsceneStartTime = -1.f;
	})
	OPC( 0x02E5, "Create Cutscene Object", 2, {
		auto id	= p->at(0).integer;

		GameObject* object = object = m->getWorld()->createCutsceneObject(id, m->getWorld()->state.currentCutscene->meta.sceneOffset );
		*p->at(1).handle = object;

		if( object == nullptr ) {
			std::cerr << "Could not create cutscene object " << id << std::endl;
		}
	})
	OPC( 0x02E6, "Set Cutscene Animation", 2, {
		GameObject* object = static_cast<GameObject*>(*p->at(0).handle);
		std::string animName = p->at(1).string;
		std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
		Animation* anim = m->getWorld()->gameData.animations[animName];
		if( anim ) {
			object->animator->setModel(object->model->model);
			object->animator->setAnimation(anim, false);
		}
		else {
			std::cerr << "Failed to find cutscene animation: " << animName << std::endl;
		}
	})
	OPC( 0x02E7, "Start Cutscene", 0, {
		m->getWorld()->state.cutsceneStartTime = m->getWorld()->gameTime;
	})
	OPC( 0x02E8, "Get Cutscene Time", 1, {
		float time = m->getWorld()->gameTime - m->getWorld()->state.cutsceneStartTime;
		*p->at(0).globalInteger = time * 1000;
	})
	OPC_COND( 0x02E9, "Is Cutscene Over", 0, {
		if( m->getWorld()->state.currentCutscene ) {
			float time = m->getWorld()->gameTime - m->getWorld()->state.cutsceneStartTime;
			return time > m->getWorld()->state.currentCutscene->tracks.duration;
		}
		return true;
	})
	OPC( 0x02EA, "Clear Cutscene", 0, {
		m->getWorld()->clearCutscene();
	})

	OPC_UNIMPLEMENTED_MSG( 0x02EC, "Create Hidden Package", 3 )

	OPC( 0x02ED, "Set Total Hidden Packages", 1, {
		m->getWorld()->state.numHiddenPackages = p->at(0).integer;
	})

	OPC( 0x02F3, "Load Special Model", 2, {
		m->getWorld()->loadSpecialModel(p->at(0).integer, p->at(1).string);
	})
	OPC( 0x02F4, "Create Cutscene Actor Head", 3, {
		auto id = p->at(1).integer;
		auto actor = static_cast<GameObject*>(*p->at(0).handle);
		CutsceneObject* object = m->getWorld()->createCutsceneObject(id, m->getWorld()->state.currentCutscene->meta.sceneOffset );

		auto headframe = actor->model->model->findFrame("shead");
		actor->animator->setFrameVisibility(headframe, false);
		object->setParentActor(actor, headframe);

		*p->at(2).handle = object;
	})
	OPC( 0x02F5, "Set Cutscene Head Animation", 2,
	{
		GameObject* object = static_cast<GameObject*>(*p->at(0).handle);
		std::string animName = p->at(1).string;
		std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
		Animation* anim = m->getWorld()->gameData.animations[animName];
		if( anim ) {
			object->animator->setModel(object->model->model);
			object->animator->setAnimation(anim, false);
		}
		else {
			std::cerr << "Failed to find cutscene animation: " << animName << std::endl;
		}
	})

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

	OPC_UNIMPLEMENTED_MSG( 0x033E, "Display Text", 3)
	OPC_UNIMPLEMENTED_MSG( 0x033F, "Set Text Scale", 2)
	OPC_UNIMPLEMENTED_MSG( 0x0340, "Set Text Colour", 4)
	OPC_UNIMPLEMENTED_MSG( 0x0341, "Set Text Justify", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0342, "Set Text Centered", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0344, "Set Center Text Size", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0345, "Set Text Background", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0346, "Set Text Background Colour", 4)

	OPC_UNIMPLEMENTED_MSG( 0x0348, "Set Text Size Proportional", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0349, "Set Text Font", 1)

	OPC( 0x0352, "Set Character Model", 2,
	{
		auto controller = static_cast<CharacterController*>(*p->at(0).handle);
		controller->getCharacter()->changeCharacterModel(p->at(1).string);
	})
	OPC_UNIMPLEMENTED_MSG( 0x0353, "Refresh Actor Model", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0354, "Start Chase Scene", 1)
	OPC_UNIMPLEMENTED_MSG( 0x0355, "Stop Chase Scene", 0)

	OPC_UNIMPLEMENTED_MSG( 0x035D, "Set Object Targetable", 1)

	OPC_UNIMPLEMENTED_MSG( 0x0363, "Set Closest Object Visibility", 6)

	OPC_UNIMPLEMENTED_MSG( 0x0368, "Create ev Crane", 10)

	OPC_UNIMPLEMENTED_MSG( 0x0373, "Set Camera Behind Player", 0)
	OPC_UNIMPLEMENTED_MSG( 0x0374, "Set Motion Blur", 1)

	OPC_UNIMPLEMENTED_MSG( 0x038B, "Load Requested Models Now", 0)

	OPC_UNIMPLEMENTED_MSG( 0x0399, "Disable ped paths in angled cube", 7)

	OPC_UNIMPLEMENTED_MSG( 0x039D, "Scatter Particles", 12)

	OPC_UNIMPLEMENTED_MSG( 0x03AD, "Set Garbage Enabled", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03AF, "Set Map Streaming Enabled", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03B6, "Change Nearest Instance Model", 6)
	OPC_UNIMPLEMENTED_MSG( 0x03B7, "Process Cutscene Only", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03BB, "Set Garage Door to Rotate", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03BF, "Set Pedestrians Ignoring Player", 2)

	OPC_COND( 0x03C6, "Is Collision In Memory", 1, {
		/// @todo verify if this is against the global models or the SCM models
		auto model = m->getFile()->getModels()[p->at(0).integer];
		auto phyit = m->getWorld()->gameData.collisions.find(model);

		return phyit != m->getWorld()->gameData.collisions.end();
	})

	OPC_UNIMPLEMENTED_MSG( 0x03CB, "Load Area Near", 3)

	OPC_UNIMPLEMENTED_MSG( 0x03DA, "Set Garage Camera Follows Player", 1)

	OPC( 0x03E1, "Get Hidden Packages Found", 1, {
		*p->at(0).globalInteger = m->getWorld()->state.numHiddenPackagesDiscovered;
	})

	OPC_UNIMPLEMENTED_MSG( 0x03E5, "Display Help Text", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03EB, "Clear Small Prints", 0)

	OPC_UNIMPLEMENTED_MSG( 0x03EE, "Draw Text", 3)
	OPC_UNIMPLEMENTED_MSG( 0x03EF, "Make Player Safe For Cutscene", 1)

	OPC_UNIMPLEMENTED_MSG( 0x03F0, "Enable Text Draw", 1 )
	OPC_UNIMPLEMENTED_MSG( 0x03F1, "Set Ped Hostility", 2 )
	OPC_UNIMPLEMENTED_MSG( 0x03F2, "Clear Ped Hostility", 2 )

	OPC_UNIMPLEMENTED_MSG( 0x03F7, "Load Collision", 1 )

	OPC( 0x0408, "Set Total Rampage Missions", 1, {
		m->getWorld()->state.numRampages = p->at(0).integer;
	})

	OPC_UNIMPLEMENTED_MSG( 0x040A, "Remove Chase Car", 1)

	OPC_UNIMPLEMENTED_MSG( 0x0418, "Set Object Draw Ontop", 2)

	OPC( 0x041D, "Set Camera Near Clip", 1, {
		m->getWorld()->renderer.camera.frustum.near = p->at(0).real;
	})

	OPC_UNIMPLEMENTED_MSG( 0x0421, "Force Rain", 1)

	/// @todo http://gtag.gtagaming.com/opcode-database/opcode/0426/
	OPC_UNIMPLEMENTED_MSG( 0x0426, "Create Save Cars Between Levels cube", 6)

	OPC( 0x042C, "Set Total Missions", 1, {
		m->getWorld()->state.numMissions = p->at(0).integer;
	})

	OPC( 0x043C, "Set Sound Fade", 1, {
		m->getWorld()->state.fadeSound = !!p->at(0).integer;
	})
	OPC( 0x043D, "Set Is Intro Playing", 1, {
		m->getWorld()->state.isIntroPlaying = !!p->at(0).integer;
	})

	OPC_COND( 0x0445, "Are Any Vehicle Cheats enabled", 0, {
		return false;
	})

	OPC_UNIMPLEMENTED_MSG( 0x044D, "Load Splash Screen", 1)
};



Opcodes3::Opcodes3()
{
	codes.insert(ops_global.begin(), ops_global.end());
	codes.insert(ops_game.begin(), ops_game.end());
}
