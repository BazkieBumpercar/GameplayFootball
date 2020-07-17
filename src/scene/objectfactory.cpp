// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "objectfactory.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

#include "scene/objects/camera.hpp"
#include "scene/objects/image2d.hpp"
#include "scene/objects/geometry.hpp"
#include "scene/objects/skybox.hpp"
#include "scene/objects/light.hpp"
#include "scene/objects/joint.hpp"
#include "scene/objects/audioreceiver.hpp"
#include "scene/objects/sound.hpp"

namespace blunted {

  template<> ObjectFactory* Singleton<ObjectFactory>::singleton = 0;

  ObjectFactory::ObjectFactory() {
  }

  ObjectFactory::~ObjectFactory() {
  }

  void ObjectFactory::Exit() {
    std::map <e_ObjectType, boost::intrusive_ptr<Object> >::iterator prototype_iter = prototypes.begin();
    while (prototype_iter != prototypes.end()) {
      prototype_iter->second->Exit();
      prototype_iter++;
    }
    prototypes.clear();
  }

  boost::intrusive_ptr<Object> ObjectFactory::CreateObject(const std::string &name, std::string objectTypeStr, std::string postfix) {
    e_ObjectType objectType = e_ObjectType_Geometry;
    if (objectTypeStr == "Camera") objectType = e_ObjectType_Camera; else
    if (objectTypeStr == "Image2D") objectType = e_ObjectType_Image2D; else
    if (objectTypeStr == "Geometry") objectType = e_ObjectType_Geometry; else
    if (objectTypeStr == "Skybox") objectType = e_ObjectType_Skybox; else
    if (objectTypeStr == "Light") objectType = e_ObjectType_Light; else
    if (objectTypeStr == "Joint") objectType = e_ObjectType_Joint; else
    if (objectTypeStr == "AudioReceiver") objectType = e_ObjectType_AudioReceiver; else
    if (objectTypeStr == "Sound") objectType = e_ObjectType_Sound;

    return CreateObject(name, objectType, postfix);
  }

  boost::intrusive_ptr<Object> ObjectFactory::CreateObject(const std::string &name, e_ObjectType objectType, std::string postfix) {
    std::map <e_ObjectType, boost::intrusive_ptr<Object> >::iterator prototype_iter = prototypes.find(objectType);
    if (prototype_iter != prototypes.end()) {
      boost::intrusive_ptr<Object> bla = CopyObject((*prototype_iter).second, postfix);
      bla->SetName(name);
      return bla;
    } else {
      Log(e_FatalError, "ObjectFactory", "CreateObject", "Object type " + int_to_str(objectType) + " not found in prototype registry");
      return 0;
    }
  }

  boost::intrusive_ptr<Object> ObjectFactory::CopyObject(boost::intrusive_ptr<Object> source, std::string postfix) {
    boost::intrusive_ptr<Object> bla;

    switch (source->GetObjectType()) {
      case e_ObjectType_Camera:
        bla = new Camera(*static_cast<Camera*>(source.get()));
        break;
      case e_ObjectType_Image2D:
        bla = new Image2D(*static_cast<Image2D*>(source.get()));
        break;
      case e_ObjectType_Geometry:
        bla = new Geometry(*static_cast<Geometry*>(source.get()), postfix);
        break;
      case e_ObjectType_Skybox:
        bla = new Skybox(*static_cast<Skybox*>(source.get()));
        break;
      case e_ObjectType_Light:
        bla = new Light(*static_cast<Light*>(source.get()));
        break;
      case e_ObjectType_Joint:
        bla = new Joint(*static_cast<Joint*>(source.get()));
        break;
      case e_ObjectType_AudioReceiver:
        bla = new AudioReceiver(*static_cast<AudioReceiver*>(source.get()));
        break;
      case e_ObjectType_Sound:
        bla = new Sound(*static_cast<Sound*>(source.get()));
        break;
      default:
        Log(e_FatalError, "ObjectFactory", "CopyObject", "Object type " + int_to_str(source->GetObjectType()) + " not found");
        break;
    }
    assert(bla != boost::intrusive_ptr<Object>());
    return bla;
  }

  void ObjectFactory::RegisterPrototype(e_ObjectType objectType, boost::intrusive_ptr<Object> prototype) {
    prototypes.insert(std::pair<e_ObjectType, boost::intrusive_ptr<Object> >(objectType, prototype));
  }

}
