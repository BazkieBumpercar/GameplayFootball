// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_GRAPHICS_TASK
#define _HPP_SYSTEMS_GRAPHICS_TASK

#include "defines.hpp"

#include "systems/isystemtask.hpp"
#include "rendering/interface_renderer3d.hpp"
#include "rendering/r3d_messages.hpp"

#include "scene/objects/camera.hpp"

namespace blunted {

  class GraphicsSystem;

  class GraphicsTask : public ISystemTask {

    public:
      GraphicsTask(GraphicsSystem *system);
      virtual ~GraphicsTask();

      virtual void operator()();

      int GetAverageFrameTime_ms(unsigned int frameCount) const;
      unsigned long GetLastSwapTime_ms() const { return lastSwapTime_ms.GetData(); }
      int GetTimeSinceLastSwap_ms() const;

    protected:
      void GetPhase();
      void ProcessPhase();
      void PutPhase();

      bool quit;
      GraphicsSystem *graphicsSystem;

      int shadowSkipFrameCounter;

      boost::intrusive_ptr<Renderer3DMessage_SwapBuffers> swapBuffers;

      mutable Lockable<unsigned long> lastSwapTime_ms;
      mutable Lockable < std::list<int> > frameTimes_ms;

  };

  class GraphicsTaskCommand_Dummy : public Command {

    public:
      GraphicsTaskCommand_Dummy() : Command("Dummy") {}

    protected:
      virtual bool Execute(void *caller = NULL);

  };

  class GraphicsTaskCommand_RenderCamera : public Command {

    public:
      GraphicsTaskCommand_RenderCamera() : Command("RenderCamera") {}

    protected:
      virtual bool Execute(void *caller = NULL);

  };

  class GraphicsTaskCommand_EnqueueView : public Command {

    public:
      GraphicsTaskCommand_EnqueueView(boost::intrusive_ptr<Camera> camera, int shadowSkipFrameCounter) : Command("EnqueueView"), camera(camera), shadowSkipFrameCounter(shadowSkipFrameCounter) {};

    protected:
      virtual bool Execute(void *caller = NULL);
      virtual void EnqueueShadowMap(boost::intrusive_ptr<Light> light);

      boost::intrusive_ptr<Camera> camera;
      int shadowSkipFrameCounter;

  };

  class GraphicsTaskCommand_RenderImage2D : public Command {

    public:
      GraphicsTaskCommand_RenderImage2D() : Command("RenderImage2D") {}

    protected:
      virtual bool Execute(void *caller = NULL);

  };

  class GraphicsTaskCommand_RenderShadowMaps : public Command {

    public:
      GraphicsTaskCommand_RenderShadowMaps() : Command("RenderShadowMaps") {}

    protected:
      virtual bool Execute(void *caller = NULL);

  };

}

#endif
