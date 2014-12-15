#include <script/Opcodes3.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <script/OpcodesVM.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <render/Model.hpp>
#include <engine/Animator.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>

#define OPC(code, name, params, func) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) func } },
#define OPC_COND(code, name, params, func) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { t->conditionResult = ([=]() {func})(); } } },
#define OPC_UNIMPLEMENTED_CRITICAL(code, name, params) { code, { name, params, [](ScriptMachine* m, SCMThread* t, SCMParams* p) { throw UnimplementedOpcode(code, *p); } } },

glm::vec3 spawnMagic( 0.f, 0.f, 1.f );

VM_OPCODE_DEF( 0x0053 )
{
	auto id	= p->at(0).integer;
	glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

	if( position.z < -99.f ) {
		position = m->getWorld()->getGroundAtPosition(position);
	}

	auto pc = m->getWorld()->createPedestrian(1, position + spawnMagic);
	m->getWorld()->state.player = new PlayerController(pc);

	*p->at(4).handle = m->getWorld()->state.player;
}

VM_OPCODE_DEF( 0x0055 )
{
	auto controller = (CharacterController*)(*p->at(0).handle);
	glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real + 1.f);
	controller->getCharacter()->setPosition(position + spawnMagic);
}
VM_CONDOPCODE_DEF( 0x0056 )
{
	auto controller = (CharacterController*)(*p->at(0).handle);
	glm::vec2 min(p->at(1).real, p->at(2).real);
	glm::vec2 max(p->at(3).real, p->at(4).real);
	bool drawMarker = !!p->at(5).integer;
	auto player = controller->getCharacter()->getPosition();
	if( player.x > min.x && player.y > min.y && player.x < max.x && player.y < max.y ) {
		return true;
	}
	return false;
}

VM_OPCODE_DEF( 0x009A )
{
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
}

VM_OPCODE_DEF( 0x00A5 )
{
	auto id	= p->at(0).integer;
	glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

	auto vehicle = m->getWorld()->createVehicle(id, position + spawnMagic);

	*p->at(4).handle = vehicle;
}

VM_OPCODE_DEF( 0x00BC )
{
	std::string str(p->at(0).string);
	str = m->getWorld()->gameData.texts.text(str);
	m->getWorld()->state.osTextString = str;
	m->getWorld()->state.osTextTime = p->at(1).integer / 1000.f;
	m->getWorld()->state.osTextStart= m->getWorld()->gameTime;
	m->getWorld()->state.osTextStyle = p->at(2).integer;
}

VM_OPCODE_DEF( 0x00BE )
{
	m->getWorld()->state.osTextTime = 0.f;
	m->getWorld()->state.osTextStart= 0.f;
}

VM_OPCODE_DEF( 0x00C0 )
{
	m->getWorld()->state.hour = p->at(0).integer;
	m->getWorld()->state.minute = p->at(1).integer;
}

VM_CONDOPCODE_DEF( 0x00DE )
{
	auto vdata = m->getWorld()->findObjectType<VehicleData>(p->at(1).integer);
	if( vdata )
	{
		auto controller = (CharacterController*)(*p->at(0).handle);
		auto character = controller->getCharacter();
		auto vehicle = character->getCurrentVehicle();
		if ( vehicle ) {

			return vehicle->model && vdata->modelName == vehicle->model->name;
		}
	}
	return false;
}

VM_CONDOPCODE_DEF( 0x00E0 )
{
	auto controller = static_cast<CharacterController*>(*p->at(0).handle);

	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	return vehicle != nullptr;
}

VM_CONDOPCODE_DEF( 0x00E1 )
{
	/// @todo implement
	return false;
}

VM_CONDOPCODE_DEF( 0x0100 )
{
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
}

VM_OPCODE_DEF( 0x0111 )
{
	*m->getWorld()->state.scriptOnMissionFlag = p->at(0).integer;
}

VM_CONDOPCODE_DEF( 0x0118 )
{
	auto controller = static_cast<CharacterController*>(*p->at(0).handle);
	return !controller->getCharacter()->isAlive();
}

VM_CONDOPCODE_DEF( 0x0121 )
{
	auto controller = static_cast<CharacterController*>(*p->at(0).handle);
	std::string zname(p->at(1).string);

	auto zfind = m->getWorld()->gameData.zones.find(zname);
	if( zfind != m->getWorld()->gameData.zones.end() ) {
		auto player = controller->getCharacter()->getPosition();
		auto& min = zfind->second.min;
		auto& max = zfind->second.max;
		if( player.x > min.x && player.y > min.y && player.z > min.z &&
			player.x < max.x && player.y < max.y && player.z < max.z ) {
			std::cout << "Player is in zone! " << zfind->second.name << std::endl;
		}
	}

	return false;
}

