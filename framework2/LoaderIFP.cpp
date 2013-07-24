#include <renderwure/loaders/LoaderIFP.hpp>

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
            bonedata->time = frame->time;

            if(type == "KR00") {
                for( size_t d = 0; d < frames->frames; ++d ) {
                    bonedata->frames.push_back({
                                                   *read<glm::quat>(data, dataI),
                                                   glm::vec3(0.f, 0.f, 0.f),
                                                   glm::vec3(1.f, 1.f, 1.f)
                                               });
                }
            }
            else if(type == "KRT0") {
                for( size_t d = 0; d < frames->frames; ++d ) {
                    bonedata->frames.push_back({
                                                   *read<glm::quat>(data, dataI),
                                                   *read<glm::vec3>(data, dataI),
                                                   glm::vec3(1.f, 1.f, 1.f)
                                               });
                }
            }
            else if(type == "KRTS") {
                for( size_t d = 0; d < frames->frames; ++d ) {
                    bonedata->frames.push_back({
                                                   *read<glm::quat>(data, dataI),
                                                   *read<glm::vec3>(data, dataI),
                                                   *read<glm::vec3>(data, dataI)
                                               });
                }
            }

            data_offs = start + sizeof(CPAN) + cpan->base.size;

            animation->bones.insert({
                                        std::string(frames->name),
                                        bonedata
                                    });
        }

        data_offs = animstart + animroot->base.size;

        animations.push_back(animation);
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
