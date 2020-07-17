// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "physics_task.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

#include "managers/taskmanager.hpp"
#include "managers/scenemanager.hpp"

#include "physics_system.hpp"

#include "framework/scheduler.hpp"

#include "blunted.hpp"

namespace blunted {

  PhysicsTask::PhysicsTask(PhysicsSystem *system) : ISystemTask(), physicsSystem(system) {
    remainder_ms = 0;
    timeStep_ms = 2;
    previousTime_ms = EnvironmentManager::GetInstance().GetTime_ms();
  }

  PhysicsTask::~PhysicsTask() {
    physicsSystem = NULL;
  }

  void PhysicsTask::operator()() {
    Log(e_Notice, "PhysicsTask", "operator()()", "Starting PhysicsSystemTask thread");

    SetState(e_ThreadState_Idle);

    quit = false;

    // the actual, per frame system task
    while (!quit) {
      boost::intrusive_ptr<Command> message(messageQueue.WaitForMessage());
      assert(message.get());
      SetState(e_ThreadState_Busy);
      if (!message->Handle(this)) quit = true;
      message.reset();
      SetState(e_ThreadState_Idle);

      //thread.yield();
    }

    Log(e_Notice, "PhysicsTask", "operator()()", "Shutting down PhysicsSystemTask thread");

    if (messageQueue.GetPending() > 0) Log(e_Error, "PhysicsTask", "operator()()", messageQueue.GetPending() + " messages left on quit!");
  }

  void PhysicsTask::GetPhase() {
  }

  void PhysicsTask::ProcessPhase() {
    TaskManager *taskManager = TaskManager::GetInstancePtr();
    IPhysicsWrapper *physics = physicsSystem->GetPhysicsWrapper();

    // run timestep loop
    //printf("%i %i %i\n", GetClocksPerFrame(), GetScheduler()->GetMasterClock_ms(), remainder_ms);
    unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();
    int timeDiff_ms = time_ms - previousTime_ms;
    previousTime_ms = time_ms;
    //printf("%i\n", timeDiff_ms);

    boost::intrusive_ptr<PhysicsTaskCommand_StepTime> stepTime(new PhysicsTaskCommand_StepTime(physicsSystem->GetPhysicsWrapper(), timeDiff_ms + remainder_ms, timeStep_ms));
    taskManager->EnqueueWork(stepTime, true);
    stepTime->Wait();
    remainder_ms = stepTime->remainder_ms;
  }

  void PhysicsTask::PutPhase() {
    TaskManager *taskManager = TaskManager::GetInstancePtr();

    // update positions/rotations etcetera
    boost::intrusive_ptr<PhysicsTaskCommand_UpdateGeometry> updateGeometry(new PhysicsTaskCommand_UpdateGeometry());
    taskManager->EnqueueWork(updateGeometry, true);
    updateGeometry->Wait();
  }


  bool PhysicsTaskCommand_StepTime::Execute(void *caller) {
    remainder_ms = physics->StepTime(timediff_ms, resolution_ms);

    return true;
  }

  bool PhysicsTaskCommand_UpdateGeometry::Execute(void *caller) {
    bool success;
    int i = 1;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene(i, success);
    while (success) {
      scene->PokeObjects(e_ObjectType_Geometry, e_SystemType_Physics);
      scene->PokeObjects(e_ObjectType_Joint, e_SystemType_Physics);
      i++;
      scene = SceneManager::GetInstance().GetScene(i, success);
    }

    return true;
  }

}