VM_OPCODE_DEF( 0x014B )
{
	glm::vec3 position(p->at(0).real, p->at(1).real, p->at(2).real);
	
	if(p->at(4).type == TString)
	{
		std::cerr << "Model names not supported for vehicle generators" << std::endl;
		return;
	}
	
	VehicleGenerator vg;
	vg.position = position;
	vg.heading = p->at(3).real;
	vg.vehicleID = p->at(4).integer;
	vg.colourFG = p->at(5).integer;
	vg.colourBG = p->at(6).integer;
	vg.alwaysSpawn = p->at(7).integer != 0;
	vg.alarmThreshold = p->at(8).integer;
	vg.lockedThreshold = p->at(9).integer;
	vg.minDelay = p->at(10).integer;
	vg.maxDelay = p->at(11).integer;
	
	vg.lastSpawnTime = 0;
	vg.remainingSpawns = 0;
	
	*p->at(12).globalInteger = m->getWorld()->state.vehicleGenerators.size();
	
	m->getWorld()->state.vehicleGenerators.push_back(vg);
}
VM_OPCODE_DEF( 0x014C )
{
	VehicleGenerator& generator = m->getWorld()->state.vehicleGenerators.at(*p->at(0).globalInteger);
	generator.remainingSpawns = p->at(1).integer;
}

VM_OPCODE_DEF( 0x0152 )
{
	auto it = m->getWorld()->gameData.zones.find(p->at(0).string);
	if( it != m->getWorld()->gameData.zones.end() )
	{
		auto day = p->at(1).integer == 1;
		for(int i = 2; i < p->size(); ++i)
		{
			if( day )
			{
				it->second.gangCarDensityDay[i-2] = p->at(i).integer;
			}
			else
			{
				it->second.gangCarDensityNight[i-2] = p->at(i).integer;
			}
		}
	}
}

VM_OPCODE_DEF( 0x015C )
{
	auto it = m->getWorld()->gameData.zones.find(p->at(0).string);
	if( it != m->getWorld()->gameData.zones.end() )
	{
		auto day = p->at(1).integer == 1;
		for(int i = 2; i < p->size(); ++i)
		{
			if( day )
			{
				it->second.gangDensityDay[i-2] = p->at(i).integer;
			}
			else
			{
				it->second.gangDensityNight[i-2] = p->at(i).integer;
			}
		}
	}
}

VM_OPCODE_DEF( 0x0169 )
{
	m->getWorld()->state.fadeColour.r = p->at(0).integer;
	m->getWorld()->state.fadeColour.g = p->at(1).integer;
	m->getWorld()->state.fadeColour.b = p->at(2).integer;
}
VM_OPCODE_DEF( 0x016A )
{
	m->getWorld()->state.fadeTime = p->at(0).integer / 1000.f;
	m->getWorld()->state.fadeOut = !!p->at(1).integer;
	m->getWorld()->state.fadeStart = m->getWorld()->gameTime;
	std::cout << "Fade " << p->at(0).integer << " " << p->at(1).integer << std::endl;
}
VM_CONDOPCODE_DEF( 0x016B )
{
	return m->getWorld()->gameTime <
		m->getWorld()->state.fadeStart + m->getWorld()->state.fadeTime;
}

VM_OPCODE_DEF( 0x0171 )
{
	auto controller = (CharacterController*)(*p->at(0).handle);
	controller->getCharacter()->setHeading(p->at(1).real);
}

VM_OPCODE_DEF( 0x0173 )
{
	auto controller = (CharacterController*)(*p->at(0).handle);
	controller->getCharacter()->setHeading(p->at(1).real);
}

VM_OPCODE_DEF( 0x0175 )
{
	auto vehicle = (VehicleObject*)(*p->at(0).handle);
	vehicle->setHeading(p->at(1).real);
}

VM_OPCODE_DEF( 0x0177 )
{
	auto inst = (InstanceObject*)(*p->at(0).handle);
	inst->setHeading(p->at(1).real);
}

VM_OPCODE_DEF( 0x0180 )
{
	m->getWorld()->state.scriptOnMissionFlag = (unsigned int*)p->at(0).globalInteger;
}

VM_OPCODE_DEF( 0x01B4 )
{
	auto controller = static_cast<PlayerController*>(*p->at(0).handle);
	controller->setInputEnabled(!!p->at(1).integer);
}

VM_OPCODE_DEF( 0x01B6 )
{
	m->getWorld()->state.currentWeather = p->at(0).integer;
}

