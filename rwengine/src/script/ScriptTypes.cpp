#include <script/ScriptTypes.hpp>
#include <script/ScriptMachine.hpp>
#include <engine/GameState.hpp>

GameState* ScriptArguments::getState() const
{
	return getVM()->getState();
}

GameWorld* ScriptArguments::getWorld() const
{
	return getVM()->getState()->world;
}
