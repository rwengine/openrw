#pragma once
#ifndef _LOADERDFF_HPP_
#define _LOADERDFF_HPP_

#include <loaders/RWBinaryStream.hpp>

#include <vector>
#include <string>
#include <platform/FileHandle.hpp>
#include <data/ResourceHandle.hpp>

class Model;
class GameData;

class DFFLoaderException
{
  std::string _message;

public:
  DFFLoaderException(const std::string& message) : _message(message) {}

  const std::string& which() { return _message; }
};

class LoaderDFF
{
  /**
   * @brief loads a Frame List chunk from stream into model.
   * @param model
   * @param stream
   */
  void readFrameList(Model* model, const RWBStream& stream);

  void readGeometryList(Model* model, const RWBStream& stream);

  void readGeometry(Model* model, const RWBStream& stream);

  void readMaterialList(Model* model, const RWBStream& stream);

  void readMaterial(Model* model, const RWBStream& stream);

  void readTexture(Model* model, const RWBStream& stream);

  void readGeometryExtension(Model* model, const RWBStream& stream);

  void readBinMeshPLG(Model* model, const RWBStream& stream);

  void readAtomic(Model* model, const RWBStream& stream);

public:
  Model* loadFromMemory(FileHandle file);
};

#endif
