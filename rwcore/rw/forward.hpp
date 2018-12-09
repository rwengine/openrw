#ifndef _LIBRW_RW_FORWARD_HPP_
#define _LIBRW_RW_FORWARD_HPP_

#include <unordered_map>
#include <memory>
#include <vector>

// Forward Declarations
struct Animation;
class Clump;
struct FileContentsInfo;
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
typedef std::unordered_map<std::string, AnimationPtr> AnimationSet;

#endif /* FORWARD_HPP */
