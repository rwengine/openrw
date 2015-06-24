#include <script/ScriptTypes.hpp>
#include <script/ScriptMachine.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/CharacterObject.hpp>
#include <ai/PlayerController.hpp>

GameState* ScriptArguments::getState() const
{
	return getVM()->getState();
}

GameWorld* ScriptArguments::getWorld() const
{
	return getVM()->getState()->world;
}

GameObject* ScriptArguments::getPlayer(unsigned int player) const
{
	auto playerId = parameters->at(player).integerValue();
	PlayerController* controller = getWorld()->players.at( playerId );
	return controller->getCharacter();
}

template<> GameObject* ScriptArguments::getObject< CharacterObject >(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	return getWorld()->pedestrianPool.find( gameObjectID );
}

template<> GameObject* ScriptArguments::getObject< CutsceneObject >(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	return getWorld()->cutscenePool.find( gameObjectID );
}

template<> GameObject* ScriptArguments::getObject< InstanceObject >(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	return getWorld()->instancePool.find( gameObjectID );
}

template<> GameObject* ScriptArguments::getObject< PickupObject >(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	return getWorld()->pickupPool.find( gameObjectID );
}

template<> GameObject* ScriptArguments::getObject< VehicleObject >(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	return getWorld()->vehiclePool.find( gameObjectID );
}

/*GameObject* ScriptArguments::getGameObject(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	auto it = getWorld()->objects.find( gameObjectID );
	return (it == getWorld()->objects.end())? nullptr : it->second;
}*/
