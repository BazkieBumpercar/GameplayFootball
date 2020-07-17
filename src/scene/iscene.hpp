// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_INTERFACE_SCENE
#define _HPP_INTERFACE_SCENE

#include "defines.hpp"

#include "types/subject.hpp"

#include "systems/isystemscene.hpp"

namespace blunted {

  class Object;
  class ISceneInterpreter;
  class ISystemObject;

  enum e_SceneType {
    e_SceneType_Scene2D = 1,
    e_SceneType_Scene3D = 2
  };

  class IScene : public Subject<ISceneInterpreter> {

    public:
      virtual void Init() = 0;
      virtual void Exit() = 0; // ATOMIC

      virtual void CreateSystemObjects(boost::intrusive_ptr<Object> object) = 0;

      virtual const std::string GetName() const = 0;
      virtual e_SceneType GetSceneType() const = 0;

      virtual void PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) = 0;
      virtual bool SupportedObjectType(e_ObjectType objectType) const = 0;

    protected:

  };

  class ISceneInterpreter : public Interpreter {

    public:
      virtual void OnLoad() = 0;
      virtual void OnUnload() = 0;

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object) = 0;

    protected:

  };

}

#endif
