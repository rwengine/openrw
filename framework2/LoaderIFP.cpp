#include <renderwure/loaders/LoaderIFP.hpp>
#include <algorithm>
#include <iostream>

bool findKeyframes(float t, AnimationBone* bone, AnimationKeyframe& f1, AnimationKeyframe& f2, float& alpha)
{
    for(size_t f = 0; f < bone->frames.size(); ++f ) {
        if( t <= bone->frames[f].starttime ) {
            f2 = bone->frames[f];

            if( f == 0 ) {
                if( bone->frames.size() != 1 ) {
                    f1 = bone->frames.back();
                }
                else {
                    f1 = f2;
                }
            }
            else {
                f1 = bone->frames[f-1];
            }

            float tdiff = (f2.starttime - f1.starttime);
            if( tdiff == 0.f ) {
                alpha = 1.f;
            }
            else {
                alpha = (t - f1.starttime) / tdiff;
            }

            return true;
        }
    }
    return false;
}

AnimationKeyframe AnimationBone::getInterpolatedKeyframe(float time)
{
    AnimationKeyframe f1, f2;

    while(time > this->duration) {
        time -= this->duration;
    }

    float alpha;

    if( findKeyframes(time, this, f1, f2, alpha) ) {
        return {
                glm::normalize(glm::mix(f1.rotation, f2.rotation, alpha)),
                glm::mix(f1.position, f2.position, alpha),
                glm::mix(f1.scale, f2.scale, alpha),
                time
        };
    }

    auto fend = frames.back();
    return {
    fend.rotation,
    fend.position,
    glm::vec3(1.f, 1.f, 1.f)
    };
}

glm::vec3 Animation::getAnimationTravel(float time)
{
    auto it = bones.find("swaist");
    if( it != bones.end() ) {
        auto frame = it->second->getInterpolatedKeyframe(time);
        return frame.position;
    }
    return glm::vec3(0.f, 0.f, 0.f);
}

bool LoaderIFP::loadFromMemory(char *data)
{
    size_t data_offs = 0;
    size_t* dataI = &data_offs;

    ANPK* fileRoot = read<ANPK>(data, dataI);
    std::string listname = readString(data, dataI);

    for( size_t a = 0; a < fileRoot->info.entries; ++a ) {
        // something about a name?
        NAME* n = read<NAME>(data, dataI);
        std::string animname = readString(data, dataI);

        Animation* animation = new Animation;
        animation->name = animname;

        size_t animstart = data_offs + 8;
        DGAN* animroot = read<DGAN>(data, dataI);
        std::string infoname = readString(data, dataI);

        for( size_t c = 0; c < animroot->info.entries; ++c ) {
            size_t start = data_offs;
            CPAN* cpan = read<CPAN>(data, dataI);
            ANIM* frames = read<ANIM>(data, dataI);

            AnimationBone* bonedata = new AnimationBone;
            bonedata->name = frames->name;
            bonedata->frames.reserve(frames->frames);

            data_offs += ((8+frames->base.size) - sizeof(ANIM));

            KFRM* frame = read<KFRM>(data, dataI);
            std::string type(frame->base.magic, 4);

            float time = 0.f;

            if(type == "KR00") {
                bonedata->type = AnimationBone::R00;
                for( size_t d = 0; d < frames->frames; ++d ) {
                    glm::quat q = glm::conjugate(*read<glm::quat>(data, dataI));
                    time = *read<float>(data,dataI);
                    bonedata->frames.push_back({
                                                   q,
                                                   glm::vec3(0.f, 0.f, 0.f),
                                                   glm::vec3(1.f, 1.f, 1.f),
                                                   time
                                               });
                }
            }
            else if(type == "KRT0") {
                bonedata->type = AnimationBone::RT0;
                for( size_t d = 0; d < frames->frames; ++d ) {
                    glm::quat q = glm::conjugate(*read<glm::quat>(data, dataI));
                    glm::vec3 p = *read<glm::vec3>(data, dataI);
                    time = *read<float>(data,dataI);
                    bonedata->frames.push_back({
                                                   q,
                                                   p,
                                                   glm::vec3(1.f, 1.f, 1.f),
                                                   time
                                               });
                }
            }
            else if(type == "KRTS") {
                bonedata->type = AnimationBone::RTS;
                for( size_t d = 0; d < frames->frames; ++d ) {
                    glm::quat q = glm::conjugate(*read<glm::quat>(data, dataI));
                    glm::vec3 p = *read<glm::vec3>(data, dataI);
                    glm::vec3 s = *read<glm::vec3>(data, dataI);
                    time = *read<float>(data,dataI);
                    bonedata->frames.push_back({
                                                   q,
                                                   p,
                                                   s,
                                                   time
                                               });
                }
            }

            bonedata->duration = time;
			animation->duration = std::max(bonedata->duration, animation->duration);

            data_offs = start + sizeof(CPAN) + cpan->base.size;

            std::string framename(frames->name);
            std::transform(framename.begin(), framename.end(), framename.begin(), ::tolower );

            animation->bones.insert({
                                        framename,
                                        bonedata
                                    });
        }

        data_offs = animstart + animroot->base.size;

        std::transform(animname.begin(), animname.end(), animname.begin(), ::tolower );
        animations.insert({ animname, animation });
    }

    return true;
}

std::string LoaderIFP::readString(char *data, size_t *ofs)
{
    size_t b = *ofs;
    for(size_t o = *ofs; o = *ofs;) {
        *ofs += 4;
        if(data[o+0] == 0) break;
        if(data[o+1] == 0) break;
        if(data[o+2] == 0) break;
        if(data[o+3] == 0) break;
    }
    return std::string(data+b);
}
