// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_GRAPHICS_SYSTEM
#define _HPP_SYSTEMS_GRAPHICS_SYSTEM

#include "defines.hpp"

#include "systems/isystem.hpp"
#include "systems/isystemscene.hpp"
#include "systems/graphics/rendering/opengl_renderer3d.hpp"

#include "scene/iscene.hpp"

#include "graphics_task.hpp"

#include "resources/texture.hpp"
#include "resources/vertexbuffer.hpp"

#include "managers/resourcemanager.hpp"

namespace blunted {

  class Renderer3D;

  class GraphicsSystem : public ISystem {

    public:
      GraphicsSystem();
      virtual ~GraphicsSystem();

      virtual void Initialize(const Properties &config);
      virtual void Exit();

      e_SystemType GetSystemType() const;

      virtual ISystemScene *CreateSystemScene(boost::shared_ptr<IScene> scene);

      virtual ISystemTask *GetTask();
      virtual Renderer3D *GetRenderer3D();

      boost::shared_ptr < ResourceManager<Texture> > GetTextureResourceManager();
      boost::shared_ptr < ResourceManager<VertexBuffer> > GetVertexBufferResourceManager();

      MessageQueue<Overlay2DQueueEntry> &GetOverlay2DQueue();

      void GetContextSize(int &width, int &height, int &bpp) { width = this->width; height = this->height; bpp = this->bpp; }
      Vector3 GetContextSize() { return Vector3(width, height, bpp); }

      int GetAverageFrameTime_ms(unsigned int frameCount) const { assert(task); return task->GetAverageFrameTime_ms(frameCount); };
      unsigned long GetLastSwapTime_ms() const { assert(task); return task->GetLastSwapTime_ms(); }
      int GetTimeSinceLastSwap_ms() const { assert(task); return task->GetTimeSinceLastSwap_ms(); }

      virtual std::string GetName() const { return "graphics"; }

      boost::mutex getPhaseMutex;

    protected:
      const e_SystemType systemType;

      Renderer3D *renderer3DTask;

      GraphicsTask *task;

      boost::shared_ptr < ResourceManager<Texture> > textureResourceManager;
      boost::shared_ptr < ResourceManager<VertexBuffer> > vertexBufferResourceManager;

      MessageQueue<Overlay2DQueueEntry> overlay2DQueue;

      int width, height, bpp;

  };

}

#endif
