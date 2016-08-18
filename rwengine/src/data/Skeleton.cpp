#include <data/Skeleton.hpp>
#include <data/Model.hpp>
#include <glm/gtc/matrix_transform.hpp>

Skeleton::FrameTransform Skeleton::IdentityTransform = {glm::vec3(0.f), glm::quat()};
Skeleton::FrameData Skeleton::IdentityData = {Skeleton::IdentityTransform,
                                              Skeleton::IdentityTransform, true};

Skeleton::Skeleton() {}

void Skeleton::setAllData(const Skeleton::FramesData& data) { framedata = data; }

const Skeleton::FrameData& Skeleton::getData(unsigned int frameIdx) const
{
  auto fdit = framedata.find(frameIdx);
  if (fdit == framedata.end()) {
    return Skeleton::IdentityData;
  }

  return fdit->second;
}

void Skeleton::setData(unsigned int frameIdx, const Skeleton::FrameData& data)
{
  framedata[frameIdx] = data;
}

void Skeleton::setEnabled(ModelFrame* frame, bool enabled)
{
  auto fdit = framedata.find(frame->getIndex());
  if (fdit != framedata.end()) {
    fdit->second.enabled = enabled;
  } else {
    FrameTransform tf{frame->getDefaultTranslation(), glm::quat_cast(frame->getDefaultRotation())};
    framedata.insert({frame->getIndex(), {tf, tf, enabled}});
  }
}

void Skeleton::setEnabled(unsigned int frameIdx, bool enabled)
{
  auto fdit = framedata.find(frameIdx);
  if (fdit == framedata.end()) {
    framedata.insert(
        {frameIdx, {Skeleton::IdentityTransform, Skeleton::IdentityTransform, enabled}});
  } else {
    fdit->second.enabled = enabled;
  }
}

const Skeleton::FrameTransform& Skeleton::getInterpolated(unsigned int frameIdx) const
{
  auto itit = interpolateddata.find(frameIdx);
  if (itit == interpolateddata.end()) {
    return Skeleton::IdentityTransform;
  }

  return itit->second;
}

void Skeleton::interpolate(float alpha)
{
  interpolateddata.clear();

  for (auto i = framedata.begin(); i != framedata.end(); ++i) {
    auto& t2 = i->second.a.translation;
    auto& t1 = i->second.b.translation;

    auto& r2 = i->second.a.rotation;
    auto& r1 = i->second.b.rotation;

    interpolateddata.insert({i->first, {glm::mix(t1, t2, alpha), glm::slerp(r1, r2, alpha)}});
  }
}

glm::mat4 Skeleton::getMatrix(unsigned int frameIdx) const
{
  const FrameTransform& ft = getInterpolated(frameIdx);

  glm::mat4 m;

  m = glm::translate(m, ft.translation);
  m = m * glm::mat4_cast(ft.rotation);

  return m;
}

glm::mat4 Skeleton::getMatrix(ModelFrame* frame) const
{
  auto itit = interpolateddata.find(frame->getIndex());
  if (itit != interpolateddata.end()) {
    glm::mat4 m;

    m = glm::translate(m, itit->second.translation);
    m = m * glm::mat4_cast(itit->second.rotation);

    return m;
  }

  return frame->getTransform();
}