VM_OPCODE_DEF( 0x01C7 )
{
	auto inst = (InstanceObject*)(*p->at(0).handle);
	std::cout << "Unable to pin object " << inst << ". Object pinning unimplimented" << std::endl;
}

VM_OPCODE_DEF( 0x01E7 )
{
	glm::vec3 min(p->at(0).real,p->at(1).real,p->at(2).real);
	glm::vec3 max(p->at(3).real,p->at(4).real,p->at(5).real);
	
	m->getWorld()->enableAIPaths(AIGraphNode::Vehicle, min, max);
}
VM_OPCODE_DEF( 0x01E8 )
{
	glm::vec3 min(p->at(0).real,p->at(1).real,p->at(2).real);
	glm::vec3 max(p->at(3).real,p->at(4).real,p->at(5).real);
	
	m->getWorld()->disableAIPaths(AIGraphNode::Vehicle, min, max);
}

VM_OPCODE_DEF( 0x01F0 )
{
	m->getWorld()->state.maxWantedLevel = p->at(0).integer;
}

// This does nothing for us.
VM_OPCODE_DEF( 0x01F5 )
{
	auto controller = (CharacterController*)(*p->at(0).handle);
	*p->at(1).handle = controller;
}

VM_CONDOPCODE_DEF( 0x0204 )
{
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
}

VM_CONDOPCODE_DEF( 0x0214 )
{
	/// @todo implement pls
	return false;
}

VM_OPCODE_DEF( 0x0219 )
{
	glm::vec3 min(p->at(0).real, p->at(1).real, p->at(2).real);
	glm::vec3 max(p->at(3).real, p->at(4).real, p->at(5).real);

	/// @todo http://www.gtamodding.com/index.php?title=Garage#GTA_III
	int garageType = p->at(6).integer;
	auto garageHandle = p->at(7).handle;

	std::cout << "Garages Unimplemented. type " << garageType << std::endl;
}

VM_OPCODE_DEF( 0x0229 )
{
	auto vehicle = (VehicleObject*)(*p->at(0).handle);

	auto& colours = m->getWorld()->gameData.vehicleColours;
	vehicle->colourPrimary = colours[p->at(1).integer];
	vehicle->colourSecondary = colours[p->at(2).integer];
}

VM_OPCODE_DEF( 0x022A )
{
	glm::vec3 min(p->at(0).real,p->at(1).real,p->at(2).real);
	glm::vec3 max(p->at(3).real,p->at(4).real,p->at(5).real);
	
	m->getWorld()->enableAIPaths(AIGraphNode::Pedestrian, min, max);
}
VM_OPCODE_DEF( 0x022B )
{
	glm::vec3 min(p->at(0).real,p->at(1).real,p->at(2).real);
	glm::vec3 max(p->at(3).real,p->at(4).real,p->at(5).real);
	
	m->getWorld()->disableAIPaths(AIGraphNode::Pedestrian, min, max);
}

VM_OPCODE_DEF( 0x023C )
{
	m->getWorld()->loadSpecialCharacter(p->at(0).integer, p->at(1).string);
}
VM_CONDOPCODE_DEF( 0x023D )
{
	auto chartype = m->getWorld()->findObjectType<CharacterData>(p->at(0).integer);
	if( chartype ) {
		auto modelfind = m->getWorld()->gameData.models.find(chartype->modelName);
		if( modelfind != m->getWorld()->gameData.models.end() && modelfind->second->model != nullptr ) {
			return true;
		}
	}

	return false;
}

VM_OPCODE_DEF( 0x0244 )
{
	glm::vec3 position(p->at(0).real, p->at(1).real, p->at(2).real);
	if( m->getWorld()->state.currentCutscene ) {
		m->getWorld()->state.currentCutscene->meta.sceneOffset = position;
	}
}

VM_CONDOPCODE_DEF( 0x0248 )
{
	/// @todo this will need changing when model loading is overhauled.
	if( p->at(0).integer == 0 ) {
		/// @todo Figure out if this really does mean the player.
		return true;
	}
	//auto model = m->getFile()->getModels()[p->at(0).integer];
	//if( model == "" ) return true; // ??
	return true;
}

/// @todo http://www.gtamodding.com/index.php?title=0256 (e.g. check if dead or busted)
VM_CONDOPCODE_DEF( 0x0256 )
{
	auto controller = (CharacterController*)(*p->at(0).handle);
	return controller != nullptr;
}

VM_OPCODE_DEF( 0x0293 )
{
	*p->at(0).globalInteger	= 0;
}

