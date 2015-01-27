#pragma once

#include <objects/PickupObject.hpp>

class GenericPickup : public PickupObject
{
public:
	
    GenericPickup(GameWorld* world, const glm::vec3& position, int modelID, int type);
	
    virtual bool onCharacterTouch(CharacterObject* character);
	
private:
	int type;
};