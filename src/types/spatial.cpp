// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "spatial.hpp"

namespace blunted {

  Spatial::Spatial(const std::string &name) : name(name), parent(0), localMode(e_LocalMode_Relative) {
    scale.Set(1, 1, 1);
    Vector axis(0, 0, -1);
    rotation.SetAngleAxis(0, axis);
    position.Set(0, 0, 0);
    aabb.data.aabb.Reset();
    aabb.data.dirty = false;
    InvalidateSpatialData();
  }

  Spatial::~Spatial() {
    parent = 0;
  }

  Spatial::Spatial(const Spatial &src) {
    name = src.GetName();
    position = src.position;
    rotation = src.rotation;
    scale = src.scale;
    localMode = src.localMode;
    aabb.data.aabb = src.GetAABB();
    aabb.data.dirty = false;
    parent = 0;
    InvalidateSpatialData();
  }

  void Spatial::SetLocalMode(e_LocalMode localMode) {
    this->localMode = localMode;
    InvalidateBoundingVolume();
  }

  bool Spatial::GetLocalMode() {
    return localMode;
  }

  void Spatial::SetName(const std::string &name) {
    spatialMutex.lock();
    this->name = name;
    spatialMutex.unlock();
  }

  const std::string Spatial::GetName() const {
    boost::mutex::scoped_lock blah(spatialMutex);
    return name.c_str();
  }

  void Spatial::SetParent(Spatial *parent) {
    this->parent = parent;
    InvalidateBoundingVolume();
  }

  Spatial *Spatial::GetParent() const {
    return parent;
  }

  void Spatial::SetPosition(const Vector3 &newPosition, bool updateSpatialData) {
    spatialMutex.lock();
    position = newPosition;
    spatialMutex.unlock();
    if (updateSpatialData) RecursiveUpdateSpatialData(e_SpatialDataType_Position);
  }

  Vector3 Spatial::GetPosition() const {
    spatialMutex.lock();
    Vector3 pos = position;
    spatialMutex.unlock();
    return pos;
  }

  void Spatial::SetRotation(const Quaternion &newRotation, bool updateSpatialData) {
    spatialMutex.lock();
    rotation = newRotation;
    spatialMutex.unlock();
    if (updateSpatialData) RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }

  Quaternion Spatial::GetRotation() const {
    spatialMutex.lock();
    Quaternion rot = rotation;
    spatialMutex.unlock();
    return rot;
  }

  void Spatial::SetScale(const Vector3 &newScale) {
    spatialMutex.lock();
    this->scale = newScale;
    spatialMutex.unlock();
    RecursiveUpdateSpatialData(e_SpatialDataType_Rotation);
  }

  Vector3 Spatial::GetScale() const {
    spatialMutex.lock();
    Vector3 retScale = scale;
    spatialMutex.unlock();
    return retScale;
  }

  Vector3 Spatial::GetDerivedPosition() const {
    boost::mutex::scoped_lock cachelock(cacheMutex);
    if (_dirty_DerivedPosition) {
      if (localMode == e_LocalMode_Relative) {
        if (parent) {
          const Quaternion parentDerivedRotation = parent->GetDerivedRotation();
          const Vector3 parentDerivedScale = parent->GetDerivedScale();
          const Vector3 parentDerivedPosition = parent->GetDerivedPosition();

          _cache_DerivedPosition.Set(parentDerivedRotation * (parentDerivedScale * GetPosition()));
          _cache_DerivedPosition += parentDerivedPosition;
        } else {
          _cache_DerivedPosition = GetPosition();
        }
      } else {
        _cache_DerivedPosition = GetPosition();
      }
      _dirty_DerivedPosition = false;
    }
    return _cache_DerivedPosition;
  }

  Quaternion Spatial::GetDerivedRotation() const {
    boost::mutex::scoped_lock cachelock(cacheMutex);
    if (_dirty_DerivedRotation) {
      if (localMode == e_LocalMode_Relative) {
        if (parent) {
          _cache_DerivedRotation = (parent->GetDerivedRotation() * GetRotation()).GetNormalized();
        } else {
          _cache_DerivedRotation = GetRotation();
        }
      } else {
        _cache_DerivedRotation = GetRotation();
      }
      _dirty_DerivedRotation = false;
    }
    return _cache_DerivedRotation;
  }

  Vector3 Spatial::GetDerivedScale() const {
    boost::mutex::scoped_lock cachelock(cacheMutex);
    if (_dirty_DerivedScale) {
      if (localMode == e_LocalMode_Relative) {
        if (parent) {
          _cache_DerivedScale = parent->GetDerivedScale() * GetScale();
        } else {
          _cache_DerivedScale = GetScale();
        }
      } else {
        _cache_DerivedScale = GetScale();
      }
      _dirty_DerivedScale = false;
    }
    return _cache_DerivedScale;
  }

  void Spatial::InvalidateBoundingVolume() {
    bool changed = false;
    aabb.Lock();
    if (aabb.data.dirty == false) {
      aabb.data.dirty = true;
      aabb.data.aabb.Reset();
      changed = true;
    }
    aabb.Unlock();

    if (changed) if (parent) parent->InvalidateBoundingVolume();
  }

  void Spatial::InvalidateSpatialData() {
    cacheMutex.lock();
    _dirty_DerivedPosition = true;
    _dirty_DerivedRotation = true;
    _dirty_DerivedScale = true;
    cacheMutex.unlock();
  }


  AABB Spatial::GetAABB() const {
    AABB tmp;
    aabb.Lock();
    tmp = aabb.data.aabb;
    aabb.Unlock();
    return tmp;
  }

}
