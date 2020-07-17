// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_RENDERER3D_MESSAGES
#define _HPP_RENDERER3D_MESSAGES

#include "interface_renderer3d.hpp"

#include "managers/environmentmanager.hpp"

namespace blunted {

  // 'camera' view
  struct ViewBuffer {
    std::deque<VertexBufferQueueEntry> visibleGeometry;
    std::deque<LightQueueEntry> visibleLights;
    std::deque<VertexBufferQueueEntry> skyboxes;

    Matrix4 cameraMatrix;
    float cameraFOV;
    float cameraNearCap;
    float cameraFarCap;
  };


  // messages

  class Renderer3DMessage_CreateContext : public Command {

    public:
      Renderer3DMessage_CreateContext(int width, int height, int bpp, bool fullscreen) : Command("r3dmsg_CreateContext"), width(width), height(height), bpp(bpp), fullscreen(fullscreen) {};

      // return values
      bool success;

    protected:
      virtual bool Execute(void *caller = NULL) {
        success = static_cast<Renderer3D*>(caller)->CreateContext(width, height, bpp, fullscreen);
        return true;
      }

      // parameters
      int width, height, bpp;
      bool fullscreen;

  };

  class Renderer3DMessage_SwapBuffers : public Command {

    public:
      Renderer3DMessage_SwapBuffers() : Command("r3dmsg_SwapBuffers"), readyTime_ms(0) {};

      unsigned long GetReadyTime_ms() { return readyTime_ms; }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->SwapBuffers();
        readyTime_ms = EnvironmentManager::GetInstance().GetTime_ms();
        return true;
      }

