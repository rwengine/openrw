#include <objects/CharacterObject.hpp>
#include <ai/CharacterController.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <objects/VehicleObject.hpp>
#include <items/InventoryItem.hpp>
#include <data/Skeleton.hpp>
#include <rw/defines.hpp>

// TODO: make this not hardcoded
static glm::vec3 enter_offset(0.81756252f, 0.34800607f, -0.486281008f);

const float CharacterObject::DefaultJumpSpeed = 2.f;

CharacterObject::CharacterObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model, std::shared_ptr<CharacterData> data)
	: GameObject(engine, pos, rot, model)
	, currentState({})
	, currentVehicle(nullptr)
	, currentSeat(0)
	, running(false)
	, jumped(false)
	, jumpSpeed(DefaultJumpSpeed)
	, motionBlockedByActivity(false)
	, ped(data)
	, physCharacter(nullptr)
	, physObject(nullptr)
	, physShape(nullptr)
	, controller(nullptr)
{
	// TODO move AnimationGroup creation somewhere else.
	animations.idle = engine->data->animations["idle_stance"];
	animations.walk = engine->data->animations["walk_player"];
	animations.walk_start = engine->data->animations["walk_start"];
	animations.run  = engine->data->animations["run_player"];

	animations.walk_right = engine->data->animations["walk_player_right"];
	animations.walk_right_start = engine->data->animations["walk_start_right"];
	animations.walk_left = engine->data->animations["walk_player_left"];
	animations.walk_left_start = engine->data->animations["walk_start_left"];

	animations.walk_back = engine->data->animations["walk_player_back"];
	animations.walk_back_start = engine->data->animations["walk_start_back"];

	animations.jump_start = engine->data->animations["jump_launch"];
	animations.jump_glide = engine->data->animations["jump_glide"];
	animations.jump_land  = engine->data->animations["jump_land"];

	animations.car_sit     = engine->data->animations["car_sit"];
	animations.car_sit_low = engine->data->animations["car_lsit"];

	animations.car_open_lhs   = engine->data->animations["car_open_lhs"];
	animations.car_getin_lhs   = engine->data->animations["car_getin_lhs"];
	animations.car_getout_lhs   = engine->data->animations["car_getout_lhs"];
	animations.car_pullout_lhs   = engine->data->animations["car_pullout_lhs"];
	animations.car_jacked_lhs   = engine->data->animations["car_jackedlhs"];

	animations.car_open_rhs   = engine->data->animations["car_open_rhs"];
	animations.car_getin_rhs   = engine->data->animations["car_getin_rhs"];
	animations.car_getout_rhs   = engine->data->animations["car_getout_rhs"];
	animations.car_pullout_rhs   = engine->data->animations["car_pullout_rhs"];
	animations.car_jacked_rhs   = engine->data->animations["car_jackedrhs"];

	animations.kd_front   = engine->data->animations["kd_front"];
	animations.ko_shot_front   = engine->data->animations["ko_shot_front"];

	if(model) {
		skeleton = new Skeleton;
		animator = new Animator(model->resource, skeleton);

		createActor();
	}
}

CharacterObject::~CharacterObject()
{
	destroyActor();
	if( currentVehicle )
	{
		currentVehicle->setOccupant(getCurrentSeat(), nullptr);
	}
}

void CharacterObject::createActor(const glm::vec2& size)
{
	if(physCharacter) {
		destroyActor();
	}
	
	// Don't create anything without a valid model.
	if(model) {
		btTransform tf;
		tf.setIdentity();
		tf.setOrigin(btVector3(position.x, position.y, position.z));

		physObject = new btPairCachingGhostObject;
		physObject->setUserPointer(this);
		physObject->setWorldTransform(tf);
		physShape = new btCapsuleShapeZ(size.x, size.y);
		physObject->setCollisionShape(physShape);
		physObject->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
		physCharacter = new btKinematicCharacterController(physObject, physShape, 0.30f, 2);
		physCharacter->setFallSpeed(120.f);
		physCharacter->setUseGhostSweepTest(true);
		physCharacter->setVelocityForTimeInterval(btVector3(1.f, 1.f, 1.f), 1.f);
		physCharacter->setGravity(engine->dynamicsWorld->getGravity().length());
		physCharacter->setJumpSpeed(4.f);

		engine->dynamicsWorld->addCollisionObject(physObject, btBroadphaseProxy::KinematicFilter,
												  btBroadphaseProxy::StaticFilter|btBroadphaseProxy::SensorTrigger);
		engine->dynamicsWorld->addAction(physCharacter);
	}
}

