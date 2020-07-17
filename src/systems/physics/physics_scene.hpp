// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_PHYSICS_SCENE
#define _HPP_SYSTEMS_PHYSICS_SCENE

#include "defines.hpp"

#include "systems/isystemscene.hpp"

#include "scene/scene3d/scene3d.hpp"

namespace blunted {

  class PhysicsSystem;
  class IPhysicsWrapper;

  class PhysicsScene : public ISystemScene {

    public:
      PhysicsScene(PhysicsSystem *physicsSystem);
      virtual ~PhysicsScene();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

      virtual boost::intrusive_ptr<ISceneInterpreter> GetInterpreter(e_SceneType sceneType);

      PhysicsSystem *GetPhysicsSystem();
      IPhysicsWrapper *GetPhysicsWrapper();

      int worldID;
      int spaceID;

    protected:
      PhysicsSystem *physicsSystem;
      IPhysicsWrapper *physicsWrapper;

  };


  class PhysicsScene_Scene3DInterpreter : public IScene3DInterpreter {

    public:
      PhysicsScene_Scene3DInterpreter(PhysicsScene *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Physics; }

      virtual void OnLoad();
      virtual void OnUnload();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

      virtual void SetGravity(const Vector3 &gravity);
      virtual void SetErrorCorrection(float value);
      virtual void SetConstraintForceMixing(float value);

    protected:
      PhysicsScene *caller;

  };

}

#endif
