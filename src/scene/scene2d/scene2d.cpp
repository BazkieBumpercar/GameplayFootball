#include "scene2d.hpp"

// todo
#include "scene/objects/image2d.hpp"
#include "scene/objects/geometry.hpp"

#include "systems/isystemscene.hpp"

#include "managers/environmentmanager.hpp"
#include "managers/systemmanager.hpp"

#include "scene/objectfactory.hpp"

namespace blunted {

  Scene2D::Scene2D(const std::string &name, const Properties &config) : Scene(name, e_SceneType_Scene2D) {
    //printf("CREATING SCENE2D\n");

    supportedObjectTypes.push_back(e_ObjectType_Image2D);

    width = config.GetInt("context_x", 1280);
    height = config.GetInt("context_y", 720);
    bpp = config.GetInt("context_bpp", 32);
  }

  Scene2D::~Scene2D() {
    //printf("DELETING SCENE2D\n");
  }

  void Scene2D::Init() {
  }

  void Scene2D::Exit() { // ATOMIC
    //printf("EXITING SCENE2D\n");
    objects.Lock();
    for (int i = 0; i < (signed int)objects.data.size(); i++) {
      objects.data.at(i)->Exit(); //now in spatial intrusiveptr
      objects.data.at(i).reset();
    }
    objects.data.clear();
    objects.Unlock();

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IScene2DInterpreter *scene2DInterpreter = static_cast<IScene2DInterpreter*>(observers.at(i).get());
      scene2DInterpreter->OnUnload();
    }

    Scene::Exit();

    subjectMutex.unlock();
  }

  void Scene2D::AddObject(boost::intrusive_ptr<Object> object) {
    if (!SupportedObjectType(object->GetObjectType())) {
      Log(e_FatalError, "Scene2D", "AddObject", "Object type not supported");
    }
    objects.Lock();
    objects->push_back(object);
    objects.Unlock();
  }

  void Scene2D::DeleteObject(boost::intrusive_ptr<Object> object) {
    objects.Lock();
    std::vector <boost::intrusive_ptr<Object> >::iterator objIter = objects.data.begin();
    while (objIter != objects.data.end()) {
      if ((*objIter) == object) {
        (*objIter)->Exit();
        objIter = objects.data.erase(objIter);
      } else {
        objIter++;
      }
    }
    objects.Unlock();
  }

  void Scene2D::RemoveObject(boost::intrusive_ptr<Object> object) {
    objects.Lock();
    std::vector <boost::intrusive_ptr<Object> >::iterator objIter = objects.data.begin();
    while (objIter != objects.data.end()) {
      if ((*objIter) == object) {
        objIter = objects.data.erase(objIter);
      } else {
        objIter++;
      }
    }
    objects.Unlock();
  }

  void Scene2D::GetObjects(e_ObjectType targetObjectType, std::vector < boost::intrusive_ptr<Object> > &gatherObjects) {
    if (!SupportedObjectType(targetObjectType)) return;
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.data.at(i)->GetObjectType() == targetObjectType) gatherObjects.push_back(objects.data.at(i));
    }
    objects.Unlock();
  }

  bool SortObjects(const boost::intrusive_ptr<Object> &a, const boost::intrusive_ptr<Object> &b) {
    return a->GetPokePriority() < b->GetPokePriority();
  }

  void Scene2D::PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) {
    if (!SupportedObjectType(targetObjectType)) return;
    objects.Lock();
    std::stable_sort(objects.data.begin(), objects.data.end(), SortObjects);
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.data.at(i)->IsEnabled()) if (objects.data.at(i)->GetObjectType() == targetObjectType) objects.data.at(i)->Poke(targetSystemType);
    }
    objects.Unlock();
  }

  void Scene2D::GetContextSize(int &width, int &height, int &bpp) {
    width = this->width;
    height = this->height;
    bpp = this->bpp;
  }

  Vector3 Scene2D::GetContextSize() {
    Vector3 size;
    size.coords[0] = width;
    size.coords[1] = height;
    size.coords[2] = bpp;
    return size;
  }

}
