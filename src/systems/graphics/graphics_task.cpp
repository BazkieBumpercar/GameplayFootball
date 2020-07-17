// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "graphics_task.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

#include "managers/taskmanager.hpp"
#include "managers/scenemanager.hpp"
#include "managers/environmentmanager.hpp"

#include "graphics_system.hpp"

#include "scene/objects/geometry.hpp"
#include "scene/objects/skybox.hpp"
#include "scene/objects/light.hpp"

#include "scene/scene3d/scene3d.hpp"

namespace blunted {

  GraphicsTask::GraphicsTask(GraphicsSystem *system) : ISystemTask(), graphicsSystem(system) {
    shadowSkipFrameCounter = 0;
    swapBuffers = boost::intrusive_ptr<Renderer3DMessage_SwapBuffers>();
    lastSwapTime_ms.SetData(EnvironmentManager::GetInstance().GetTime_ms());
  }

  GraphicsTask::~GraphicsTask() {
    graphicsSystem = NULL;
    // if a previous swapbuffer is still busy, wait
    if (swapBuffers != boost::intrusive_ptr<Renderer3DMessage_SwapBuffers>()) {
      swapBuffers->Wait();
    }
    swapBuffers.reset();
    swapBuffers = boost::intrusive_ptr<Renderer3DMessage_SwapBuffers>();
  }

  void GraphicsTask::operator()() {
    Log(e_Notice, "GraphicsTask", "operator()()", "Starting GraphicsSystemTask thread");

    SetState(e_ThreadState_Idle);

    quit = false;

    // the actual, per frame system task
    while (!quit) {
      boost::intrusive_ptr<Command> message(messageQueue.WaitForMessage());
      //assert(message.get());
      //printf("gfx task got message!\n");
      SetState(e_ThreadState_Busy); // recently added
      if (!message->Handle(this)) quit = true;
      message.reset(); // recently added
      SetState(e_ThreadState_Idle); // recently added
    }

    Log(e_Notice, "GraphicsTask", "operator()()", "Shutting down GraphicsSystemTask thread");

    if (messageQueue.GetPending() > 0) Log(e_Error, "GraphicsTask", "operator()()", messageQueue.GetPending() + " messages left on quit!");
  }


  int GraphicsTask::GetAverageFrameTime_ms(unsigned int frameCount) const {
    frameTimes_ms.Lock();
    std::list<int>::const_reverse_iterator iter = frameTimes_ms->rbegin();
    unsigned int total = 0;
    int totalTime_ms = 0;
    while (iter != frameTimes_ms->rend()) {
      totalTime_ms += *iter;
      //printf("added %i\n", *iter);
      total++;
      if (total > frameCount) break;
      iter++;
    }
    frameTimes_ms.Unlock();
    if (total > 0) {
      //printf("total time %i, total count %i, result %i\n", totalTime_ms, total, totalTime_ms / total);
      return totalTime_ms / total;
    } else {
      return -1;
    }
  }

  int GraphicsTask::GetTimeSinceLastSwap_ms() const {
    return EnvironmentManager::GetInstance().GetTime_ms() - lastSwapTime_ms.GetData();
  }


  void GraphicsTask::GetPhase() {

    boost::mutex::scoped_lock getPhaseLock(graphicsSystem->getPhaseMutex);

    TaskManager *taskManager = TaskManager::GetInstancePtr();
    Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();


    // poke all image2D objects

    boost::intrusive_ptr<GraphicsTaskCommand_RenderImage2D> renderImage2D(new GraphicsTaskCommand_RenderImage2D());
    taskManager->EnqueueWork(renderImage2D, true);
    //taskManager->NotifyWorkers();


    // collect visibles

    bool success;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) {
      std::list < boost::intrusive_ptr<Camera> > cameras;
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Camera>(e_ObjectType_Camera, cameras);
      //printf("cameras.size: %i\n", (signed int)cameras.size());

      std::vector < boost::intrusive_ptr<GraphicsTaskCommand_EnqueueView> > enqueueView; // [cameras.size()]
      std::list < boost::intrusive_ptr<Camera> >::iterator cameraIter = cameras.begin();
      int i = 0;
      // enqueue all camera views
      while (cameraIter != cameras.end()) {

        if ((*cameraIter)->IsEnabled()) {
          enqueueView.push_back(boost::intrusive_ptr<GraphicsTaskCommand_EnqueueView>(new GraphicsTaskCommand_EnqueueView((*cameraIter), shadowSkipFrameCounter)));
          taskManager->EnqueueWork(enqueueView.back(), false);
        }

        cameraIter++;
      }

      taskManager->NotifyWorkers();

      for (unsigned int i = 0; i < enqueueView.size(); i++) {
        // wait on camera views
        enqueueView[i]->Wait();
      }

    }

