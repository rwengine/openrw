#include <script/ScriptTypes.hpp>
#include <script/ScriptMachine.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>

GameState* ScriptArguments::getState() const
{
	return getVM()->getState();
}

GameWorld* ScriptArguments::getWorld() const
{
	return getVM()->getState()->world;
}

GameObject* ScriptArguments::getGameObject(unsigned int arg) const
{
	auto gameObjectID = parameters->at(arg).integerValue();
	auto it = getWorld()->objects.find( gameObjectID );
	return (it == getWorld()->objects.end())? nullptr : it->second;
}
