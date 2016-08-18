#pragma once

#include <memory>
#include <string>

namespace RW
{
/**
 * Possible states for ResourceHandle
 */
enum ResourceState {
  /// Resource has been declared but not loaded
  Loading = 0,
  /// Resource has been loaded and is available
  Loaded = 1,
  /// Loading the resource failed
  Failed = 2
};
}

template <class T>
class ResourceHandle
{
public:
  T* resource;
  RW::ResourceState state;
  std::string name;

  typedef std::shared_ptr<ResourceHandle<T>> Ref;

  ResourceHandle(const std::string& name) : resource(nullptr), state(RW::Loading), name(name) {}
};