VM_OPCODE_DEF( 0x029B )
{
	int id = 0;
	switch(p->at(0).type) {
	case TInt8:
		id = (std::int8_t)p->at(0).integer;
		break;
	case TInt16:
		id = (std::int16_t)p->at(0).integer;
		break;
	}

	if( id < 0 ) {
		auto& modelname = m->getFile()->getModels()[-id];
		id = m->getWorld()->findModelDefinition(modelname);
		if( id == (uint16_t)-1 ) {
			std::cerr << "Failed to find model: " << modelname << std::endl;
		}
	}

	auto& object = m->getWorld()->objectTypes[id];
	glm::vec3 position(p->at(1).real, p->at(2).real, p->at(3).real);

	auto inst = m->getWorld()->createInstance(object->ID, position);

	*p->at(4).handle = inst;
}

VM_CONDOPCODE_DEF( 0x02DE )
{
	auto controller = static_cast<CharacterController*>(*p->at(0).handle);

	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	return vehicle && (vehicle->vehicle->classType & VehicleData::TAXI) == VehicleData::TAXI;
}

VM_OPCODE_DEF( 0x02E4 )
{
	m->getWorld()->loadCutscene(p->at(0).string);
	m->getWorld()->state.cutsceneStartTime = -1.f;
}
VM_OPCODE_DEF( 0x02E5 )
{
	auto id	= p->at(0).integer;

	GameObject* object = object = m->getWorld()->createCutsceneObject(id, m->getWorld()->state.currentCutscene->meta.sceneOffset );
	*p->at(1).handle = object;

	if( object == nullptr ) {
		std::cerr << "Could not create cutscene object " << id << std::endl;
	}
}
VM_OPCODE_DEF( 0x02E6 )
{
	GameObject* object = static_cast<GameObject*>(*p->at(0).handle);
	std::string animName = p->at(1).string;
	std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
	Animation* anim = m->getWorld()->gameData.animations[animName];
	if( anim ) {
		object->animator->setAnimation(anim, false);
	}
	else {
		std::cerr << "Failed to find cutscene animation: " << animName << std::endl;
	}
}
VM_OPCODE_DEF( 0x02E7 )
{
	m->getWorld()->startCutscene();
}
VM_OPCODE_DEF( 0x02E8 )
{
	float time = m->getWorld()->gameTime - m->getWorld()->state.cutsceneStartTime;
	*p->at(0).globalInteger = time * 1000;
}
VM_CONDOPCODE_DEF( 0x02E9 )
{
	if( m->getWorld()->state.currentCutscene ) {
		float time = m->getWorld()->gameTime - m->getWorld()->state.cutsceneStartTime;
		return time > m->getWorld()->state.currentCutscene->tracks.duration;
	}
	return true;
}
VM_OPCODE_DEF( 0x02EA )
{
	m->getWorld()->clearCutscene();
}

VM_OPCODE_DEF( 0x02ED )
{
	m->getWorld()->state.numHiddenPackages = p->at(0).integer;
}

VM_OPCODE_DEF( 0x02F3 )
{
	m->getWorld()->loadSpecialModel(p->at(0).integer, p->at(1).string);
}
VM_OPCODE_DEF( 0x02F4 )
{
	auto id = p->at(1).integer;
	auto actor = static_cast<GameObject*>(*p->at(0).handle);
	CutsceneObject* object = m->getWorld()->createCutsceneObject(id, m->getWorld()->state.currentCutscene->meta.sceneOffset );

	auto headframe = actor->model->model->findFrame("shead");
	actor->skeleton->setEnabled(headframe, false);
	object->setParentActor(actor, headframe);

	*p->at(2).handle = object;
}
VM_OPCODE_DEF( 0x02F5 )
{
	GameObject* object = static_cast<GameObject*>(*p->at(0).handle);
	std::string animName = p->at(1).string;
	std::transform(animName.begin(), animName.end(), animName.begin(), ::tolower);
	Animation* anim = m->getWorld()->gameData.animations[animName];
	if( anim ) {
		object->animator->setAnimation(anim, false);
	}
	else {
		std::cerr << "Failed to find cutscene animation: " << animName << std::endl;
	}
}

VM_OPCODE_DEF( 0x030D )
{
	m->getWorld()->state.maxProgress = p->at(0).integer;
}

VM_OPCODE_DEF( 0x0314 )
{
	m->getWorld()->state.numUniqueJumps = p->at(0).integer;
}

VM_OPCODE_DEF( 0x0324 )
{
	auto it = m->getWorld()->gameData.zones.find(p->at(0).string);
	if( it != m->getWorld()->gameData.zones.end() )
	{
		auto day = p->at(1).integer == 1;
		if( day )
		{
			it->second.pedGroupDay = p->at(2).integer;
		}
		else
		{
			it->second.pedGroupNight = p->at(2).integer;
		}
	}
}

