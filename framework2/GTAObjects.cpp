#include <renderwure/engine/GTAObjects.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>

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
                if( model->frameNames[fi] == "swaist" ) {
                    this->animposition = keyframe.position;
                    this->animrotation = keyframe.rotation;
                    fmat = glm::mat4(1.0f);
                }
                else {
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
