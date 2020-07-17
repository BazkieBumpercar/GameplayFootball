// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "object.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  Object::Object(std::string name, e_ObjectType objectType) : Spatial(name), objectType(objectType) {
    MustUpdateSpatialData clear;
    clear.haveTo = false;
    clear.excludeSystem = e_SystemType_None;
    updateSpatialDataAfterPoke.SetData(clear);
    pokePriority.SetData(0);
    enabled = true;
  }

  Object::~Object() {
    if (observers.size() != 0) {
      Log(e_FatalError, "Object", "~Object", "Observer(s) still present at destruction time (spatial named: " + GetName() + ")");
    }
  }

  Object::Object(const Object &src) : Subject<Interpreter>(), Spatial(src) {
    objectType = src.objectType;
    properties = src.properties;
    requestProperties->AddProperties(src.requestProperties.GetData());

    MustUpdateSpatialData clear;
    clear.haveTo = false;
    clear.excludeSystem = e_SystemType_None;
    updateSpatialDataAfterPoke.SetData(clear);
    pokePriority = src.GetPokePriority();
    enabled = src.enabled;

    assert(observers.size() == 0);
  }

  void Object::Exit() {
    //printf("detaching all observers from object named %s\n", GetName().c_str());
    DetachAll();
  }

  inline e_ObjectType Object::GetObjectType() {
    return objectType;
  }

  const Properties &Object::GetProperties() const {
    return properties;
  }

  bool Object::PropertyExists(const char *property) const {
    bool exists = properties.Exists(property);
    return exists;
  }

  const std::string &Object::GetProperty(const char *property) const {
    return properties.Get(property);
  }

  void Object::SetProperties(Properties newProperties) {
    properties.AddProperties(&newProperties);
  }

  void Object::SetProperty(const char *name, const char *value) {
    properties.Set(name, value);
  }

  bool Object::RequestPropertyExists(const char *property) const {
    requestProperties.Lock();
    bool exists = requestProperties->Exists(property);
    requestProperties.Unlock();
    return exists;
  }

  std::string Object::GetRequestProperty(const char *property) const {
    requestProperties.Lock();
    std::string result = requestProperties->Get(property);
    requestProperties.Unlock();
    return result;
  }

  void Object::AddRequestProperty(const char *property) {
    requestProperties.Lock();
    requestProperties->Set(property, "");
    requestProperties.Unlock();
  }

  void Object::SetRequestProperty(const char *property, const char *value) {
    requestProperties.Lock();
    requestProperties->Set(property, value);
    requestProperties.Unlock();
  }

  void Object::Synchronize() {
    subjectMutex.lock();
    boost::shared_ptr<Interpreter> result;
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      boost::intrusive_ptr<Interpreter> interpreter = static_pointer_cast<Interpreter>(observers.at(i));
      interpreter->OnSynchronize();
    }
    subjectMutex.unlock();
  }

  void Object::Poke(e_SystemType targetSystemType) {
  }

  inline void Object::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
  }

  boost::intrusive_ptr<Interpreter> Object::GetInterpreter(e_SystemType targetSystemType) {
    subjectMutex.lock();
    boost::intrusive_ptr<Interpreter> result;
    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      boost::intrusive_ptr<Interpreter> interpreter = static_pointer_cast<Interpreter>(observers.at(i));
      if (interpreter->GetSystemType() == targetSystemType) result = interpreter;
    }
    subjectMutex.unlock();

    return result;
  }

  void Object::LockSubject() {
    subjectMutex.lock();
  }

  void Object::UnlockSubject() {
    subjectMutex.unlock();
  }

}