void CharacterObject::destroyActor()
{
	if(physCharacter) {
		engine->dynamicsWorld->removeCollisionObject(physObject);
		engine->dynamicsWorld->removeAction(physCharacter);

		delete physCharacter;
		delete physObject;
		delete physShape;
		physCharacter = nullptr;
	}
}

glm::vec3 CharacterObject::updateMovementAnimation(float dt)
{
	glm::vec3 animTranslate;

	if (motionBlockedByActivity)
	{
		// Clear any residual motion animation
		animator->playAnimation(AnimIndexMovement, nullptr, 1.f, false);
		return glm::vec3();
	}

	// Things are simpler if we're in a vehicle
	if (getCurrentVehicle())
	{
		animator->playAnimation(0, animations.car_sit, 1.f, true);
		return glm::vec3();
	}

	Animation* movementAnimation = animations.idle;
	Animation* currentAnim = animator->getAnimation(AnimIndexMovement);
	bool isActionHappening = (animator->getAnimation(AnimIndexAction) != nullptr);
	float animationSpeed = 1.f;
	bool repeat = true;
	constexpr float movementEpsilon = 0.1f;

	float movementLength = glm::length(movement);
	if (!isAlive()) {
		movementAnimation = animations.ko_shot_front;
		repeat = false;
	}
	else if (jumped) {
		repeat = false;
		if (currentAnim == animations.jump_start &&
				animator->isCompleted(AnimIndexMovement)) {
			movementAnimation = animations.jump_start;
		}
		if (isOnGround()) {
			if (currentAnim != animations.jump_land
					|| !animator->isCompleted(AnimIndexMovement)) {
				movementAnimation = animations.jump_land;
			}
			else {
				// We are done jumping
				jumped = false;
			}
		}
		else {
			movementAnimation = animations.jump_glide;
		}
	}
	else if (movementLength > movementEpsilon)
	{
		if (running && !isActionHappening) {
			// No slow running
			movementAnimation = animations.run;
			animationSpeed = 1.f;
		}
		else {
			animationSpeed = 1.f / movementLength;
			// Determine if we need to play the walk start animation
			if (currentAnim != animations.walk)
			{
				if (currentAnim != animations.walk_start || !animator->isCompleted(AnimIndexMovement))
				{
					movementAnimation = animations.walk_start;
				}
				else
				{
					movementAnimation = animations.walk;
				}
			}
			else
			{
				// Keep walkin
				movementAnimation = animations.walk;
			}
		}
	}

	// Check if we need to change the animation or change speed
	if (animator->getAnimation(AnimIndexMovement) != movementAnimation)
	{
		animator->playAnimation(AnimIndexMovement, movementAnimation, animationSpeed, repeat);
	}
	else
	{
		animator->setAnimationSpeed(AnimIndexMovement, animationSpeed);
	}

	// If we have to, interrogate the movement animation
	if (movementAnimation != animations.idle)
	{
		if(! model->resource->frames[0]->getChildren().empty() )
		{
			ModelFrame* root = model->resource->frames[0]->getChildren()[0];
			auto it = movementAnimation->bones.find(root->getName());
			if (it != movementAnimation->bones.end())
			{
				AnimationBone* rootBone = it->second;
				float step = dt;
				const float duration = animator->getAnimation(AnimIndexMovement)->duration;
				float animTime = fmod(animator->getAnimationTime(AnimIndexMovement), duration);

				// Handle any remaining transformation before the end of the keyframes
				if ((animTime+step) > duration)
				{
					glm::vec3 a = rootBone->getInterpolatedKeyframe(animTime).position;
					glm::vec3 b = rootBone->getInterpolatedKeyframe(duration).position;
					glm::vec3 d = (b-a);
					animTranslate.y += d.y;
					step -= (duration - animTime);
					animTime = 0.f;
				}

				glm::vec3 a = rootBone->getInterpolatedKeyframe(animTime).position;
				glm::vec3 b = rootBone->getInterpolatedKeyframe(animTime+step).position;
				glm::vec3 d = (b-a);
				animTranslate.y += d.y;

				Skeleton::FrameData fd = skeleton->getData(root->getIndex());
				fd.a.translation.y = 0.f;
				skeleton->setData(root->getIndex(), fd);
			}
		}
	}

	return animTranslate;
}