VM_OPCODE_DEF( 0x033E )
{
	glm::vec2 pos(p->at(0).real, p->at(1).real);
	std::string str(p->at(2).string);
	str = m->getWorld()->gameData.texts.text(str);
	m->getWorld()->state.nextText.text = str;
	m->getWorld()->state.nextText.position = pos;
	m->getWorld()->state.texts.push_back(m->getWorld()->state.nextText);
}

VM_OPCODE_DEF( 0x0340 )
{
	m->getWorld()->state.nextText.colourFG.r = p->at(0).integer / 255.f;
	m->getWorld()->state.nextText.colourFG.g = p->at(1).integer / 255.f;
	m->getWorld()->state.nextText.colourFG.b = p->at(2).integer / 255.f;
	m->getWorld()->state.nextText.colourFG.a = p->at(3).integer / 255.f;
}

VM_OPCODE_DEF( 0x0346 )
{
	m->getWorld()->state.nextText.colourBG.r = p->at(0).integer / 255.f;
	m->getWorld()->state.nextText.colourBG.g = p->at(1).integer / 255.f;
	m->getWorld()->state.nextText.colourBG.b = p->at(2).integer / 255.f;
	m->getWorld()->state.nextText.colourBG.a = p->at(3).integer / 255.f;
}

VM_OPCODE_DEF( 0x0352 )
{
	auto controller = static_cast<CharacterController*>(*p->at(0).handle);
	controller->getCharacter()->changeCharacterModel(p->at(1).string);
}

VM_OPCODE_DEF( 0x03B6 )
{
	glm::vec3 position(p->at(0).real, p->at(1).real, p->at(2).real);
	float radius = p->at(3).real;
	int newid = 0, oldid = 0;

	/// @todo fix this being a problem.
	switch(p->at(4).type) {
	case TInt8:
		oldid = (std::int8_t)p->at(4).integer;
		break;
	case TInt16:
		oldid = (std::int16_t)p->at(4).integer;
		break;
	}

	switch(p->at(5).type) {
	case TInt8:
		newid = (std::int8_t)p->at(5).integer;
		break;
	case TInt16:
		newid = (std::int16_t)p->at(5).integer;
		break;
	}

	if( std::abs(newid) > 178 || std::abs(oldid) > 178 ) {
		/// @todo implement this path,
		return;
	}

	std::string newmodel;
	std::string oldmodel;

	if(newid < 0) newid = -newid;
	if(oldid < 0) oldid = -oldid;

	newmodel = m->getFile()->getModels()[newid];
	oldmodel = m->getFile()->getModels()[oldid];
	std::transform(newmodel.begin(), newmodel.end(), newmodel.begin(), ::tolower);
	std::transform(oldmodel.begin(), oldmodel.end(), oldmodel.begin(), ::tolower);

	auto newobjectid = m->getWorld()->findModelDefinition(newmodel);
	auto nobj = m->getWorld()->findObjectType<ObjectData>(newobjectid);

	/// @todo Objects need to adopt the new object ID, not just the model.
	for(auto o : m->getWorld()->objects) {
		if( o->type() == GameObject::Instance ) {
			if( !o->model ) continue;
			if( o->model->name != oldmodel ) continue;
			float d = glm::distance(position, o->getPosition());
			if( d < radius ) {
				m->getWorld()->gameData.loadDFF(newmodel + ".dff", false);
				InstanceObject* inst = static_cast<InstanceObject*>(o);
				inst->changeModel(nobj);
				inst->model = m->getWorld()->gameData.models[newmodel];
			}
		}
	}
}

VM_CONDOPCODE_DEF( 0x03C6 )
{
	// The paramter to this is actually the island number.
	// Not sure how that will fit into the scheme of full paging
	/// @todo use the current player zone island number to return the correct value.
	return true;
}

VM_OPCODE_DEF( 0x03E1 )
{
	*p->at(0).globalInteger = m->getWorld()->state.numHiddenPackagesDiscovered;
}

VM_OPCODE_DEF( 0x03F7 )
{
	// Collision is loaded when required, not sure what this is supposed to mean.
}

VM_OPCODE_DEF( 0x0408 )
{
	m->getWorld()->state.numRampages = p->at(0).integer;
}

VM_OPCODE_DEF( 0x041D )
{
	m->getWorld()->state.cameraNear = p->at(0).real;
}

VM_OPCODE_DEF( 0x042C )
{
	m->getWorld()->state.numMissions = p->at(0).integer;
}

