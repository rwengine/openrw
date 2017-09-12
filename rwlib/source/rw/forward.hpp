#ifndef RWLIB_FORWARD_HPP
#define RWLIB_FORWARD_HPP

#include <memory>
#include <vector>

// Forward Declarations
class Clump;
class ModelFrame;
struct Geometry;
class Atomic;
class Clump;

// Pointer types
using ModelFramePtr = std::shared_ptr<ModelFrame>;
using GeometryPtr = std::shared_ptr<Geometry>;
using AtomicPtr = std::shared_ptr<Atomic>;
using AtomicList = std::vector<AtomicPtr>;
using ClumpPtr = std::shared_ptr<Clump>;

#endif /* FORWARD_HPP */