void CharacterObject::tick(float dt)
{
	if(controller) {
		controller->update(dt);
	}

	animator->tick(dt);
	updateCharacter(dt);

	// Ensure the character doesn't need to be reset
	if(getPosition().z < -100.f) {
		resetToAINode();
	}
}

#include <algorithm>
void CharacterObject::changeCharacterModel(const std::string &name)
{
	auto modelName = std::string(name);
	std::transform(modelName.begin(), modelName.end(), modelName.begin(), ::tolower);

	engine->data->loadDFF(modelName + ".dff");
	engine->data->loadTXD(modelName + ".txd");

	auto& models = engine->data->models;
	auto mfind = models.find(modelName);
	if( mfind != models.end() ) {
		model = mfind->second;
	}

	if( skeleton )
	{
		delete animator;
		delete skeleton;
	}

	skeleton = new Skeleton;
	animator = new Animator(model->resource, skeleton);
}

void CharacterObject::updateCharacter(float dt)
{
	/*
	 * You can fire weapons while moving
	 *
	 * Two Modes: Moving and Action
	 * Moving covers walking & jumping
	 * Action covers complex things like shooting, entering vehicles etc.
	 *
	 * Movement animation should be handled here
	 *  If Current weapon is one handed, then it can be used while walking.
	 *  This means blending the weapon animation with the walk animation.
	 *  No weapons can be used while sprinting.
	 *  Need an "aim vector" to apply torso correction.
	 *
	 *  If movement vector is less than some threshold, fully walk animation
	 *  (time adjusted for velocity).
	 */

	if(physCharacter) {
		glm::vec3 walkDir = updateMovementAnimation(dt);

		position = getPosition();

		if (canTurn()) {
			rotation = glm::angleAxis(m_look.x, glm::vec3{0.f, 0.f, 1.f});
		}

		walkDir = rotation * walkDir;

		if( jumped )
		{
			if( !isOnGround() )
			{
				walkDir = rotation * glm::vec3(0.f, jumpSpeed * dt, 0.f);
			}
		}

		if (isAlive())
		{
			physCharacter->setWalkDirection(btVector3(walkDir.x, walkDir.y, walkDir.z));
		}
		else
		{
			physCharacter->setWalkDirection(btVector3(0.f, 0.f, 0.f));
		}

		auto Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
		position = glm::vec3(Pos.x(), Pos.y(), Pos.z());

		// Handle above waist height water.
		auto wi = engine->data->getWaterIndexAt(getPosition());
		if( wi != NO_WATER_INDEX ) {
			float wh = engine->data->waterHeights[wi];
			auto ws = getPosition();
			wh += engine->data->getWaveHeightAt(ws);
			
			// If Not in water before
			//  If last position was above water
			//   Now Underwater
			//  Else Not Underwater
			// Else
			//  Underwater
			
			if( ! inWater && ws.z < wh && _lastHeight > wh ) {
				ws.z = wh;

				btVector3 bpos(ws.x, ws.y, ws.z);
				physCharacter->warp(bpos);
				auto& wt = physObject->getWorldTransform();
				wt.setOrigin(bpos);
				physObject->setWorldTransform(wt);

				physCharacter->setGravity(0.f);
				inWater = true;
			}
			else {
				physCharacter->setGravity(9.81f);
				inWater = false;
			}
		}
		_lastHeight = getPosition().z;
	}
	else
	{
		updateMovementAnimation(dt);
	}
}

