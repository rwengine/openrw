#include <objects/CharacterObject.hpp>
#include <ai/CharacterController.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Animator.hpp>
#include <objects/VehicleObject.hpp>
#include <items/InventoryItem.hpp>
#include <data/Skeleton.hpp>

// TODO: make this not hardcoded
static glm::vec3 enter_offset(0.81756252f, 0.34800607f, -0.486281008f);

const float CharacterObject::DefaultJumpSpeed = 2.f;

CharacterObject::CharacterObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model, std::shared_ptr<CharacterData> data)
: GameObject(engine, pos, rot, model),
  currentVehicle(nullptr), currentSeat(0),
  _hasTargetPosition(false), _activeInventoryItem(0),
  ped(data), physCharacter(nullptr),
  controller(nullptr), jumped(false), jumpSpeed(DefaultJumpSpeed)
{
	mHealth = 100.f;

	// TODO move AnimationGroup creation somewhere else.
	animations.idle = engine->gameData.animations["idle_stance"];
	animations.walk = engine->gameData.animations["walk_player"];
	animations.walk_start = engine->gameData.animations["walk_start"];
	animations.run  = engine->gameData.animations["run_player"];

	animations.walk_right = engine->gameData.animations["walk_player_right"];
	animations.walk_right_start = engine->gameData.animations["walk_start_right"];
	animations.walk_left = engine->gameData.animations["walk_player_left"];
	animations.walk_left_start = engine->gameData.animations["walk_start_left"];

	animations.walk_back = engine->gameData.animations["walk_player_back"];
	animations.walk_back_start = engine->gameData.animations["walk_start_back"];

	animations.jump_start = engine->gameData.animations["jump_launch"];
	animations.jump_glide = engine->gameData.animations["jump_glide"];
	animations.jump_land  = engine->gameData.animations["jump_land"];

	animations.car_sit     = engine->gameData.animations["car_sit"];
	animations.car_sit_low = engine->gameData.animations["car_lsit"];

	animations.car_open_lhs   = engine->gameData.animations["car_open_lhs"];
	animations.car_getin_lhs   = engine->gameData.animations["car_getin_lhs"];
	animations.car_getout_lhs   = engine->gameData.animations["car_getout_lhs"];
	
	animations.car_open_rhs   = engine->gameData.animations["car_open_rhs"];
	animations.car_getin_rhs   = engine->gameData.animations["car_getin_rhs"];
	animations.car_getout_rhs   = engine->gameData.animations["car_getout_rhs"];

	if(model) {
		skeleton = new Skeleton;
		animator = new Animator(model->resource, skeleton);

		createActor();
	}
}

