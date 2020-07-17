// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_PHYSICS_SYSTEM
#define _HPP_SYSTEMS_PHYSICS_SYSTEM

#include "defines.hpp"

#include "systems/isystem.hpp"
#include "systems/isystemscene.hpp"
#include "wrappers/ode_physics.hpp"

#include "scene/iscene.hpp"

#include "physics_task.hpp"

namespace blunted {

  class Renderer3D;

  class PhysicsSystem : public ISystem {

    public:
      PhysicsSystem();
      virtual ~PhysicsSystem();

      virtual void Initialize(const Properties &config);
      virtual void Exit();

      virtual e_SystemType GetSystemType() const;

      virtual ISystemScene *CreateSystemScene(boost::shared_ptr<IScene> scene);

      virtual ISystemTask *GetTask();
      virtual IPhysicsWrapper *GetPhysicsWrapper();

      virtual std::string GetName() const { return "physics"; }

    protected:
      const e_SystemType systemType;

      IPhysicsWrapper *physicsWrapper;
      PhysicsTask *task;

  };

}

#endif