void CharacterObject::setPosition(const glm::vec3& pos)
{
	if( physCharacter )
	{
		btVector3 bpos(pos.x, pos.y, pos.z);
		if( std::abs(-100.f - pos.z) < 0.01f )
		{
			// Find the ground position
			auto gpos = engine->getGroundAtPosition(pos);
			bpos.setZ(gpos.z+1.f);
		}
		physCharacter->warp(bpos);
	}
	position = pos;
}

glm::vec3 CharacterObject::getPosition() const
{
	if(physCharacter) {
		btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z());
	}
	if(currentVehicle) {
		/// @todo this is hacky.
		if( animator->getAnimation(AnimIndexAction) == animations.car_getout_lhs ) {
			return currentVehicle->getSeatEntryPosition(currentSeat);
		}

		auto v = getCurrentVehicle();
		auto R = glm::mat3_cast(v->getRotation());
		glm::vec3 offset;
		auto o = (animator->getAnimation(AnimIndexAction) == animations.car_getin_lhs) ? enter_offset : glm::vec3();
		if(getCurrentSeat() < v->info->seats.size()) {
			offset = R * (v->info->seats[getCurrentSeat()].offset -
					o);
		}
		return currentVehicle->getPosition() + offset;
	}
	return position;
}

glm::quat CharacterObject::getRotation() const
{
	if(currentVehicle) {
		return currentVehicle->getRotation();
	}
	return GameObject::getRotation();
}

bool CharacterObject::isAlive() const
{
	return currentState.health > 0.f;
}

bool CharacterObject::enterVehicle(VehicleObject* vehicle, size_t seat)
{
	if(vehicle) {
		// Check that the seat is free
		if(vehicle->getOccupant(seat)) {
			return false;
		}
		else {
			// Make sure we leave any vehicle we're inside
			enterVehicle(nullptr, 0);
			vehicle->setOccupant(seat, this);
			setCurrentVehicle(vehicle, seat);
			//enterAction(VehicleSit);
			return true;
		}
	}
	else {
		if(currentVehicle) {
			currentVehicle->setOccupant(seat, nullptr);
			// Disabled due to crashing.
			//setPosition(currentVehicle->getPosition()); 
			setCurrentVehicle(nullptr, 0);
			return true;
		}
	}
	return false;
}

bool CharacterObject::isStopped() const
{
	if (currentVehicle != nullptr) {
		return currentVehicle->isStopped();
	}

	return controller->getCurrentActivity() == nullptr;
}

VehicleObject *CharacterObject::getCurrentVehicle() const
{
	return currentVehicle;
}

size_t CharacterObject::getCurrentSeat() const
{
	return currentSeat;
}

void CharacterObject::setCurrentVehicle(VehicleObject *value, size_t seat)
{
	currentVehicle = value;
	currentSeat = seat;
	if(currentVehicle == nullptr && physCharacter == nullptr) {
		createActor();
	}
	else if(currentVehicle) {
		destroyActor();
	}
}

bool CharacterObject::takeDamage(const GameObject::DamageInfo& dmg)
{
	// Right now there's no state that determines immunity to any kind of damage
	float dmgPoints = dmg.hitpoints;
	if (currentState.armour > 0.f) {
		dmgPoints -= currentState.armour;
		currentState.armour = std::max(0.f, currentState.armour - dmg.hitpoints);
	}
	if (dmgPoints > 0.f) {
		currentState.health = std::max(0.f, currentState.health - dmgPoints);
	}
	return true;
}

void CharacterObject::jump()
{
	if( physCharacter && !jumped ) {
		physCharacter->jump();
		jumped = true;
		animator->playAnimation(AnimIndexMovement, animations.jump_start, 1.f, false);
	}
}

