// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "camera.hpp"

#include "systems/isystemobject.hpp"

#include "scene/objects/light.hpp"
#include "scene/objects/geometry.hpp"
#include "scene/objects/skybox.hpp"

namespace blunted {

  Camera::Camera(std::string name) : Object(name, e_ObjectType_Camera) {
  }

  Camera::~Camera() {
  }


  void Camera::Init() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ICameraInterpreter *CameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
      CameraInterpreter->OnLoad(properties);
    }

    subjectMutex.unlock();
  }

  void Camera::Exit() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ICameraInterpreter *CameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
      CameraInterpreter->OnUnload();
    }

    Object::Exit();

    subjectMutex.unlock();
  }

  void Camera::SetFOV(float fov) {
    subjectMutex.lock();

    this->fov = fov;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ICameraInterpreter *CameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
      CameraInterpreter->SetFOV(fov);
    }

    subjectMutex.unlock();
  }

  void Camera::SetCapping(float nearCap, float farCap) {
    subjectMutex.lock();

    this->nearCap = nearCap;
    this->farCap = farCap;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ICameraInterpreter *CameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
      CameraInterpreter->SetCapping(nearCap, farCap);
    }

    subjectMutex.unlock();
  }

  void Camera::EnqueueView(std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ICameraInterpreter *CameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
      CameraInterpreter->EnqueueView(GetName(), visibleGeometry, visibleLights, skyboxes);
    }

    subjectMutex.unlock();
  }

  void Camera::Poke(e_SystemType targetSystemType) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ICameraInterpreter *CameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
      if (CameraInterpreter->GetSystemType() == targetSystemType) CameraInterpreter->OnPoke();
    }

    subjectMutex.unlock();
  }

  void Camera::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    InvalidateSpatialData();

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      if (observers.at(i)->GetSystemType() != excludeSystem) {
        ICameraInterpreter *cameraInterpreter = static_cast<ICameraInterpreter*>(observers.at(i).get());
        cameraInterpreter->OnSpatialChange(GetDerivedPosition(), GetDerivedRotation());
      }
    }

    subjectMutex.unlock();
  }

}
