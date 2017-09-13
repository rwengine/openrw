#ifndef RWLIB_FORWARD_HPP
#define RWLIB_FORWARD_HPP

#include <map>
#include <memory>
#include <vector>

// Forward Declarations
struct Animation;
class Clump;
class ModelFrame;
struct Geometry;
class Atomic;
class Clump;

// Pointer types
using AnimationPtr = std::shared_ptr<Animation>;
using ModelFramePtr = std::shared_ptr<ModelFrame>;
using GeometryPtr = std::shared_ptr<Geometry>;
using AtomicPtr = std::shared_ptr<Atomic>;
using ClumpPtr = std::shared_ptr<Clump>;

// Collections
using AtomicList = std::vector<AtomicPtr>;
typedef std::map<std::string, AnimationPtr> AnimationSet;

#endif /* FORWARD_HPP */
