// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "scene.hpp"

namespace blunted {

  Scene::Scene(std::string name, e_SceneType sceneType) : name(name), sceneType(sceneType) {
  }

  Scene::~Scene() {
  }

  void Scene::Exit() {
    DetachAll();
  }

  void Scene::CreateSystemObjects(boost::intrusive_ptr<Object> object) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISceneInterpreter *sceneInterpreter = static_cast<ISceneInterpreter*>(observers.at(i).get());
      sceneInterpreter->CreateSystemObject(object);
    }

    subjectMutex.unlock();
  }

  const std::string Scene::GetName() const {
    return name;
  }

  e_SceneType Scene::GetSceneType() const {
    return sceneType;
  }

  bool Scene::SupportedObjectType(e_ObjectType objectType) const {
    for (int i = 0; i < (signed int)supportedObjectTypes.size(); i++) {
      if (objectType == supportedObjectTypes.at(i)) return true;
    }
    return false;
  }

}
