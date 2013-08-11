  #include <renderwure/engine/GTAObjects.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/engine/GTAEngine.hpp>

void GTAObject::updateFrames()
{
    if( model == nullptr ) return;

    for( size_t fi = 0; fi < model->frames.size(); ++fi ) {
        glm::mat4 fmat = glm::mat4(model->frames[fi].defaultRotation);
        fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);

        if( animation && fi < model->frameNames.size() ) {
            AnimationBone* boneanim = animation->bones[model->frameNames[fi]];
            if( boneanim && boneanim->frames.size() > 0 ) {
                auto keyframe = boneanim->getInterpolatedKeyframe(this->animtime);

                // TODO: check for the actual root bone and not just the name.
                if( model->frameNames[fi] != "swaist" ) {
                    fmat = glm::mat4(1.0f) * glm::mat4_cast(keyframe.rotation);

                    // Only add the translation back if is is not present.
                    if( boneanim->type == AnimationBone::R00 ) {
                        fmat[3] = glm::vec4(model->frames[fi].defaultTranslation, 1.f);
                    }
                    else {
                        fmat[3] = glm::vec4(keyframe.position, 1.f);
                    }
                }
            }
        }

        model->frames[fi].matrix = fmat;
    }
}

GTACharacter::GTACharacter(const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped)
: GTAObject(pos, rot, model), ped(ped), currentActivity(None)
{
    btTransform tf;
	tf.setIdentity();
	tf.setOrigin(btVector3(pos.x, pos.y, pos.z));
	
    physObject = new btPairCachingGhostObject;
    physObject->setWorldTransform(tf);
	physShape = new btBoxShape(btVector3(0.25f, 0.25f, 0.5f));
	skeletonOffset = glm::vec3(0.25f)/2.f;
    physObject->setCollisionShape(physShape);
    physObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	physCharacter = new btKinematicCharacterController(physObject, physShape, 0.35f, 2);
}

void GTACharacter::changeAction(Activity newAction, const AnimationSet &animations)
{
    if(currentActivity != newAction) {
        currentActivity = newAction;
        switch( currentActivity ) {
        case Idle:
            if( animation == nullptr || animation->name != "idle_stance" ) {
                animation = animations.at("idle_stance");
                animtime = 0.f;
            }
            break;
        case Walk:
            if( animation == nullptr || animation->name != "walk_civi" ) {
                animation = animations.at("walk_civi");
                animtime = 0.f;
            }
            break;
        }
    }
}

void GTACharacter::updateCharacter()
{
    glm::vec3 direction = rotation * glm::vec3(0.f, 1.f, 0.f);
    float speed = 0.f;
    switch(currentActivity) {
    case Walk:
        speed = .125f;
        break;
    case Run:
        speed = .25f;
        break;
    case Crouch:
        speed = .125f;
        break;
	case Idle:
		speed = 0.f;
		break;
    }
    physCharacter->setWalkDirection(btVector3(direction.x, direction.y, direction.z)*speed);
	
	btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
	position = glm::vec3(Pos.x(), Pos.y(), Pos.z()) + skeletonOffset;
}

void GTACharacter::updateAnimation(float dt)
{
    if( animation != nullptr ) {
        animtime += dt;
	}
}
