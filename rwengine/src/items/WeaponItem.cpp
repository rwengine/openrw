#include <items/WeaponItem.hpp>
#include <objects/CharacterObject.hpp>
#include <ai/CharacterController.hpp>

void WeaponItem::primary(CharacterObject* character, bool active)
{
	_firing = active;
	if( active ) {
		character->controller->setNextActivity(new Activities::ShootWeapon(this));
	}
}


void WeaponItem::secondary(CharacterObject* character, bool active)
{

}
