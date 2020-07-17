// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "geometry.hpp"

#include "base/log.hpp"

#include "systems/isystemobject.hpp"

#include "managers/resourcemanagerpool.hpp"

namespace blunted {

  Geometry::Geometry(std::string name, e_ObjectType objectType) : Object(name, objectType) {
  }

  Geometry::Geometry(const Geometry &src, const std::string &postfix) : Object(src) {
    if (src.geometryData != boost::intrusive_ptr < Resource<GeometryData> >()) {
      src.geometryData->resourceMutex.lock();
      std::string srcName = src.geometryData->GetIdentString();
      src.geometryData->resourceMutex.unlock();
      bool alreadyThere = false;

      // todo: make selectable if we want to deep copy or not
      geometryData = (ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->FetchCopy(srcName, srcName + postfix, alreadyThere));
      //geometryData = (ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch(srcName, false, alreadyThere, true));
    }
    InvalidateBoundingVolume();
  }

  Geometry::~Geometry() {
  }

  void Geometry::Exit() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->OnUnload();
    }

    Object::Exit();

    if (geometryData) geometryData.reset();

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  void Geometry::SetGeometryData(boost::intrusive_ptr < Resource<GeometryData> > geometryData) {
    subjectMutex.lock();

    // todo: check if already pointing to existing data and maybe delete?
    this->geometryData = geometryData;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->OnLoad(this);
    }

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  void Geometry::OnUpdateGeometryData(bool updateMaterials) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->OnUpdateGeometry(this, updateMaterials);
    }

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  boost::intrusive_ptr < Resource<GeometryData> > Geometry::GetGeometryData() {
    return geometryData;
  }

  void Geometry::Poke(e_SystemType targetSystemType) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      // todo: cache the interpreters?
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      if (geometryInterpreter->GetSystemType() == targetSystemType) geometryInterpreter->OnPoke();
    }

    subjectMutex.unlock();

    // did a system object feedback a new pos/rot?
    updateSpatialDataAfterPoke.Lock();
    // todo: does this work when multiple systems want to exclude themselves? i guess not.. make it (excludes) an array?
    if (updateSpatialDataAfterPoke.data.haveTo == true) {
      RecursiveUpdateSpatialData(e_SpatialDataType_Both, updateSpatialDataAfterPoke.data.excludeSystem);
      MustUpdateSpatialData clear;
      clear.haveTo = false;
      clear.excludeSystem = e_SystemType_None;
      updateSpatialDataAfterPoke.data = clear;
    }
    updateSpatialDataAfterPoke.Unlock();
  }

  void Geometry::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {

    InvalidateSpatialData();
    InvalidateBoundingVolume();

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      //printf("%i %i\n", observers.at(i)->GetSystemType(), excludeSystem);
      if (observers.at(i)->GetSystemType() != excludeSystem) {
        IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
        if (spatialDataType == e_SpatialDataType_Position) {
          geometryInterpreter->OnMove(GetDerivedPosition());
        }
        else if (spatialDataType == e_SpatialDataType_Rotation) {
          // need to update both: position relies on rotation
          geometryInterpreter->OnMove(GetDerivedPosition());
          geometryInterpreter->OnRotate(GetDerivedRotation());
        }
        else if (spatialDataType == e_SpatialDataType_Both) {
          geometryInterpreter->OnMove(GetDerivedPosition());
          geometryInterpreter->OnRotate(GetDerivedRotation());
        }
      }
    }

    subjectMutex.unlock();

    //Object::RecursiveUpdateSpatialData(spatialDataType);
  }

  AABB Geometry::GetAABB() const {

    aabb.Lock();

    if (aabb.data.dirty == true) {
      geometryData->resourceMutex.lock();
      assert(geometryData->GetResource());
      aabb.data.aabb = geometryData->GetResource()->GetAABB() * GetDerivedRotation() + GetDerivedPosition();
      geometryData->resourceMutex.unlock();
      aabb.data.dirty = false;
    }

    AABB tmp = aabb.data.aabb;

    aabb.Unlock();

    return tmp;
  }

  void Geometry::ApplyForceAtRelativePosition(float force, const Vector3 &direction, const Vector3 &position) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IGeometryInterpreter *geometryInterpreter = static_cast<IGeometryInterpreter*>(observers.at(i).get());
      geometryInterpreter->ApplyForceAtRelativePosition(force, direction, position);
    }

    subjectMutex.unlock();
  }

}
