#ifndef _SKELETON_HPP_
#define _SKELETON_HPP_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vector>

class ModelFrame;
/**
 * Data class for additional frame transformation and meta data.
 *
 * Provides interfaces to modify and query the visibility of model frames,
 * as well as their transformation. Modified by Animator to animate models.
 */
class Skeleton
{
public:
  struct FrameTransform {
    glm::vec3 translation;
    glm::quat rotation;
  };

  static FrameTransform IdentityTransform;

  struct FrameData {
    FrameTransform a;
    FrameTransform b;
    bool enabled;
  };

  static FrameData IdentityData;

  typedef std::map<unsigned int, FrameData> FramesData;
  typedef std::map<unsigned int, FrameTransform> TransformData;

  Skeleton();

  void setAllData(const FramesData& data);

  const FrameData& getData(unsigned int frameIdx) const;

  void setData(unsigned int frameIdx, const FrameData& data);
  void setEnabled(ModelFrame* frame, bool enabled);

  void setEnabled(unsigned int frameIdx, bool enabled);

  const FrameTransform& getInterpolated(unsigned int frameIdx) const;

  glm::mat4 getMatrix(unsigned int frameIdx) const;
  glm::mat4 getMatrix(ModelFrame* frame) const;

  void interpolate(float alpha);

private:
  FramesData framedata;
  TransformData interpolateddata;
};

#endif