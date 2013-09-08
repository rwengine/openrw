#include <renderwure/engine/GTAObject.hpp>
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