CharacterObject::~CharacterObject()
{
	for(auto p : _inventory) {
		destroyItem(p.first);
	}
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
		physCharacter->setFallSpeed(20.f);
		physCharacter->setUseGhostSweepTest(true);
		physCharacter->setVelocityForTimeInterval(btVector3(1.f, 1.f, 1.f), 1.f);
		physCharacter->setGravity(engine->dynamicsWorld->getGravity().length());
		physCharacter->setJumpSpeed(5.f);

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

	engine->gameData.loadDFF(modelName + ".dff");
	engine->gameData.loadTXD(modelName + ".txd");

	auto& models = engine->gameData.models;
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
	if(physCharacter) {
		// Check to see if the character should be knocked down.
		btManifoldArray   manifoldArray;
		btBroadphasePairArray& pairArray = physObject->getOverlappingPairCache()->getOverlappingPairArray();
		int numPairs = pairArray.size();
	
		for (int i=0;i<numPairs;i++)
		{
			manifoldArray.clear();
			
			const btBroadphasePair& pair = pairArray[i];
	
			//unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
			btBroadphasePair* collisionPair = engine->dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
			if (!collisionPair)
				continue;
	
			if (collisionPair->m_algorithm)
				collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
	
			for (int j=0;j<manifoldArray.size();j++)
			{
				btPersistentManifold* manifold = manifoldArray[j];
				for (int p=0;p<manifold->getNumContacts();p++)
				{
					const btManifoldPoint&pt = manifold->getContactPoint(p);
					if (pt.getDistance() < 0.f)
					{
						auto otherObject = static_cast<const btCollisionObject*>(
							manifold->getBody0() == physObject ? manifold->getBody1() : manifold->getBody0());
						if(otherObject->getUserPointer()) {
							GameObject* object = static_cast<GameObject*>(otherObject->getUserPointer());
							if(object->type() == Vehicle) {
								VehicleObject* vehicle = static_cast<VehicleObject*>(object);
								if(vehicle->physBody->getLinearVelocity().length() > 0.1f) {
									/// @todo play knocked down animation.
								}
							}
						}
					}
				}
			}
		}

		glm::vec3 walkDir;
		glm::vec3 animTranslate;

		if( isAnimationFixed() && animator->getAnimation() != nullptr ) {
			auto d = animator->getRootTranslation() / animator->getAnimation()->duration;
			animTranslate = d * dt;

			if(! model->resource->frames[0]->getChildren().empty() )
			{
				auto root = model->resource->frames[0]->getChildren()[0];
				Skeleton::FrameData fd = skeleton->getData(root->getIndex());
				fd.a.translation -= d * animator->getAnimationTime(1.f);
				skeleton->setData(root->getIndex(), fd);
			}
		}

		position = getPosition();

		walkDir = rotation * animTranslate;

		if( jumped )
		{
			if( physCharacter->onGround() )
			{
				jumped = false;
			}
			else
			{
				walkDir = rotation * glm::vec3(0.f, jumpSpeed * dt, 0.f);
			}
		}

		physCharacter->setWalkDirection(btVector3(walkDir.x, walkDir.y, walkDir.z));

		auto Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
		position = glm::vec3(Pos.x(), Pos.y(), Pos.z());

		// Handle above waist height water.
		auto wi = engine->gameData.getWaterIndexAt(getPosition());
		if( wi != NO_WATER_INDEX ) {
			float wh = engine->gameData.waterHeights[wi];
			auto ws = getPosition();
			wh += engine->gameData.getWaveHeightAt(ws);
			
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
}

void CharacterObject::setPosition(const glm::vec3& pos)
{
	if( physCharacter )
	{
		btVector3 bpos(pos.x, pos.y, pos.z);
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
		if( animator->getAnimation() == animations.car_getout_lhs ) {
			return currentVehicle->getSeatEntryPosition(currentSeat);
		}

		auto v = getCurrentVehicle();
		auto R = glm::mat3_cast(v->getRotation());
		glm::vec3 offset;
		auto o = (animator->getAnimation() == animations.car_getin_lhs) ? enter_offset : glm::vec3();
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
	///  @todo remove immortality
	return true;
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
	mHealth -= dmg.hitpoints;
	return true;
}

void CharacterObject::jump()
{
	if( physCharacter ) {
		physCharacter->jump();
		jumped = true;
	}
}

float CharacterObject::getJumpSpeed() const
{
	return jumpSpeed;
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

void CharacterObject::setTargetPosition(const glm::vec3 &target)
{
	_targetPosition = target;
	_hasTargetPosition = true;
}

void CharacterObject::clearTargetPosition()
{
	_hasTargetPosition = false;
}

void CharacterObject::playAnimation(Animation *animation, bool repeat)
{
	animator->setAnimation(animation, repeat);
}

void CharacterObject::addToInventory(InventoryItem *item)
{
	_inventory[item->getInventorySlot()] = item;
}

void CharacterObject::setActiveItem(int slot)
{
	_activeInventoryItem = slot;
}

InventoryItem *CharacterObject::getActiveItem()
{
	if ( currentVehicle ) return nullptr;
	return _inventory[_activeInventoryItem];
}

void CharacterObject::destroyItem(int slot)
{
	delete _inventory[slot];
	_inventory[slot] = nullptr;
}

void CharacterObject::cycleInventory(bool up)
{
	if( up ) {
		for(auto it	= _inventory.begin(); it != _inventory.end(); ++it) {
			if( it->first < 0 ) continue;
			if( it->first > _activeInventoryItem ) {
				setActiveItem(it->first);
				return;
			}
		}

		// if there's no higher slot, set the first item.
		auto next = _inventory.lower_bound(0);
		if( next != _inventory.end() ) {
			setActiveItem(next->first);
		}
	}
	else {
		for(auto it	= _inventory.rbegin(); it != _inventory.rend(); ++it) {
			if( it->first < 0 ) break;
			if( it->first < _activeInventoryItem ) {
				setActiveItem(it->first);
				return;
			}
		}

		// if there's no lower slot, set the last item.
		auto next = _inventory.rbegin();
		if( next != _inventory.rend() ) {
			setActiveItem(next->first);
		}
	}
}