      unsigned long readyTime_ms;

  };

  class Renderer3DMessage_CreateTexture : public Command {

    public:
      Renderer3DMessage_CreateTexture(e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, int width, int height, bool alpha = false, bool repeat = true, bool mipmaps = true, bool filter = true, bool compareDepth = false) : Command("r3dmsg_CreateTexture"), internalPixelFormat(internalPixelFormat), pixelFormat(pixelFormat), width(width), height(height), alpha(alpha), repeat(repeat), mipmaps(mipmaps), filter(filter), compareDepth(compareDepth) {};

      int textureID;

    protected:
      virtual bool Execute(void *caller = NULL) {
        textureID = static_cast<Renderer3D*>(caller)->CreateTexture(internalPixelFormat, pixelFormat, width, height, alpha, repeat, mipmaps, filter, false, compareDepth); // false == multisample

        return true;
      }

      e_InternalPixelFormat internalPixelFormat;
      e_PixelFormat pixelFormat;
      int width;
      int height;
      bool alpha;
      bool repeat;
      bool mipmaps;
      bool filter;
      bool compareDepth;

  };

  class Renderer3DMessage_DeleteTexture : public Command {

    public:
      Renderer3DMessage_DeleteTexture(int textureID) : Command("r3dmsg_DeleteTexture"), textureID(textureID) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->DeleteTexture(textureID);

        return true;
      }

      int textureID;

  };

  class Renderer3DMessage_ResizeTexture : public Command {

    public:
      Renderer3DMessage_ResizeTexture(int textureID, SDL_Surface *source, e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, bool alpha = false, bool mipmaps = true) : Command("r3dmsg_ResizeTexture"), textureID(textureID), internalPixelFormat(internalPixelFormat), pixelFormat(pixelFormat), alpha(alpha), mipmaps(mipmaps) {
        // copy image so caller doesn't have to wait for update to complete
        // DAMN YOU SDL! this function won't actually copy right surface, just make a shallow copy instead. that explains a crash i got. fuuufuuuuuu
        //this->source = SDL_CreateRGBSurfaceFrom(source->pixels, source->w, source->h, 0, source->pitch, 0, 0, 0, 0);
        // use this hax instead. really SDL, no deep surface copy function? pfff
        this->source = SDL_ConvertSurface(source, source->format, 0);
        //assert(source->pixels != this->source->pixels);
      }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->ResizeTexture(textureID, source, internalPixelFormat, pixelFormat, alpha, mipmaps);

        SDL_FreeSurface(source);

        return true;
      }

      int textureID;
      SDL_Surface *source;
      e_InternalPixelFormat internalPixelFormat;
      e_PixelFormat pixelFormat;
      bool alpha, mipmaps;

  };

  class Renderer3DMessage_UpdateTexture : public Command {

    public:
      Renderer3DMessage_UpdateTexture(int textureID, SDL_Surface *source, bool alpha = false, bool mipmaps = true) : Command("r3dmsg_UpdateTexture"), textureID(textureID), alpha(alpha), mipmaps(mipmaps) {
        // copy image so caller doesn't have to wait for update to complete
        // DAMN YOU SDL! this function won't actually copy right surface, just make a shallow copy instead. that explains a crash i got. fuuufuuuuuu
        //this->source = SDL_CreateRGBSurfaceFrom(source->pixels, source->w, source->h, 0, source->pitch, 0, 0, 0, 0);
        // use this hax instead. really SDL, no deep surface copy function? pfff
        this->source = SDL_ConvertSurface(source, source->format, 0);
        //assert(source->pixels != this->source->pixels);
      }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->UpdateTexture(textureID, source, alpha, mipmaps);

        SDL_FreeSurface(this->source);

        return true;
      }

      int textureID;
      SDL_Surface *source;
      bool alpha, mipmaps;

  };

  class Renderer3DMessage_CreateVertexBuffer : public Command {

    public:
      Renderer3DMessage_CreateVertexBuffer(float *vertices, unsigned int verticesDataSize, std::vector<unsigned int> indices, e_VertexBufferUsage usage) : Command("r3dmsg_CreateVertexBuffer"), vertices(vertices), verticesDataSize(verticesDataSize), indices(indices), usage(usage) {};

      VertexBufferID vertexBufferID;

    protected:
      virtual bool Execute(void *caller = NULL) {
        vertexBufferID = static_cast<Renderer3D*>(caller)->CreateVertexBuffer(vertices, verticesDataSize, indices, usage);

        return true;
      }

      float *vertices;
      unsigned int verticesDataSize;
      std::vector<unsigned int> indices;
      e_VertexBufferUsage usage;

  };

  class Renderer3DMessage_UpdateVertexBuffer : public Command {

    public:
      Renderer3DMessage_UpdateVertexBuffer(VertexBufferID vertexBufferID, float *vertices, unsigned int verticesDataSize) : Command("r3dmsg_UpdateVertexBuffer"), vertexBufferID(vertexBufferID), verticesDataSize(verticesDataSize) {
        // copying causes terrible last-level cache misses. so use pointer and make sure at client side that we won't touch them in the meantime
        //this->vertices = new float[verticesDataSize];
        //memcpy(this->vertices, vertices, verticesDataSize * sizeof(float));
        this->vertices = vertices;
      }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->UpdateVertexBuffer(vertexBufferID, vertices, verticesDataSize);

        return true;
      }

      VertexBufferID vertexBufferID;

      float *vertices;
      int verticesDataSize;

  };

  class Renderer3DMessage_DeleteVertexBuffer : public Command {

    public:
      Renderer3DMessage_DeleteVertexBuffer(VertexBufferID vertexBufferID) : Command("r3dmsg_DeleteVertexBuffer"), vertexBufferID(vertexBufferID) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->DeleteVertexBuffer(vertexBufferID);

        return true;
      }

      VertexBufferID vertexBufferID;

  };

  class Renderer3DMessage_RenderOverlay2D : public Command {

    public:
      Renderer3DMessage_RenderOverlay2D(MessageQueue<Overlay2DQueueEntry> &overlay2DQueue) : Command("r3dmsg_RenderOverlay2D") {
        bool isMessage = true;
        while (isMessage) {
          Overlay2DQueueEntry queueEntry = overlay2DQueue.GetMessage(isMessage);
          if (isMessage) this->overlay2DQueue.push_back(queueEntry);
        }
      };

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->RenderOverlay2D(overlay2DQueue);

        return true;
      }

      std::vector<Overlay2DQueueEntry> overlay2DQueue;

  };

  // todo: it's a bit lame that we have to wait for the OpenGL thread on the context size, even though we could cache this
  class Renderer3DMessage_GetContextSize : public Command {

    public:
      Renderer3DMessage_GetContextSize() : Command("r3dmsg_GetContextSize") {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->GetContextSize(width, height, bpp);

        return true;
      }

      int width;
      int height;
      int bpp;

  };

  class Renderer3DMessage_SetFOV : public Command {

    public:
      Renderer3DMessage_SetFOV(float angle) : Command("r3dmsg_SetFOV"), angle(angle) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->SetFOV(angle);

        return true;
      }

      float angle;

  };

  class Renderer3DMessage_CreateView : public Command {

    public:
      Renderer3DMessage_CreateView(float x_percent, float y_percent, float width_percent, float height_percent) : Command("r3dmsg_CreateView"), x_percent(x_percent), y_percent(y_percent), width_percent(width_percent), height_percent(height_percent) {};

      int viewID;

    protected:
      virtual bool Execute(void *caller = NULL) {
        viewID = static_cast<Renderer3D*>(caller)->CreateView(x_percent, y_percent, width_percent, height_percent);

        return true;
      }

      float x_percent;
      float y_percent;
      float width_percent;
      float height_percent;

  };

  class Renderer3DMessage_RenderView : public Command {

    public:
      // note 7/20/2010: made buffer param into a copy instead of a ref, so caller does not have to wait to clean up the mess
      // thought: could also leave cleaning to this class itself? but that would destroy acquisition == ownership
      // note 6/29/2011: too slow to copy, using a reference again. breaks acq = ownership though :|
      Renderer3DMessage_RenderView(int viewID, ViewBuffer &buffer) : Command("r3dmsg_RenderView"), viewID(viewID), buffer(buffer) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int viewID;
      ViewBuffer &buffer;

  };

  class Renderer3DMessage_DeleteView : public Command {

    public:
      Renderer3DMessage_DeleteView(int viewID) : Command("r3dmsg_DeleteView"), viewID(viewID) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->DeleteView(viewID);

        return true;
      }

      int viewID;

  };

  class Renderer3DMessage_RenderShadowMap : public Command {

    public:
      Renderer3DMessage_RenderShadowMap(const ShadowMap &map) : Command("r3dmsg_RenderShadowMap"), map(map) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      const ShadowMap &map;

  };

  class Renderer3DMessage_CreateFrameBuffer : public Command {

    public:
      Renderer3DMessage_CreateFrameBuffer(e_TargetAttachment target1 = e_TargetAttachment_None, int texID1 = 0,
                                          e_TargetAttachment target2 = e_TargetAttachment_None, int texID2 = 0,
                                          e_TargetAttachment target3 = e_TargetAttachment_None, int texID3 = 0,
                                          e_TargetAttachment target4 = e_TargetAttachment_None, int texID4 = 0,
                                          e_TargetAttachment target5 = e_TargetAttachment_None, int texID5 = 0) : Command("r3dmsg_CreateFrameBuffer"),
                                          target1(target1), texID1(texID1),
                                          target2(target2), texID2(texID2),
                                          target3(target3), texID3(texID3),
                                          target4(target4), texID4(texID4),
                                          target5(target5), texID5(texID5) {};

      int frameBufferID;

    protected:
      virtual bool Execute(void *caller = NULL);

      e_TargetAttachment target1;
      int texID1;
      e_TargetAttachment target2;
      int texID2;
      e_TargetAttachment target3;
      int texID3;
      e_TargetAttachment target4;
      int texID4;
      e_TargetAttachment target5;
      int texID5;
  };

  class Renderer3DMessage_DeleteFrameBuffer : public Command {

    public:
      Renderer3DMessage_DeleteFrameBuffer(int frameBufferID,
                                          e_TargetAttachment target1 = e_TargetAttachment_None,
                                          e_TargetAttachment target2 = e_TargetAttachment_None,
                                          e_TargetAttachment target3 = e_TargetAttachment_None,
                                          e_TargetAttachment target4 = e_TargetAttachment_None,
                                          e_TargetAttachment target5 = e_TargetAttachment_None) : Command("r3dmsg_DeleteFrameBuffer"),
                                          frameBufferID(frameBufferID),
                                          target1(target1),
                                          target2(target2),
                                          target3(target3),
                                          target4(target4),
                                          target5(target5) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int frameBufferID;
      e_TargetAttachment target1, target2, target3, target4, target5;

  };

}

#endif
