// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifdef WIN32
#include <windows.h>
#endif

#include "main.hpp"

#include "animator.hpp"
#include "gui/gui.hpp"

#include "base/utils.hpp"
#include "base/properties.hpp"

#include "scene/scene2d/scene2d.hpp"
#include "scene/scene3d/scene3d.hpp"

#include "systems/graphics/graphics_system.hpp"
#include "systems/physics/physics_system.hpp"

#include "framework/scheduler.hpp"

#include "managers/systemmanager.hpp"
#include "managers/scenemanager.hpp"

#include "base/log.hpp"

//#include <SDL/SDL.h>

#if defined(WIN32) && defined(__MINGW32__)
#undef main
#endif

using namespace blunted;

GraphicsSystem *graphicsSystem;
//PhysicsSystem *physicsSystem;

boost::shared_ptr<Scene2D> scene2D;
boost::shared_ptr<Scene3D> scene3D;

boost::shared_ptr<TaskSequence> graphicsSequence;
boost::shared_ptr<TaskSequence> physicsSequence;

//int main(int argc, char* argv[]) {
int main(int argc, const char** argv) {
  Properties config;
  config.LoadFile("animator.config");

  Initialize(config);


  // initialize systems

  SystemManager *systemManager = SystemManager::GetInstancePtr();

  graphicsSystem = new GraphicsSystem();
  bool returnvalue = systemManager->RegisterSystem("GraphicsSystem", graphicsSystem);
  if (!returnvalue) Log(e_FatalError, "blunted", "Initialize", "Could not register GraphicsSystem");

//  physicsSystem = new PhysicsSystem();
//  returnvalue = systemManager->RegisterSystem("PhysicsSystem", physicsSystem);
//  if (!returnvalue) Log(e_FatalError, "blunted", "Initialize", "Could not register PhysicsSystem");

  // todo: let systemmanager init systems?
  graphicsSystem->Initialize(config);
//  physicsSystem->Initialize(config);


  // init scenes

  scene2D = boost::shared_ptr<Scene2D>(new Scene2D("scene2D", config));
  SceneManager::GetInstance().RegisterScene(scene2D);

  scene3D = boost::shared_ptr<Scene3D>(new Scene3D("scene3D"));
  SceneManager::GetInstance().RegisterScene(scene3D);


  // sequences

  boost::mutex graphicsMutex;

  graphicsSequence = boost::shared_ptr<TaskSequence>(new TaskSequence("graphics", config.GetInt("graphics3d_frametime_ms", 0)));
  graphicsSequence->AddLockEntry(graphicsMutex, e_LockAction_Lock);
  graphicsSequence->AddSystemTaskEntry(graphicsSystem, e_TaskPhase_Get);
  graphicsSequence->AddLockEntry(graphicsMutex, e_LockAction_Unlock);
  graphicsSequence->AddSystemTaskEntry(graphicsSystem, e_TaskPhase_Process);
  graphicsSequence->AddSystemTaskEntry(graphicsSystem, e_TaskPhase_Put);
  GetScheduler()->RegisterTaskSequence(graphicsSequence);


  // gui

  boost::shared_ptr<GuiTask> guiTask(new GuiTask(scene2D, 20 / 10.0, 10));
  GuiInterface *guiInterface = guiTask->GetInterface();
  boost::shared_ptr<Animator> animator(new Animator(scene2D, scene3D, guiInterface));
  physicsSequence = boost::shared_ptr<TaskSequence>(new TaskSequence("physics", config.GetInt("physics_frametime_ms", 10), false));
  //physicsSequence->AddSystemTaskEntry(physicsSystem, e_TaskPhase_Get);
  //physicsSequence->AddSystemTaskEntry(physicsSystem, e_TaskPhase_Process);
  physicsSequence->AddLockEntry(graphicsMutex, e_LockAction_Lock);
  //physicsSequence->AddSystemTaskEntry(physicsSystem, e_TaskPhase_Put);
  physicsSequence->AddUserTaskEntry(guiTask, e_TaskPhase_Get);
  physicsSequence->AddUserTaskEntry(guiTask, e_TaskPhase_Process);
  physicsSequence->AddUserTaskEntry(guiTask, e_TaskPhase_Put);
  physicsSequence->AddUserTaskEntry(animator, e_TaskPhase_Get);
  physicsSequence->AddUserTaskEntry(animator, e_TaskPhase_Process);
  physicsSequence->AddUserTaskEntry(animator, e_TaskPhase_Put);
  physicsSequence->AddLockEntry(graphicsMutex, e_LockAction_Unlock);
  GetScheduler()->RegisterTaskSequence(physicsSequence);


  // fire!

//  while (RunOnce()) {
//  }
  Run();


  // exit

  animator.reset();
  physicsSequence.reset();
  graphicsSequence.reset();
  guiTask.reset();

  scene2D.reset();
  scene3D.reset();

  Exit();

  return 0;
}
