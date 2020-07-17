// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "joint.hpp"

#include "geometry.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  Joint::Joint(std::string name) : Object(name, e_ObjectType_Joint) {
  }

  Joint::~Joint() {
  }

  void Joint::Exit() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->OnUnload();
    }

    Object::Exit();

    subjectMutex.unlock();
  }

  void Joint::Connect(boost::intrusive_ptr<Geometry> object1, boost::intrusive_ptr<Geometry> object2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->OnLoad(this, object1, object2, anchor, axis1, axis2);
    }

    subjectMutex.unlock();
  }

  void Joint::SetStops(radian lowStop, radian highStop, int paramNum) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetStops(lowStop, highStop, paramNum);
    }

    subjectMutex.unlock();
  }

  // desired velocity
  void Joint::SetVelocity(float velocity, int paramNum) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetVelocity(velocity, paramNum);
    }

    subjectMutex.unlock();
  }

  // maximum force to reach the desired velocity
  void Joint::SetMaxForce(float force, int paramNum) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetMaxForce(force, paramNum);
    }

    subjectMutex.unlock();
  }

  void Joint::SetConstraintForceMixing(float value, int paramNum) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetConstraintForceMixing(value, paramNum);
    }

    subjectMutex.unlock();
  }

  void Joint::SetErrorCorrection(float value, int paramNum) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetErrorCorrection(value, paramNum);
    }

    subjectMutex.unlock();
  }

  // only on hinge2 joints
  void Joint::SetSuspensionConstraintForceMixing(float value) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetSuspensionConstraintForceMixing(value);
    }

    subjectMutex.unlock();
  }

  void Joint::SetSuspensionErrorReduction(float value) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->SetSuspensionErrorReduction(value);
    }

    subjectMutex.unlock();
  }

  void Joint::Poke(e_SystemType targetSystemType) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      if (jointInterpreter->GetSystemType() == targetSystemType) jointInterpreter->OnPoke();
    }

    subjectMutex.unlock();
  }

  void Joint::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    // that's odd, no code here, only this commented block. I don't know either.
/*
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IJointInterpreter *jointInterpreter = static_cast<IJointInterpreter*>(observers.at(i).get());
      jointInterpreter->OnSpatialChange(GetDerivedPosition(), GetDerivedRotation());
    }

    subjectMutex.unlock();
*/
  }

}
