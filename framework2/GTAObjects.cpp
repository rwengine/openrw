#include <renderwure/engine/GTAObjects.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/ai/GTAAIController.hpp>

void GTAObject::updateFrames()
{
    if( model == nullptr ) return;

    for( size_t fi = 0; fi < model->frames.size(); ++fi ) {
        glm::mat4 fmat = glm::mat4(model->frames[fi].defaultRotation);
        fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);

        if( animation && fi < model->frameNames.size() ) {
			animtime = fmod(animtime, animation->duration);
			if( lastAnimtime > animtime ) {
				lastRootPosition = rootPosition = glm::vec3(0.f);
			}
			
            AnimationBone* boneanim = animation->bones[model->frameNames[fi]];
            if( boneanim && boneanim->frames.size() > 0 ) {
                auto keyframe = boneanim->getInterpolatedKeyframe(this->animtime);
				fmat = glm::mat4(1.0f) * glm::mat4_cast(keyframe.rotation);

				// Only add the translation back if is is not present.
				if( boneanim->type == AnimationBone::R00 ) {
					fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);
				}
				else {
					if( model->frameNames[fi] == "swaist" ) {
						// Track the root bone.
						lastRootPosition = rootPosition;
						rootPosition = keyframe.position;
						fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);
					}
					else {
						fmat[3] = glm::vec4(keyframe.position, 1.f);
					}
				}
            }
        }
        
        lastAnimtime = animtime;

        model->frames[fi].matrix = fmat;
    }
}

void GTAObject::setPosition(const glm::vec3& pos)
{
	position = pos;
}

glm::vec3 GTAObject::getPosition() const
{
	return position;
}

GTACharacter::GTACharacter(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped)
: GTAObject(engine, pos, rot, model), ped(ped), currentActivity(None), controller(nullptr)
{
    btTransform tf;
	tf.setIdentity();
	tf.setOrigin(btVector3(pos.x, pos.y, pos.z));
	
    physObject = new btPairCachingGhostObject;
    physObject->setWorldTransform(tf);
	physShape = new btBoxShape(btVector3(0.25f, 0.25f, 0.5f));
    physObject->setCollisionShape(physShape);
    physObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	physCharacter = new btKinematicCharacterController(physObject, physShape, 0.35f, 2);
}

void GTACharacter::changeAction(Activity newAction)
{
    if(currentActivity != newAction) {
        currentActivity = newAction;
        switch( currentActivity ) {
        case Idle:
            if( animation == nullptr || animation->name != "idle_stance" ) {
                animation = engine->gameData.animations.at("idle_stance");
                animtime = 0.f;
            }
            break;
        case Walk:
            if( animation == nullptr || animation->name != "walk_civi" ) {
                animation = engine->gameData.animations.at("walk_civi");
                animtime = 0.f;
            }
            break;
		case Run:
            if( animation == nullptr || animation->name != "run_civi" ) {
                animation = engine->gameData.animations.at("run_civi");
                animtime = 0.f;
            }
			break;
        }
    }
}

void GTACharacter::updateCharacter()
{
	if( controller ) {
		rotation = glm::normalize(controller->getTargetRotation());
	}
	
    glm::vec3 direction = rotation * (rootPosition - lastRootPosition);
    physCharacter->setWalkDirection(btVector3(direction.x, direction.y, direction.z));
	
	btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
	position = glm::vec3(Pos.x(), Pos.y(), Pos.z());
}

void GTACharacter::updateAnimation(float dt)
{
    if( animation != nullptr ) {
        animtime += dt;
	}
}

void GTACharacter::setPosition(const glm::vec3& pos)
{
    btTransform& tf = physCharacter->getGhostObject()->getWorldTransform();
	tf.setOrigin(btVector3(pos.x, pos.y, pos.z));
	position = pos;
}

glm::vec3 GTACharacter::getPosition() const
{
	btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
	return glm::vec3(Pos.x(), Pos.y(), Pos.z());
}