VM_OPCODE_DEF( 0x043C )
{
	m->getWorld()->state.fadeSound = !!p->at(0).integer;
}
VM_OPCODE_DEF( 0x043D )
{
	m->getWorld()->state.isIntroPlaying = !!p->at(0).integer;
}

VM_CONDOPCODE_DEF( 0x0445 )
{
	return false;
}

VM_OPCODE_DEF( 0x044D )
{
	m->getWorld()->gameData.loadSplash(p->at(0).string);
}

#define MERGE(other) insert(other.begin(), other.end())

Opcodes3::Opcodes3()
{
	codes.MERGE(Opcodes::VM::get().codes);

	VM_OPCODE_DEC( 0x0053, 5, "Create Player" );

	VM_OPCODE_DEC( 0x0055, 4, "Set Player Position" );
	VM_CONDOPCODE_DEC( 0x0056, 6, "Is In Area" );

	VM_OPCODE_DEC( 0x009A, 6, "Create Character" );

	VM_OPCODE_DEC( 0x00A5, 5, "Create Vehicle" );

	VM_OPCODE_DEC( 0x00BC, 3, "Print Message Now" );

	VM_OPCODE_DEC( 0x00BE, 0, "Clear Message Prints" );

	VM_OPCODE_DEC( 0x00C0, 2, "Set Time Of Day" );

	VM_CONDOPCODE_DEC( 0x00DE, 2, "Is Player In Model" );

	VM_CONDOPCODE_DEC( 0x00E0, 1, "Is Player In Any Vehicle" );
	VM_CONDOPCODE_DEC( 0x00E1, 2, "Is Button Pressed" );

	VM_CONDOPCODE_DEC( 0x0100, 8, "Is Character near point in car" );

	VM_OPCODE_DEC( 0x0111, 1, "Set Dead or Arrested" );

	VM_CONDOPCODE_DEC( 0x0118, 1, "Is Character Dead" );

	VM_CONDOPCODE_DEC( 0x0121, 2, "Is Player In Zone" );

	VM_OPCODE_DEC( 0x014B, 13, "Create Car Generator" );
	VM_OPCODE_DEC( 0x014C, 2, "Set Car Generator count" );

	VM_OPCODE_DEC( 0x0152, 17, "Set zone car info" );

	VM_OPCODE_DEC( 0x015C, 11, "Set zone ped info" );

	VM_OPCODE_DEC_U( 0x0164, 1, "Disable Radar Blip" );

	VM_OPCODE_DEC( 0x0169, 3, "Set Fade Colour" );
	VM_OPCODE_DEC( 0x016A, 2, "Fade Screen" );
	VM_CONDOPCODE_DEC( 0x016B, 0, "Is Screen Fading" );
	VM_OPCODE_DEC_U( 0x016C, 4, "Add Hospital Restart" );
	VM_OPCODE_DEC_U( 0x016D, 4, "Add Police Restart" );

	VM_OPCODE_DEC( 0x0171, 2, "Set Player Heading" );

	VM_OPCODE_DEC( 0x0173, 2, "Set Character Heading" );

	VM_OPCODE_DEC( 0x0175, 2, "Set Vehicle heading" );

	VM_OPCODE_DEC( 0x0177, 2, "Set Object heading" );

	VM_OPCODE_DEC( 0x0180, 1, "Link ONMISSION Flag" );
	VM_OPCODE_DEC_U( 0x0181, 2, "Link Character Mission Flag" );
	VM_OPCODE_DEC_U( 0x0182, 2, "Unknown Character Opcode" );

	VM_OPCODE_DEC_U( 0x018B, 2, "Change Blip Display Mode" );

	VM_OPCODE_DEC_U( 0x018D, 5, "Create soundscape" );

	VM_OPCODE_DEC( 0x01B4, 2, "Set Player Input Enabled" );

	VM_OPCODE_DEC( 0x01B6, 1, "Set Weather Now" );

	VM_OPCODE_DEC_U( 0x01BE, 4, "Turn Character To Face Point" );

	VM_OPCODE_DEC( 0x01C7, 1, "Don't remove object" );

	VM_OPCODE_DEC( 0x01E7, 6, "Enable Roads" );
	VM_OPCODE_DEC( 0x01E8, 6, "Disable Roads" );

	VM_OPCODE_DEC_U( 0x01ED, 1, "Clear Character Threat Search" );

	VM_OPCODE_DEC( 0x01F0, 1, "Set Max Wanted Level" );

	VM_OPCODE_DEC( 0x01F5, 2, "Get Player Character" );

	VM_CONDOPCODE_DEC( 0x0204, 5, "Is Char near Car in Car 2D" );

	/// @todo http://gtag.gtagaming.com/opcode-database/opcode/0213/
	VM_OPCODE_DEC_U( 0x0213, 6, "Create pickup" );
	VM_CONDOPCODE_DEC( 0x0214, 1, "Has Pickup been collected" );
	VM_OPCODE_DEC_U( 0x0215, 1, "Destroy Pickup" );

	VM_OPCODE_DEC( 0x0219, 8, "Create Garage" );

	VM_OPCODE_DEC( 0x0229, 3, "Set Vehicle Colours" );

	VM_OPCODE_DEC( 0x022A, 6, "Disable ped paths" );
	VM_OPCODE_DEC( 0x022B, 6, "Disable ped paths" );

	VM_OPCODE_DEC_U( 0x022D, 2, "Set Character Always Face Player" );

	VM_OPCODE_DEC_U( 0x0236, 2, "Set Gang Car" );
	VM_OPCODE_DEC_U( 0x0237, 3, "Set Gang Weapons" );

	VM_OPCODE_DEC( 0x023C, 2, "Load Special Character" );
	VM_CONDOPCODE_DEC( 0x023D, 1, "Is Special Character Loaded" );

	VM_OPCODE_DEC( 0x0244, 3, "Set Cutscene Offset" );
	VM_OPCODE_DEC_U( 0x0245, 2, "Set Character Animation Group" );

	VM_OPCODE_DEC_U( 0x0247, 1, "Request Model Loaded" );

	VM_OPCODE_DEC_U( 0x024A, 3, "Get Phone Near" );

	VM_CONDOPCODE_DEC( 0x0248, 1, "Is Model Loaded" );
	VM_OPCODE_DEC_U( 0x0249, 1, "Mark Model As Unneeded" );

	VM_OPCODE_DEC_U( 0x0250, 6, "Create Light" );

	VM_CONDOPCODE_DEC( 0x0256, 1, "Is Player Playing" );

	VM_OPCODE_DEC( 0x0293, 1, "Get Controller Mode" );

	VM_OPCODE_DEC_U( 0x0296, 1, "Unload Special Character" );

	VM_OPCODE_DEC( 0x029B, 5, "Create Object no offset" );

	VM_OPCODE_DEC_U( 0x02A7, 5, "Add Radar Contact Blip" );
	VM_OPCODE_DEC_U( 0x02A8, 5, "Add Radar Blip" );

	VM_CONDOPCODE_DEC( 0x02DE, 1, "Is Player In Taxi" );

	VM_OPCODE_DEC( 0x02E4, 1, "Load Cutscene Data" );
	VM_OPCODE_DEC( 0x02E5, 2, "Create Cutscene Object" );
	VM_OPCODE_DEC( 0x02E6, 2, "Set Cutscene Animation" );
	VM_OPCODE_DEC( 0x02E7, 0, "Start Cutscene" );
	VM_OPCODE_DEC( 0x02E8, 1, "Get Cutscene Time" );
	VM_CONDOPCODE_DEC( 0x02E9, 0, "Is Cutscene Over" );
	VM_OPCODE_DEC( 0x02EA, 0, "Clear Cutscene" );

	VM_OPCODE_DEC_U( 0x02EC, 3, "Create Hidden Package" );

	VM_OPCODE_DEC( 0x02ED, 1, "Set Total Hidden Packages" );

	VM_OPCODE_DEC( 0x02F3, 2, "Load Special Model" );
	VM_OPCODE_DEC( 0x02F4, 3, "Create Cutscene Actor Head" );
	VM_OPCODE_DEC( 0x02F5, 2, "Set Cutscene Head Animation" );

	VM_OPCODE_DEC_U( 0x02FB, 10, "Create Crusher Crane" );

	VM_OPCODE_DEC( 0x030D, 1, "Set Max Progress" );

	VM_OPCODE_DEC( 0x0314, 1, "Set Total Unique Jumps" );

	VM_OPCODE_DEC( 0x0324, 3, "Set zone ped group" );
	VM_OPCODE_DEC_U( 0x0325, 2, "Create Car Fire" );

	VM_OPCODE_DEC_U( 0x032B, 7, "Create Weapon Pickup" );

	VM_OPCODE_DEC_U( 0x0336, 2, "Set Player Visible" );

	VM_OPCODE_DEC( 0x033E, 3, "Display Text" );
	VM_OPCODE_DEC_U( 0x033F, 2, "Set Text Scale" );
	VM_OPCODE_DEC( 0x0340, 4, "Set Text Colour" );
	VM_OPCODE_DEC_U( 0x0341, 1, "Set Text Justify" );
	VM_OPCODE_DEC_U( 0x0342, 1, "Set Text Centered" );
	VM_OPCODE_DEC_U( 0x0344, 1, "Set Center Text Size" );
	VM_OPCODE_DEC_U( 0x0345, 1, "Set Text Background" );
	VM_OPCODE_DEC( 0x0346, 4, "Set Text Background Colour" );

	VM_OPCODE_DEC_U( 0x0348, 1, "Set Text Size Proportional" );
	VM_OPCODE_DEC_U( 0x0349, 1, "Set Text Font" );

	VM_OPCODE_DEC( 0x0352, 2, "Set Character Model" );
	VM_OPCODE_DEC_U( 0x0353, 1, "Refresh Actor Model" );
	VM_OPCODE_DEC_U( 0x0354, 1, "Start Chase Scene" );
	VM_OPCODE_DEC_U( 0x0355, 0, "Stop Chase Scene" );

	VM_OPCODE_DEC_U( 0x035D, 1, "Set Object Targetable" );

	VM_OPCODE_DEC_U( 0x0363, 6, "Set Closest Object Visibility" );

	VM_OPCODE_DEC_U( 0x0368, 10, "Create ev Crane" );

	VM_OPCODE_DEC_U( 0x0373, 0, "Set Camera Behind Player" );
	VM_OPCODE_DEC_U( 0x0374, 1, "Set Motion Blur" );

	VM_OPCODE_DEC_U( 0x038B, 0, "Load Requested Models Now" );

	VM_OPCODE_DEC_U( 0x0399, 7, "Disable ped paths in angled cube" );

	VM_OPCODE_DEC_U( 0x039D, 12, "Scatter Particles" );

	VM_OPCODE_DEC_U( 0x03AD, 1, "Set Garbage Enabled" );

	VM_OPCODE_DEC_U( 0x03AF, 1, "Set Map Streaming Enabled" );

	VM_OPCODE_DEC( 0x03B6, 6, "Change Nearest Instance Model" );
	VM_OPCODE_DEC_U( 0x03B7, 1, "Process Cutscene Only" );

	VM_OPCODE_DEC_U( 0x03BB, 1, "Set Garage Door to Rotate" );

	VM_OPCODE_DEC_U( 0x03BF, 2, "Set Pedestrians Ignoring Player" );

	VM_CONDOPCODE_DEC( 0x03C6, 1, "Is Collision In Memory" );

	VM_OPCODE_DEC_U( 0x03CB, 3, "Load Area Near" );

	VM_OPCODE_DEC_U( 0x03DA, 1, "Set Garage Camera Follows Player" );

	VM_OPCODE_DEC( 0x03E1, 1, "Get Hidden Packages Found" );

	VM_OPCODE_DEC_U( 0x03E5, 1, "Display Help Text" );

	VM_OPCODE_DEC_U( 0x03EB, 0, "Clear Small Prints" );

	VM_OPCODE_DEC_U( 0x03EE, 3, "Draw Text" );
	VM_OPCODE_DEC_U( 0x03EF, 1, "Make Player Safe For Cutscene" );

	VM_OPCODE_DEC_U( 0x03F0, 1, "Enable Text Draw" );
	VM_OPCODE_DEC_U( 0x03F1, 2, "Set Ped Hostility" );
	VM_OPCODE_DEC_U( 0x03F2, 2, "Clear Ped Hostility" );

	VM_OPCODE_DEC( 0x03F7, 1, "Load Collision" );

	VM_OPCODE_DEC( 0x0408, 1, "Set Total Rampage Missions" );

	VM_OPCODE_DEC_U( 0x040A, 1, "Remove Chase Car" );

	VM_OPCODE_DEC_U( 0x0418, 2, "Set Object Draw Ontop" );

	VM_OPCODE_DEC( 0x041D, 1, "Set Camera Near Clip" );

	VM_OPCODE_DEC_U( 0x0421, 1, "Force Rain" );

	VM_OPCODE_DEC_U( 0x0426, 6, "Create Save Cars Between Levels cube" );

	VM_OPCODE_DEC( 0x042C, 1, "Set Total Missions" );

	VM_OPCODE_DEC( 0x043C, 1, "Set Sound Fade" );
	VM_OPCODE_DEC( 0x043D, 1, "Set Is Intro Playing" );

	VM_CONDOPCODE_DEC( 0x0445, 0, "Are Any Vehicle Cheats enabled" );

	VM_OPCODE_DEC( 0x044D, 1, "Load Splash Screen" );
}