float CharacterObject::getJumpSpeed() const
{
	return jumpSpeed;
}

bool CharacterObject::isOnGround() const
{
	if (physCharacter) {
		return physCharacter->onGround();
	}
	return true;
}

bool CharacterObject::canTurn() const
{
	return isOnGround() && !jumped && isAlive() && controller->getCurrentActivity() == nullptr;
}

void CharacterObject::setJumpSpeed(float speed)
{
	jumpSpeed = speed;
}

void CharacterObject::resetToAINode()
{
	auto nodes = engine->aigraph.nodes;
	bool vehicleNode = !! getCurrentVehicle();
	AIGraphNode* nearest = nullptr; float d = std::numeric_limits<float>::max();
	for(auto it = nodes.begin(); it != nodes.end(); ++it) {
		if(vehicleNode) {
			if((*it)->type == AIGraphNode::Pedestrian) continue;
		}
		else {
			if((*it)->type == AIGraphNode::Vehicle) continue;
		}
		
		float dist = glm::length((*it)->position - getPosition());
		if(dist < d) {
			nearest = *it;
			d = dist;
		}
	}
	
	if(nearest) {
		if(vehicleNode) {
			getCurrentVehicle()->setPosition(nearest->position + glm::vec3(0.f, 0.f, 2.5f));
		}
		else {
			setPosition(nearest->position + glm::vec3(0.f, 0.f, 2.5f));
		}
	}
}

void CharacterObject::playActivityAnimation(Animation* animation, bool repeat, bool blocked)
{
	RW_CHECK(animator != nullptr, "No Animator");
	animator->playAnimation(AnimIndexAction, animation, 1.f, repeat);
	motionBlockedByActivity = blocked;
}

void CharacterObject::activityFinished()
{
	animator->playAnimation(AnimIndexAction, nullptr, 1.f, false);
	motionBlockedByActivity = false;
}

void CharacterObject::addToInventory(InventoryItem *item)
{
	RW_CHECK(item->getInventorySlot() < maxInventorySlots, "Inventory Slot greater than maxInventorySlots");
	if (item->getInventorySlot() < maxInventorySlots) {
		currentState.weapons[item->getInventorySlot()].weaponId = item->getItemID();
	}
}

void CharacterObject::setActiveItem(int slot)
{
	currentState.currentWeapon = slot;
}

InventoryItem *CharacterObject::getActiveItem()
{
	if ( currentVehicle ) return nullptr;
	auto weaponId = currentState.weapons[currentState.currentWeapon].weaponId;
	return engine->getInventoryItem(weaponId);
}

void CharacterObject::removeFromInventory(int slot)
{
	currentState.weapons[slot].weaponId = 0;
}

void CharacterObject::cycleInventory(bool up)
{
	if( up ) {
		for(int j = currentState.currentWeapon+1; j < maxInventorySlots; ++j) {
			if (currentState.weapons[j].weaponId != 0) {
				currentState.currentWeapon = j;
				return;
			}
		}

		// if there's no higher slot, set the first item.
		currentState.currentWeapon = 0;
	}
	else {
		for(int j = currentState.currentWeapon-1; j >= 0; --j) {
			if (currentState.weapons[j].weaponId != 0) {
				currentState.currentWeapon = j;
				return;
			}
		}

		// Nothing? set the highest
		for(int j = maxInventorySlots-1; j >= 0; --j) {
			if (currentState.weapons[j].weaponId != 0 || j == 0) {
				currentState.currentWeapon = j;
				return;
			}
		}
	}
}

void CharacterObject::useItem(bool active, bool primary)
{
	if( getActiveItem() ) {
		if( primary ) {
			if (active)
				currentState.primaryStartTime = engine->getGameTime() * 1000.f;
			else
				currentState.primaryEndTime = engine->getGameTime() * 1000.f;
			currentState.primaryActive = active;
			getActiveItem()->primary(this);
		}
		else {
			currentState.secondaryActive = active;
			getActiveItem()->secondary(this);
		}
	}
}
