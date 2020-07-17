// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "light.hpp"

#include "camera.hpp"
#include "geometry.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  Light::Light(std::string name) : Object(name, e_ObjectType_Light) {
    radius = 512;
    color.Set(1, 1, 1);
    lightType = e_LightType_Point;
    shadow = false;
  }

  Light::~Light() {
  }

  void Light::Exit() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
      LightInterpreter->OnUnload();
    }

    Object::Exit();

    subjectMutex.unlock();
  }

  void Light::SetColor(const Vector3 &color) {
    subjectMutex.lock();
    this->color = color;
    subjectMutex.unlock();
    UpdateValues();
  }

  Vector3 Light::GetColor() const {
    subjectMutex.lock();
    Vector3 retColor = color;
    subjectMutex.unlock();
    return retColor;

  }

  void Light::SetRadius(float radius) {
    subjectMutex.lock();
    this->radius = radius;
    subjectMutex.unlock();
    UpdateValues();

    InvalidateBoundingVolume();
  }

  float Light::GetRadius() const {
    subjectMutex.lock();
    float rad = radius;
    subjectMutex.unlock();
    return rad;
  }

  void Light::SetType(e_LightType lightType) {
    subjectMutex.lock();

    this->lightType = lightType;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
      LightInterpreter->SetType(lightType);
    }

    subjectMutex.unlock();
  }

  e_LightType Light::GetType() const {
    subjectMutex.lock();
    e_LightType theType = lightType;
    subjectMutex.unlock();
    return theType;
  }

  void Light::SetShadow(bool shadow) {
    subjectMutex.lock();

    this->shadow = shadow;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
      LightInterpreter->SetShadow(shadow);
    }

    subjectMutex.unlock();
  }

  bool Light::GetShadow() const {
    subjectMutex.lock();
    bool tmp = shadow;
    subjectMutex.unlock();
    return tmp;
  }

  void Light::UpdateValues() {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
      LightInterpreter->SetValues(color, radius);
    }

    subjectMutex.unlock();
  }

  void Light::EnqueueShadowMap(boost::intrusive_ptr<Camera> camera, std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
      LightInterpreter->EnqueueShadowMap(camera, visibleGeometry);
    }

    subjectMutex.unlock();
  }

  void Light::Poke(e_SystemType targetSystemType) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ILightInterpreter *LightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
      if (LightInterpreter->GetSystemType() == targetSystemType) LightInterpreter->OnPoke();
    }

    subjectMutex.unlock();
  }

  void Light::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    InvalidateSpatialData();
    InvalidateBoundingVolume();

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      if (observers.at(i)->GetSystemType() != excludeSystem) {
        ILightInterpreter *lightInterpreter = static_cast<ILightInterpreter*>(observers.at(i).get());
        lightInterpreter->OnSpatialChange(GetDerivedPosition(), GetDerivedRotation());
      }
    }

    subjectMutex.unlock();
  }

  AABB Light::GetAABB() const {
    aabb.Lock();
    if (aabb.data.dirty == true) {
      Vector3 pos = GetDerivedPosition();
      aabb.data.aabb.minxyz = pos - radius;
      aabb.data.aabb.maxxyz = pos + radius;
      aabb.data.aabb.MakeDirty();
      aabb.data.dirty = false;
    }
    AABB tmp = aabb.data.aabb;
    aabb.Unlock();
    return tmp;
  }

}
