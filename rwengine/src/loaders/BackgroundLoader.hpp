#pragma once

#include <job/WorkContext.hpp>
#include <data/ResourceHandle.hpp>
#include <platform/FileIndex.hpp>

/**
 * Implementation of a worker that loads a resource in the background.
 */
template <class T, class L>
class BackgroundLoaderJob : public WorkJob
{
public:
  typedef typename ResourceHandle<T>::Ref TypeRef;

  BackgroundLoaderJob(WorkContext* context, FileIndex* index, const std::string& file,
                      const TypeRef& ref)
      : WorkJob(context), index(index), filename(file), resourceRef(ref)
  {
  }

  void work() { data = index->openFile(filename); }

  void complete()
  {
    if (data) {
      L loader;

      resourceRef->resource = loader.loadFromMemory(data);
      resourceRef->state = RW::Loaded;
    }
  }

private:
  FileIndex* index;
  std::string filename;
  FileHandle data;
  TypeRef resourceRef;
};
