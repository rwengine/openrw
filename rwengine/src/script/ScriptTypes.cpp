#include <script/ScriptTypes.hpp>
#include <script/ScriptMachine.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/CharacterObject.hpp>
#include <ai/PlayerController.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameData.hpp>

GameState* ScriptArguments::getState() const { return getVM()->getState(); }

GameWorld* ScriptArguments::getWorld() const { return getVM()->getState()->world; }

int ScriptArguments::getModel(unsigned int arg) const
{
  RW_CHECK(arg < getParameters().size(), "arg out of range");
  if (arg >= getParameters().size()) {
    return 0;
  }

  int id = getParameters()[arg].integerValue();

  /// @todo verify this behaviour
  if (id < 0) {
    id = -id;
    const auto& model = getVM()->getFile()->getModels()[id];
    id = getWorld()->data->findModelObject(model);
  }

  return id;
}

GameObject* ScriptArguments::getPlayerCharacter(unsigned int player) const
{
  auto playerId = parameters->at(player).integerValue();
  PlayerController* controller = getWorld()->players.at(playerId);
  RW_CHECK(controller != nullptr, "No controller for player " << player);
  return controller->getCharacter();
}

template <>
GameObject* ScriptArguments::getObject<PlayerController>(unsigned int arg) const
{
  return getPlayerCharacter(arg);
}

template <>
GameObject* ScriptArguments::getObject<CharacterObject>(unsigned int arg) const
{
  auto gameObjectID = parameters->at(arg).integerValue();
  auto object = getWorld()->pedestrianPool.find(gameObjectID);
  RW_CHECK(object != nullptr, "No pedestrian for ID " << gameObjectID);
  return object;
}

template <>
GameObject* ScriptArguments::getObject<CutsceneObject>(unsigned int arg) const
{
  auto gameObjectID = parameters->at(arg).integerValue();
  auto object = getWorld()->cutscenePool.find(gameObjectID);
  RW_CHECK(object != nullptr, "No cutscene object for ID " << gameObjectID);
  return object;
}

template <>
GameObject* ScriptArguments::getObject<InstanceObject>(unsigned int arg) const
{
  auto gameObjectID = parameters->at(arg).integerValue();
  auto object = getWorld()->instancePool.find(gameObjectID);
  RW_CHECK(object != nullptr, "No instance for ID " << gameObjectID);
  return object;
}

template <>
GameObject* ScriptArguments::getObject<PickupObject>(unsigned int arg) const
{
  auto gameObjectID = parameters->at(arg).integerValue();
  auto object = getWorld()->pickupPool.find(gameObjectID);
  RW_CHECK(object != nullptr, "No pickup for ID " << gameObjectID);
  return object;
}

template <>
GameObject* ScriptArguments::getObject<VehicleObject>(unsigned int arg) const
{
  auto gameObjectID = parameters->at(arg).integerValue();
  auto object = getWorld()->vehiclePool.find(gameObjectID);
  RW_CHECK(object != nullptr, "No pedestrian for ID " << gameObjectID);
  return object;
}

/*GameObject* ScriptArguments::getGameObject(unsigned int arg) const
{
    auto gameObjectID = parameters->at(arg).integerValue();
    auto it = getWorld()->objects.find( gameObjectID );
    return (it == getWorld()->objects.end())? nullptr : it->second;
}*/
