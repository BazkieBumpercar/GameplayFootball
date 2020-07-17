// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_PHYSICS_TASK
#define _HPP_SYSTEMS_PHYSICS_TASK

#include "defines.hpp"

#include "systems/isystemtask.hpp"
#include "wrappers/interface_physics.hpp"

namespace blunted {

  class PhysicsSystem;

  class PhysicsTask : public ISystemTask {

    public:
      PhysicsTask(PhysicsSystem *system);
      virtual ~PhysicsTask();

      virtual void operator()();

    protected:
      void GetPhase();
      void ProcessPhase();
      void PutPhase();

      bool quit;
      PhysicsSystem *physicsSystem;

      // remaining time to pass to next RenderFrame
      int remainder_ms;

      // resolution
      int timeStep_ms;

      unsigned long previousTime_ms;
  };

  class PhysicsTaskCommand_StepTime : public Command {

    public:
      PhysicsTaskCommand_StepTime(IPhysicsWrapper *physics, int timediff_ms, int resolution_ms) : Command("StepTime"), physics(physics), timediff_ms(timediff_ms), resolution_ms(resolution_ms) {};

      int remainder_ms;

    protected:
      virtual bool Execute(void *caller = NULL);

      IPhysicsWrapper *physics;
      int timediff_ms, resolution_ms;

  };

  class PhysicsTaskCommand_UpdateGeometry : public Command {

    public:
      PhysicsTaskCommand_UpdateGeometry() : Command("UpdateGeometry") {}

    protected:
      virtual bool Execute(void *caller = NULL);

  };

}

#endif