    renderImage2D->Wait();
  }

  void GraphicsTask::ProcessPhase() {

    TaskManager *taskManager = TaskManager::GetInstancePtr();
    Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();

    // poke lights
    if (shadowSkipFrameCounter == 0) {
      boost::intrusive_ptr<GraphicsTaskCommand_RenderShadowMaps> renderShadowMaps(new GraphicsTaskCommand_RenderShadowMaps());
      taskManager->EnqueueWork(renderShadowMaps, true);
      //renderShadowMaps->Execute();

      renderShadowMaps->Wait();
    }

    // poke camera
    boost::intrusive_ptr<GraphicsTaskCommand_RenderCamera> renderCamera(new GraphicsTaskCommand_RenderCamera());
    taskManager->EnqueueWork(renderCamera, true);
    //renderCamera->Execute();

    renderCamera->Wait();

    // render the Overlay2D queue
    boost::intrusive_ptr<Renderer3DMessage_RenderOverlay2D> renderOverlay2D(new Renderer3DMessage_RenderOverlay2D(graphicsSystem->GetOverlay2DQueue()));
    renderer3D->messageQueue.PushMessage(renderOverlay2D, true);

  }

  void GraphicsTask::PutPhase() {
    TaskManager *taskManager = TaskManager::GetInstancePtr();
    Renderer3D *renderer3D = graphicsSystem->GetRenderer3D();

    // swap the buffers and stare in awe
    swapBuffers = new Renderer3DMessage_SwapBuffers();
    renderer3D->messageQueue.PushMessage(swapBuffers, true);


    // wait for previous frame to swap
    // opengl thread has only one message processor, so don't wait for swap (might take long to vsync). messages will be processed serially anyway
    // todo: maybe still should be on. have to check. seems that the lazy wait causes time problems: the next frame is drawn for situation time = x,
    // but since the gfx thread has to wait for the vsync, the situation drawn should be that of time = x + vsync_delay. this makes for choppy gfx.
    // update: with high enough fps, this might be ok
    // update: last frame time thing obviously doesn't work when only setting lastswaptime at next frame time, so waiting here for retrace again.
    //         kinda seems more fluent, though this may be psychological

    if (swapBuffers != boost::intrusive_ptr<Renderer3DMessage_SwapBuffers>()) {
      swapBuffers->Wait();
      unsigned long readyTime_ms = swapBuffers->GetReadyTime_ms();
      frameTimes_ms.Lock();
      //printf("readyTime_ms = %i, lastSwapTime_ms = %i\n", readyTime_ms, lastSwapTime_ms.GetData());
      frameTimes_ms->push_back(readyTime_ms - lastSwapTime_ms.GetData());
      if (frameTimes_ms->size() > 100) frameTimes_ms->pop_front();
      frameTimes_ms.Unlock();
      lastSwapTime_ms.SetData(readyTime_ms);
    }


    shadowSkipFrameCounter++;
    if (shadowSkipFrameCounter > 1) shadowSkipFrameCounter = 0;
  }


  bool GraphicsTaskCommand_Dummy::Execute(void *caller) {
    for (int i = 0; i < 100000; i++) {
      random(0, 1);
    }
    return true;
  }

  bool GraphicsTaskCommand_RenderCamera::Execute(void *caller) {
    bool success;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) scene->PokeObjects(e_ObjectType_Camera, e_SystemType_Graphics);

    return true;
  }

  bool GraphicsTaskCommand_EnqueueView::Execute(void *caller) {
    bool success;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);

    if (success) {
      // test camera->SetPosition(Vector3(sin((float)EnvironmentManager::GetInstance().GetTime_ms() * 0.001f) * 60, 0, 0), true);
      Vector3 cameraPos = camera->GetDerivedPosition();
      Quaternion cameraRot = camera->GetDerivedRotation();
      float nearCap, farCap;
      camera->GetCapping(nearCap, farCap);
      // todo
      float fov = camera->GetFOV() * 2.0f;
      float wideScreenMultiplier = 2.5f;
      vector_Planes bounding;
      Plane plane(cameraPos + cameraRot * Vector3(0, 0, -nearCap), cameraRot * Vector3(0, 0, -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(0, (3.6f * wideScreenMultiplier) / (fov / 24.0f), -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(0, (-3.6f * wideScreenMultiplier) / (fov / 24.0f), -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(2.4f / (fov / 24.0f), 0, -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos, cameraRot * Vector3(-2.4f / (fov / 24.0f), 0, -1).GetNormalized());
      bounding.push_back(plane);
      plane.Set(cameraPos + cameraRot * Vector3(0, 0, -farCap), cameraRot * Vector3(0, 0, 1).GetNormalized());
      bounding.push_back(plane);

      //(cameraRot * Vector3(0, 1, 0).GetNormalized()).Print();


      /* per-objecttype version

      std::list < boost::intrusive_ptr<Geometry> > visibleGeometry;
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Geometry>(e_ObjectType_Geometry, visibleGeometry, bounding);
      //printf("geometry.size: %i\n", visibleGeometry.size());

      std::list < boost::intrusive_ptr<Light> > visibleLights;
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Light>(e_ObjectType_Light, visibleLights, bounding);
      //printf("lights.size: %i\n", visibleLights.size());

      std::list < boost::intrusive_ptr<Skybox> > skyboxes;
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Skybox>(e_ObjectType_Skybox, skyboxes);
      //printf("skyboxes.size: %i\n", skyboxes.size());

      */


      // altogether function (may be slow in scenes with lots of objects)

      std::deque < boost::intrusive_ptr<Object> > visibleObjects;
      //std::list < boost::intrusive_ptr<Object> > visibleObjects;
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects(visibleObjects, bounding);

      std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry;
      std::deque < boost::intrusive_ptr<Light> > visibleLights;
      std::deque < boost::intrusive_ptr<Skybox> > skyboxes;

      std::deque < boost::intrusive_ptr<Object> >::iterator objectIter = visibleObjects.begin();
      while (objectIter != visibleObjects.end()) {
        if ((*objectIter)->IsEnabled()) {
          e_ObjectType objectType = (*objectIter)->GetObjectType();
          if      (objectType == e_ObjectType_Geometry) visibleGeometry.push_back(boost::static_pointer_cast<Geometry>(*objectIter));
          else if (objectType == e_ObjectType_Light)    visibleLights.push_back(boost::static_pointer_cast<Light>(*objectIter));
          else if (objectType == e_ObjectType_Skybox)   skyboxes.push_back(boost::static_pointer_cast<Skybox>(*objectIter));
        }
        objectIter++;
      }


      // prepare shadow maps

      if (shadowSkipFrameCounter == 0) {
        std::deque < boost::intrusive_ptr<Light> >::iterator lightIter = visibleLights.begin();
        while (lightIter != visibleLights.end()) {
          if ((*lightIter)->IsEnabled()) {
            if ((*lightIter)->GetShadow()) {
              EnqueueShadowMap(*lightIter);
            }
          }
          lightIter++;
        }
      }


      // enqueue camera view

      camera->EnqueueView(visibleGeometry, visibleLights, skyboxes);
    }

    return true;
  }

  void GraphicsTaskCommand_EnqueueView::EnqueueShadowMap(boost::intrusive_ptr<Light> light) {
    // todo: bounding box (this is a hax)
    bool success;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) {
      std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry;
      vector_Planes bounding;
      Plane plane(Vector3(0, -40, 0), Vector3(0, 1, 0.3).GetNormalized());
      bounding.push_back(plane);
      plane.Set(Vector3(0, 40, 0), Vector3(0, -1, 0.3).GetNormalized());
      bounding.push_back(plane);
      plane.Set(Vector3(-60, 0, 0), Vector3(1, 0, 0.3).GetNormalized());
      bounding.push_back(plane);
      plane.Set(Vector3(60, 0, 0), Vector3(-1, 0, 0.3).GetNormalized());
      bounding.push_back(plane);
      boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Geometry>(e_ObjectType_Geometry, visibleGeometry, bounding);
      //boost::static_pointer_cast<Scene3D>(scene)->GetObjects<Geometry>(e_ObjectType_Geometry, visibleGeometry);
      light->EnqueueShadowMap(camera, visibleGeometry);
    }
  }

  bool GraphicsTaskCommand_RenderImage2D::Execute(void *caller) {
    bool success;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene2D", success);
    if (success) {
      scene->PokeObjects(e_ObjectType_Image2D, e_SystemType_Graphics);
    }

    return true;
  }

  bool GraphicsTaskCommand_RenderShadowMaps::Execute(void *caller) {
    bool success;
    boost::shared_ptr<IScene> scene = SceneManager::GetInstance().GetScene("scene3D", success);
    if (success) {
      scene->PokeObjects(e_ObjectType_Light, e_SystemType_Graphics);
    }

    return true;
  }

}
