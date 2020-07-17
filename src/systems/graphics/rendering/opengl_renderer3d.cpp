// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "opengl_renderer3d.hpp"

#include <GL/glu.h>
#include <SDL/SDL.h>

#include "base/log.hpp"
#include "managers/environmentmanager.hpp"
#include "managers/usereventmanager.hpp"
#include "types/command.hpp"
#include "base/sdl_surface.hpp"
#include "base/utils.hpp"
#include "base/math/bluntmath.hpp"

#include "base/geometry/aabb.hpp"
#include "base/geometry/trianglemeshutils.hpp"

#include "../resources/texture.hpp"

#ifdef WIN32
#include <wingdi.h>
#endif

namespace blunted {

  OpenGLRenderer3D::OpenGLRenderer3D() : context(0), contextIsActive(true) {
    FOV = 45;
    overallBrightness = 128;

    _cache_activeTextureUnit = -1;

    currentShader = shaders.end();

    //SetPriorityClass(thread.native_handle(), HIGH_PRIORITY_CLASS);
  };

  OpenGLRenderer3D::~OpenGLRenderer3D() {
  };

  void OpenGLRenderer3D::SwapBuffers() {
    SDL_GL_SwapBuffers();
  }

  void OpenGLRenderer3D::LoadMatrix(const Matrix4 &mat) {
    glLoadMatrixf((float*)mat.GetTransposed().elements);
  }

  Matrix4 OpenGLRenderer3D::GetMatrix(e_MatrixMode matrixMode) const {
    float the_matrix[16];
    if (matrixMode == e_MatrixMode_Projection) glGetFloatv(GL_PROJECTION_MATRIX, the_matrix);
    if (matrixMode == e_MatrixMode_ModelView) glGetFloatv(GL_MODELVIEW_MATRIX, the_matrix);
    return Matrix4(the_matrix).GetTransposed();
  }

  void OpenGLRenderer3D::SetMatrix(const std::string &shaderUniformName, const Matrix4 &matrix) {
    SetUniformMatrix4(currentShader->first, shaderUniformName, matrix);
  }

  //void OpenGLRenderer3D::RenderOverlay2D(MessageQueue<Overlay2DQueueEntry> &overlay2DQueue) {
  void OpenGLRenderer3D::RenderOverlay2D(const std::vector<Overlay2DQueueEntry> &overlay2DQueue) {

    assert(context);

    // todo: use shaders maybe? (not sure if use of compat. context slows things down)

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, context->w, context->h, 0, 0.1, 10);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, 0, -1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    SetDepthMask(false);
    glShadeModel(GL_FLAT);
  	glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);

    for (unsigned int i = 0; i < overlay2DQueue.size(); i++) {
      const Overlay2DQueueEntry &queueEntry = overlay2DQueue.at(i);

      /* fun!
      float xf = (context->w * 1.0) / 2.0 - 128  + sin(i       + i2 * 1.2) * 200  + sin(i * 0.4 + i2 * 0.6) * 100;
      float yf = (context->h * 1.0) / 2.0 - 32   + cos(i * 0.7 + i2 * 0.7) * 160  + cos(i * 0.6 + i2 * 1.3) * 80;
      i2 += 0.15 - (i2 * 0.01);
      */

      glBindTexture(GL_TEXTURE_2D, queueEntry.texture->GetResource()->GetID());

      glBegin(GL_QUADS);

      glTexCoord3f(0, 0, 0);
      glVertex2f(queueEntry.position[0]                     , queueEntry.position[1]);
      glTexCoord3f(1, 0, 0);
      glVertex2f(queueEntry.position[0] + queueEntry.size[0], queueEntry.position[1]);
      glTexCoord3f(1, 1, 0);
      glVertex2f(queueEntry.position[0] + queueEntry.size[0], queueEntry.position[1] + queueEntry.size[1]);
      glTexCoord3f(0, 1, 0);
      glVertex2f(queueEntry.position[0]                     , queueEntry.position[1] + queueEntry.size[1]);

      glEnd();
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    SetDepthMask(true);

    glDisable(GL_BLEND);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
  }

  void OpenGLRenderer3D::RenderOverlay2D() {
    assert(context);

    Matrix4 orthoMatrix = CreateOrthoMatrix(-1, 1, -1, 1, 0.0f, 1.0f);
    SetMatrix("orthoProjectionMatrix", orthoMatrix);
    Matrix4 viewMatrix(MATRIX4_IDENTITY);
    viewMatrix.SetTranslation(Vector3(0, 0, -0.5f));
    SetMatrix("orthoViewMatrix", viewMatrix);


    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);

    SetCullingMode(e_CullingMode_Off);

    glEnable(GL_TEXTURE_2D);
    SetTextureUnit(4); // noise
    BindTexture(noiseTexID);
    SetTextureUnit(0);

    glBegin(GL_QUADS);

    glTexCoord3f(0, 1, 0);
    glVertex2f(-1, 1);
    glTexCoord3f(1, 1, 0);
    glVertex2f(1, 1);
    glTexCoord3f(1, 0, 0);
    glVertex2f(1, -1);
    glTexCoord3f(0, 0, 0);
    glVertex2f(-1, -1);

    glEnd();

    // unbind noise
    SetTextureUnit(4);
    BindTexture(0);

    SetCullingMode(e_CullingMode_Back);
  }

  void drawSphere(float r, int lats, int longs) {
    int i, j;
    for (i = 0; i <= lats; i++) {
      float lat0 = pi * (-0.5 + (float) (i - 1) / lats);
      float z0  = sin(lat0);
      float zr0 =  cos(lat0);

      float lat1 = pi * (-0.5 + (float) i / lats);
      float z1 = sin(lat1);
      float zr1 = cos(lat1);

      glBegin(GL_QUAD_STRIP);
      for (j = 0; j <= longs; j++) {
          float lng = 2 * pi * (float) (j - 1) / longs;
          float x = cos(lng);
          float y = sin(lng);

          glNormal3f(x * zr0, y * zr0, z0);
          glVertex3f(x * zr0 * r, y * zr0 * r, z0 * r);
          glNormal3f(x * zr1, y * zr1, z1);
          glVertex3f(x * zr1 * r, y * zr1 * r, z1 * r);
      }
      glEnd();
    }
  }

  void OpenGLRenderer3D::RenderLights(std::deque<LightQueueEntry> &lightQueue, const Matrix4 &projectionMatrix, const Matrix4 &viewMatrix) {
    Vector3 cameraPos = viewMatrix.GetInverse().GetTranslation();

  	glDisable(GL_ALPHA_TEST);

    SetUniformFloat3(currentShader->first, "cameraPosition", cameraPos.coords[0], cameraPos.coords[1], cameraPos.coords[2]);

    std::deque<LightQueueEntry>::iterator lightIter = lightQueue.begin();

    while (lightIter != lightQueue.end()) {
      const LightQueueEntry &light = (*lightIter);

      // todo: add light types SetUniformInt("lighting", "lightType", (int)light.type);

      // bind shadow map
      SetUniformInt(currentShader->first, "has_shadow", (int)light.hasShadow);
      if (light.hasShadow) {
        SetTextureUnit(7);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, light.shadowMapTexture->GetResource()->GetID());

        Matrix4 toTexCoords(MATRIX4_IDENTITY);
        toTexCoords.SetTranslation(Vector3(0.5f, 0.5f, 0.5f));
        toTexCoords.SetScale(Vector3(0.5f, 0.5f, 0.5f));
        SetUniformMatrix4(currentShader->first, "lightViewProjectionMatrix", toTexCoords * (*lightIter).lightProjectionMatrix * (*lightIter).lightViewMatrix);

      } else {
        glDisable(GL_TEXTURE_2D);
      }

      SetUniformFloat3(currentShader->first, "lightColor", light.color.coords[0], light.color.coords[1], light.color.coords[2]);
      SetUniformFloat(currentShader->first, "lightRadius", light.radius);
      SetUniformFloat3(currentShader->first, "lightPosition", light.position.coords[0], light.position.coords[1], light.position.coords[2]);

      int quad_or_sphere = 1;
      if (light.type == 0) {

        quad_or_sphere = 0; // directional light: visible anyway, everywhere. draw fullscreen quad

      } else if (light.type == 1) { // sphere
        AABB aabb = light.aabb;
        // get the spherical bounding box of the aabb
        // if the camera is inside this sphere, draw a fullscreen quad instead of a sphere
        // else, the sphere wouldn't be rendered..
        //float boundingSphereRadius = aabb.GetRadius();//light.position.GetDistance(Vector3(aabb.minxyz.coords[0], aabb.minxyz.coords[1], aabb.minxyz.coords[2]));
        if (cameraPos.GetDistance(light.position) <= aabb.GetRadius()) {
          // cam is within light radius, draw all
          quad_or_sphere = 0;
        } else {
          quad_or_sphere = 1;
        }
      }

      if (quad_or_sphere == 0) {

        // QUAD

        SetCullingMode(e_CullingMode_Off);
        SetDepthFunction(e_DepthFunction_Always);

        Matrix4 orthoMatrix = CreateOrthoMatrix(-1, 1, -1, 1, 0.0f, 1.0f);
        SetMatrix("projectionMatrix", orthoMatrix);
        Matrix4 xviewMatrix(MATRIX4_IDENTITY);
        xviewMatrix.SetTranslation(Vector3(0, 0, -0.5f));
        SetMatrix("viewMatrix", xviewMatrix);
        Matrix4 modelMatrix(MATRIX4_IDENTITY);
        SetMatrix("modelMatrix", modelMatrix);

        glBegin(GL_QUADS);

        glTexCoord3f(0, 1, 0);
        glVertex2f(-1, 1);
        glTexCoord3f(1, 1, 0);
        glVertex2f(1, 1);
        glTexCoord3f(1, 0, 0);
        glVertex2f(1, -1);
        glTexCoord3f(0, 0, 0);
        glVertex2f(-1, -1);

        glEnd();

      } else {

        // SPHERE

        SetCullingMode(e_CullingMode_Back);
        SetDepthFunction(e_DepthFunction_Less);

        AABB aabb = light.aabb;

        SetMatrix("projectionMatrix", projectionMatrix);
        SetMatrix("viewMatrix", viewMatrix);
        Matrix4 modelMatrix(MATRIX4_IDENTITY);
        modelMatrix.SetTranslation(light.position);
        SetMatrix("modelMatrix", modelMatrix);
        drawSphere(aabb.GetRadius() * 0.52f, 6, 6);
      }

      if (light.hasShadow) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        SetTextureUnit(0);
      }

      lightIter++;
    }

  }


  // --- new & improved


  // init & exit

  bool OpenGLRenderer3D::CreateContext(int width, int height, int bpp, bool fullscreen) {
    this->context_width = width;
    this->context_height = height;
    this->context_bpp = bpp;

    // default values
    this->cameraNear = 30.0;
    this->cameraFar = 270.0;

// #ifdef __linux__
//     #include <X11/Xlib.h>
//     XInitThreads();
// #endif

//#ifdef WIN32
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // todo: remember to enable this later on, after migrating to sdl 2 (though it is on by default with most drivers, or so it seems)
    //SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
//recently disabled  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); // wait for vsync?
//  int SDLerror = SDL_GL_SetSwapInterval(-1);
//  if (SDLerror == -1) SDL_GL_SetSwapInterval(1);

//#endif

    context = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL/* | SDL_RESIZABLE*/ | (fullscreen ? SDL_FULLSCREEN : 0));
    if (!context) {
      std::string errorString = SDL_GetError();
      Log(e_FatalError, "OpenGLRenderer3D", "CreateContext", "Failed on SDL error: " + errorString);
      return false;
    }

    std::string glVersionString = (char*)glGetString(GL_VERSION);
    Log(e_Notice, "OpenGLRenderer3D", "CreateContext", "Using OpenGL version " + glVersionString);

    int glVersion[2] = { 0, 0 };
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    Log(e_Notice, "OpenGLRenderer3D", "CreateContext", "OpenGL major/minor " + int_to_str(glVersion[0]) + "." + int_to_str(glVersion[1]));

    bool higherThan32 = false;
    if (glVersion[0] < 4) {
      if (glVersion[0] == 3 && glVersion[1] >= 2) higherThan32 = true;
    } else higherThan32 = true;

    if (!higherThan32) Log(e_Warning, "OpenGLRenderer3D", "CreateContext", "OpenGL version not equal to or higher than 3.2 (or not reported as such)");

    SDL_WM_SetCaption("Gameplay Football", NULL);

#ifdef WIN32
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWMInfo(&wmInfo)) {

      HWND hWnd = wmInfo.window;
      // center window
      // todo: needs linux version for centering as well. not sure how. find out.
      if (!fullscreen) {
        //#undef _WIN32_WINNT
        //#define _WIN32_WINNT 0x0500
        //#undef WINVER
        //#define WINVER 0x0500
        //#include <windows.h>
        //#include <winuser.h>
        HMONITOR mon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monInfo;
        monInfo.cbSize = sizeof(MONITORINFO);
        bool getMonInfo = GetMonitorInfo(mon, &monInfo);
        if (getMonInfo != 0) {
          RECT rect = monInfo.rcWork;
          int xSize = rect.right - rect.left;
          int xCenter = rect.left + (xSize * 0.5f);
          int ySize = rect.bottom - rect.top;
          int yCenter = rect.top + (ySize * 0.5f);
          int xWin = xCenter - (width * 0.5f);
          int yWin = yCenter - (height * 0.5f);
          xWin = std::max(xWin, 0); // super annoying to have a window's controls being unreachable
          yWin = std::max(yWin, 0);
          xWin -= 3; // window border size (educated guess - is a user setting, after all)
          yWin -= 12; // window border size + taskbar (educated guess - is a user setting, after all)
          SetWindowPos(hWnd, NULL, xWin, yWin, 0, 0, SWP_NOSIZE);
        }
        //SetWindowPos(hWnd, NULL, 976, 0, 0, 0, SWP_NOSIZE);
      }

      //HDC hDC = GetDC(wmInfo.window);
      //wglMakeCurrent(hDC, wmInfo.hglrc);
    }
#endif

#ifdef WIN32
    bool success = false;//wglSwapIntervalEXT(-1);
    if (!success) wglSwapIntervalEXT(1);
    //if (!success) printf("ANTI TEAR NOT SUPPORTED\n\n\n\n\n");
#endif

#ifdef __linux__
    GLenum err = glewInit();
    if (GLEW_OK != err) {
      std::string errorString =  (char*) glewGetErrorString(err);
      Log(e_FatalError, "OpenGLRenderer3D", "CreateContext", errorString);
    }

    bool success = false;//glXSwapIntervalSGI(-1); // anti-tear blah
    if (!success) glXSwapIntervalSGI(1);
    //if (!success) printf("ANTI TEAR NOT SUPPORTED\n\n\n\n\n");
#endif

    //SDL_ShowCursor(SDL_DISABLE); // todo: make customizable
    //SDL_WarpMouse(1280 * 3, 1024);
    largest_supported_anisotropy = 2;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
    //largest_supported_anisotropy = clamp(largest_supported_anisotropy, 0, 8); // don't overdo it

    glDisable(GL_LIGHTING);

    GLfloat color[4] = { 0, 0, 0, 0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, color);
    glMateriali(GL_FRONT, GL_SHININESS, 80);

    // enable color tracking
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glDisable(GL_MULTISAMPLE);

    glCullFace(GL_BACK);


    // shaders

    LoadShader("simple", "media/shaders/simple");
    LoadShader("lighting", "media/shaders/lighting");
    LoadShader("ambient", "media/shaders/ambient");
    LoadShader("zphase", "media/shaders/zphase");
    LoadShader("postprocess", "media/shaders/postprocess");

    currentShader = shaders.begin();


    SDL_Surface *noise = IMG_Load("media/shaders/noise.png");
    noiseTexID = CreateTexture(e_InternalPixelFormat_RGB8, e_PixelFormat_RGB, noise->w, noise->h, false, true, false, false, false);
    UpdateTexture(noiseTexID, noise, false, false);
    SDL_FreeSurface(noise);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glDisable(GL_MULTISAMPLE);

    return true;
  }

  void OpenGLRenderer3D::Exit() {
    DeleteTexture(noiseTexID);

    std::map<std::string, Shader>::iterator shaderIter = shaders.begin();
    while (shaderIter != shaders.end()) {
      glDeleteShader((*shaderIter).second.vertexShaderID);
      glDeleteShader((*shaderIter).second.fragmentShaderID);
      glDeleteProgram((*shaderIter).second.programID);
      shaderIter++;
    }

    currentShader = shaders.end();

    // assert(views.size() == 0);
    // todo: make views erase-able, as for now a views vector index is used when requesting views, which prohibits erasion
    // (actually, erasion 'works', but it does leave 'empty' records in the views vector, which is ugly and memory-leaky)
  }

  int OpenGLRenderer3D::CreateView(float x_percent, float y_percent, float width_percent, float height_percent) {

    Log(e_Notice, "OpenGLRenderer3D", "CreateView", "Creating new view, id " + int_to_str(views.size()));

    View view;

    view.target = e_ViewRenderTarget_Context;
    view.targetTexID = 0;
    view.x = int(floor(x_percent * 0.01 * context_width));
    view.y = int(floor(y_percent * 0.01 * context_height));
    view.width = int(floor(width_percent * 0.01 * context_width));
    view.height = int(floor(height_percent * 0.01 * context_height));


    // init FBO

    int gBufWidth = view.width;
    int gBufHeight = view.height;

    view.gBufferID = CreateFrameBuffer();
    BindFrameBuffer(view.gBufferID);

    // texture buffers
    view.gBuffer_DepthTexID = CreateTexture(e_InternalPixelFormat_DepthComponent16, e_PixelFormat_DepthComponent, gBufWidth, gBufHeight, false, false, false, false, false);
    SetFrameBufferTexture2D(e_TargetAttachment_Depth, view.gBuffer_DepthTexID);

    //view.gBuffer_AlbedoTexID = CreateTexture(e_InternalPixelFormat_RGBA8, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    view.gBuffer_AlbedoTexID = CreateTexture(e_InternalPixelFormat_RGBA16F, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    //view.gBuffer_AlbedoTexID = CreateTexture(e_InternalPixelFormat_RGBA32F, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    SetFrameBufferTexture2D(e_TargetAttachment_Color0, view.gBuffer_AlbedoTexID);

    //view.gBuffer_NormalTexID = CreateTexture(e_InternalPixelFormat_RGBA8, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    view.gBuffer_NormalTexID = CreateTexture(e_InternalPixelFormat_RGBA16F, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    //view.gBuffer_NormalTexID = CreateTexture(e_InternalPixelFormat_RGBA32F, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false);
    SetFrameBufferTexture2D(e_TargetAttachment_Color1, view.gBuffer_NormalTexID);

    //view.gBuffer_AuxTexID = CreateTexture(e_InternalPixelFormat_RGBA8, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    view.gBuffer_AuxTexID = CreateTexture(e_InternalPixelFormat_RGBA16F, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    //view.gBuffer_AuxTexID = CreateTexture(e_InternalPixelFormat_RGBA32F, e_PixelFormat_RGBA, gBufWidth, gBufHeight, false, false, false, false, false);
    SetFrameBufferTexture2D(e_TargetAttachment_Color2, view.gBuffer_AuxTexID);

    std::vector<e_TargetAttachment> targets;
    targets.push_back(e_TargetAttachment_Color0);
    targets.push_back(e_TargetAttachment_Color1);
    targets.push_back(e_TargetAttachment_Color2);
    SetRenderTargets(targets);
    targets.clear();

    if (!CheckFrameBufferStatus()) Log(e_FatalError, "OpenGLRenderer3D", "CreateView", "Could not create G-buffer");

    BindFrameBuffer(0);

    targets.push_back(e_TargetAttachment_Back);
    SetRenderTargets(targets);
    targets.clear();


    // accumulation buffer

    int accumBufWidth = view.width;
    int accumBufHeight = view.height;

    view.accumBufferID = CreateFrameBuffer();
    BindFrameBuffer(view.accumBufferID);

    //view.accumBuffer_DepthTexID = CreateTexture(e_InternalPixelFormat_DepthComponent32F, e_PixelFormat_DepthComponent, accumBufWidth, accumBufHeight, false, false, false, false);
    //SetFrameBufferTexture2D(e_TargetAttachment_Depth, view.accumBuffer_DepthTexID);

    //view.accumBuffer_AccumTexID = CreateTexture(e_InternalPixelFormat_RGB8, e_PixelFormat_RGB, accumBufWidth, accumBufHeight, false, false, false, false, false);
    view.accumBuffer_AccumTexID = CreateTexture(e_InternalPixelFormat_RGBA16F, e_PixelFormat_RGBA, accumBufWidth, accumBufHeight, false, false, false, false, false);
    //view.accumBuffer_AccumTexID = CreateTexture(e_InternalPixelFormat_RGBA32F, e_PixelFormat_RGBA, accumBufWidth, accumBufHeight, false, false, false, false, false);
    SetFrameBufferTexture2D(e_TargetAttachment_Color0, view.accumBuffer_AccumTexID);

    // Geforce GT 230 doesn't seem to be too happy about e_InternalPixelFormat_RGBA16 for some reason. Very slow framerates ensue
    //view.accumBuffer_ModifierTexID = CreateTexture(e_InternalPixelFormat_RGBA8, e_PixelFormat_RGB, accumBufWidth, accumBufHeight, false, false, false, false, false);
    view.accumBuffer_ModifierTexID = CreateTexture(e_InternalPixelFormat_RGBA16F, e_PixelFormat_RGBA, accumBufWidth, accumBufHeight, false, false, false, false, false);
    //view.accumBuffer_ModifierTexID = CreateTexture(e_InternalPixelFormat_RGBA32F, e_PixelFormat_RGBA, accumBufWidth, accumBufHeight, false, false, false, false, false);
    SetFrameBufferTexture2D(e_TargetAttachment_Color1, view.accumBuffer_ModifierTexID);

    targets.push_back(e_TargetAttachment_Color0);
    targets.push_back(e_TargetAttachment_Color1);
    SetRenderTargets(targets);
    targets.clear();

    if (!CheckFrameBufferStatus()) Log(e_FatalError, "OpenGLRenderer3D", "CreateView", "Could not create Accumulation buffer");

    BindFrameBuffer(0);

    targets.push_back(e_TargetAttachment_Back);
    SetRenderTargets(targets);
    targets.clear();

    views.push_back(view);

    return views.size() - 1;
  }

  View &OpenGLRenderer3D::GetView(int viewID) {
    return views.at(viewID);
  }

  void OpenGLRenderer3D::DeleteView(int viewID) {

    Log(e_Notice, "OpenGLRenderer3D", "DeleteView", "Deleting view, id " + int_to_str(viewID));

    View *view = &views.at(viewID);

    BindFrameBuffer(view->gBufferID);
    SetFrameBufferTexture2D(e_TargetAttachment_Depth, 0);
    SetFrameBufferTexture2D(e_TargetAttachment_Color0, 0);
    SetFrameBufferTexture2D(e_TargetAttachment_Color1, 0);
    SetFrameBufferTexture2D(e_TargetAttachment_Color2, 0);
    DeleteFrameBuffer(view->gBufferID);

    BindFrameBuffer(view->accumBufferID);
    SetFrameBufferTexture2D(e_TargetAttachment_Color0, 0);
    SetFrameBufferTexture2D(e_TargetAttachment_Color1, 0);
    DeleteFrameBuffer(view->accumBufferID);

    DeleteTexture(view->gBuffer_AlbedoTexID);
    DeleteTexture(view->gBuffer_NormalTexID);
    DeleteTexture(view->gBuffer_DepthTexID);
    DeleteTexture(view->gBuffer_AuxTexID);

    DeleteTexture(view->accumBuffer_AccumTexID);
    DeleteTexture(view->accumBuffer_ModifierTexID);
    //printf("done deleting view %i\n", viewID);
  }

  // general

  void OpenGLRenderer3D::SetCullingMode(e_CullingMode cullingMode) {

    switch (cullingMode) {

      case e_CullingMode_Off:     glDisable(GL_CULL_FACE);
                                  break;

      case e_CullingMode_Front:   glEnable(GL_CULL_FACE);
                                  glCullFace(GL_FRONT);
                                  break;

      case e_CullingMode_Back:    glEnable(GL_CULL_FACE);
                                  glCullFace(GL_BACK);
                                  break;
    }
  }

  void OpenGLRenderer3D::SetBlendingMode(e_BlendingMode blendingMode) {
    switch (blendingMode) {

      case e_BlendingMode_Off:    glDisable(GL_BLEND);
                                  break;

      case e_BlendingMode_On:     glEnable(GL_BLEND);
                                  break;
    }
  }

  GLenum GetGLBlendingFunction(e_BlendingFunction blendingFunction) {

    GLenum GLfunc = GL_ONE;

    switch (blendingFunction) {

      case e_BlendingFunction_Zero : GLfunc = GL_ZERO;
                                     break;

      case e_BlendingFunction_One :  GLfunc = GL_ONE;
                                     break;
    }

    return GLfunc;
  }

  void OpenGLRenderer3D::SetBlendingFunction(e_BlendingFunction blendingFunction1, e_BlendingFunction blendingFunction2) {

    GLenum func1 = GetGLBlendingFunction(blendingFunction1);
    GLenum func2 = GetGLBlendingFunction(blendingFunction2);

    glBlendFunc(func1, func2);
  }

  void OpenGLRenderer3D::SetDepthFunction(e_DepthFunction depthFunction) {
    switch (depthFunction) {

      case e_DepthFunction_Never:           glDepthFunc(GL_NEVER);
                                            break;

      case e_DepthFunction_Equal:           glDepthFunc(GL_EQUAL);
                                            break;

      case e_DepthFunction_Greater:         glDepthFunc(GL_GREATER);
                                            break;

      case e_DepthFunction_Less:            glDepthFunc(GL_LESS);
                                            break;

      case e_DepthFunction_GreaterOrEqual:  glDepthFunc(GL_GEQUAL);
                                            break;

      case e_DepthFunction_LessOrEqual:     glDepthFunc(GL_LEQUAL);
                                            break;

      case e_DepthFunction_NotEqual:        glDepthFunc(GL_NOTEQUAL);
                                            break;

      case e_DepthFunction_Always:          glDepthFunc(GL_ALWAYS);
                                            break;
    }
  }

  void OpenGLRenderer3D::SetDepthTesting(bool OnOff) {
    if (OnOff) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }

  void OpenGLRenderer3D::SetDepthMask(bool OnOff) {
    if (OnOff) {
      glDepthMask(GL_TRUE);
    } else {
      glDepthMask(GL_FALSE);
    }
  }

  void OpenGLRenderer3D::SetTextureMode(e_TextureMode textureMode) {
    // todo: err its not a mode i guess
    switch (textureMode) {

      case e_TextureMode_Off:     glDisable(GL_TEXTURE_2D);
                                  break;

      case e_TextureMode_2D:      glEnable(GL_TEXTURE_2D);
                                  break;
    }
  }

  void OpenGLRenderer3D::SetColor(const Vector3 &color, float alpha) {
    glColor4f(color.coords[0], color.coords[1], color.coords[2], alpha);
  }

  void OpenGLRenderer3D::SetColorMask(bool r, bool g, bool b, bool alpha) {
    glColorMask(r, g, b, alpha);
  }

  void OpenGLRenderer3D::ClearBuffer(const Vector3 &color, bool clearDepth, bool clearColor) {
//xx    glClearColor(color.coords[0], color.coords[1], color.coords[2], 0.0);
    //REDUNDANT? deprecated?
//xx    glClearDepth(1.0f);
    //glClear((GL_COLOR_BUFFER_BIT && clearColor) | (GL_DEPTH_BUFFER_BIT && clearDepth));
    GLbitfield mask = 0;
    if (clearDepth) mask |= GL_DEPTH_BUFFER_BIT;
    if (clearColor) mask |= GL_COLOR_BUFFER_BIT;
    glClear(mask);
  }

  Matrix4 OpenGLRenderer3D::CreatePerspectiveMatrix(float aspectRatio, float nearCap, float farCap) {
    //XX deprecated glMatrixMode(GL_PROJECTION);
    //XX deprecated glLoadIdentity();
    float Xnear = nearCap;
    if (Xnear == -1) Xnear = cameraNear;
    float Xfar = farCap;
    if (Xfar == -1) Xfar = cameraFar;
    //XX deprecated gluPerspective(FOV, aspectRatio, Xnear, Xfar);

    Matrix4 projectionMatrix;
    projectionMatrix.ConstructProjection(FOV, aspectRatio, Xnear, Xfar);

    return projectionMatrix;
  }

  Matrix4 OpenGLRenderer3D::CreateOrthoMatrix(float left, float right, float bottom, float top, float nearCap, float farCap) {
    //XX deprecated glMatrixMode(GL_PROJECTION);
    //XX deprecated glLoadIdentity();
    float Xnear = nearCap;
    if (Xnear == -1) Xnear = cameraNear;
    float Xfar = farCap;
    if (Xfar == -1) Xfar = cameraFar;
    //XX deprecated glOrtho(left, right, bottom, top, Xnear, Xfar);

    Matrix4 orthoMatrix;
    orthoMatrix.ConstructOrtho(left, right, bottom, top, Xnear, Xfar);

    return orthoMatrix;
  }


  // vertex buffers

  GLenum GetGLVertexBufferUsage(e_VertexBufferUsage usage) {

    GLenum GLfunc = GL_STREAM_DRAW;

    switch (usage) {

      case e_VertexBufferUsage_StreamDraw: GLfunc = GL_STREAM_DRAW;
                                           break;

      case e_VertexBufferUsage_StreamRead: GLfunc = GL_STREAM_READ;
                                           break;

      case e_VertexBufferUsage_StreamCopy: GLfunc = GL_STREAM_COPY;
                                           break;

      case e_VertexBufferUsage_StaticDraw: GLfunc = GL_STATIC_DRAW;
                                           break;

      case e_VertexBufferUsage_StaticRead: GLfunc = GL_STATIC_READ;
                                           break;

      case e_VertexBufferUsage_StaticCopy: GLfunc = GL_STATIC_COPY;
                                           break;

      case e_VertexBufferUsage_DynamicDraw: GLfunc = GL_DYNAMIC_DRAW;
                                            break;

      case e_VertexBufferUsage_DynamicRead: GLfunc = GL_DYNAMIC_READ;
                                            break;

      case e_VertexBufferUsage_DynamicCopy: GLfunc = GL_DYNAMIC_COPY;
                                            break;

    }

    return GLfunc;
  }

  VertexBufferID OpenGLRenderer3D::CreateVertexBuffer(float *vertices, unsigned int verticesDataSize, std::vector<unsigned int> indices, e_VertexBufferUsage usage) {

    GLuint iid; // element indices
    unsigned int vertexArrayID;

    bool pingPong = false;
    if (usage == e_VertexBufferUsage_DynamicDraw || usage == e_VertexBufferUsage_DynamicRead || usage == e_VertexBufferUsage_DynamicCopy ||
        usage == e_VertexBufferUsage_StreamDraw || usage == e_VertexBufferUsage_StreamRead || usage == e_VertexBufferUsage_StreamCopy) pingPong = true;
    pingPong = false; // use orphaning instead
    int number = 1;
    if (pingPong) number = 2;

    int firstBuffer_id = 0;
    int firstArray_id = 0;

    for (int b = 0; b < number; b++) {
      GLuint vid;
      glGenVertexArrays(1, &vid);
      glBindVertexArray(vid);
      vertexArrayID = vid;
      if (b == 0) firstArray_id = vertexArrayID;


      GLuint bid;
      glGenBuffers(1, &bid);
      int buffer_id = bid;
      if (b == 0) firstBuffer_id = buffer_id;
      //printf("array size: %i\n", triangleCount * 3 * 3);
      glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
      glBufferData(GL_ARRAY_BUFFER, verticesDataSize * sizeof(float), NULL, GetGLVertexBufferUsage(usage));
      glBufferSubData(GL_ARRAY_BUFFER, 0, verticesDataSize * sizeof(float), vertices);


      // generate a buffer for the indices

      if (indices.size() == 0) {
        for (unsigned int i = 0; i < verticesDataSize / GetTriangleMeshElementCount() / 3; i++) {
          indices.push_back(i);
        }
      }
      glGenBuffers(1, &iid);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iid);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), NULL, GetGLVertexBufferUsage(usage));
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(unsigned int), &indices[0]);


      #define BUFFER_OFFSET( i ) ((char *)NULL + (i))
      for (int i = 0; i < GetTriangleMeshElementCount(); i++) {
        glVertexAttribPointer((GLuint)i, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesDataSize / GetTriangleMeshElementCount() * i * sizeof(float)));
        glEnableVertexAttribArray(i);
      }

      if (b == 1) {
        VAOReadIndex.insert(std::pair<int, int>(firstArray_id, 0));
        VBOPingPongMap.insert(std::pair<int, int>(firstBuffer_id, buffer_id));
        VAOPingPongMap.insert(std::pair<int, int>(firstArray_id, vertexArrayID));
      }
    }

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    VertexBufferID vertexBufferID;
    vertexBufferID.bufferID = firstBuffer_id;
    vertexBufferID.vertexArrayID = firstArray_id;
    vertexBufferID.elementArrayID = iid;

    //vertexArrayID = firstArray_id;
    return vertexBufferID;
  }

  void OpenGLRenderer3D::UpdateVertexBuffer(VertexBufferID vertexBufferID, float *vertices, unsigned int verticesDataSize) {
    int writeVertexBufferID = vertexBufferID.bufferID;
    int writeVertexArrayID = vertexBufferID.vertexArrayID;

    int pingPongReadSwitch = 0;
    //bool pingPong = false;
    std::map<int, int>::iterator pingPongIter = VAOReadIndex.find(vertexBufferID.vertexArrayID);
    if (pingPongIter != VAOReadIndex.end()) {
      pingPongReadSwitch = pingPongIter->second;
      //pingPong = true;

      pingPongIter->second++;
      if (pingPongIter->second == 2) pingPongIter->second = 0;
    }

    if (pingPongReadSwitch == 0) {
      pingPongIter = VBOPingPongMap.find(vertexBufferID.bufferID);
      if (pingPongIter != VBOPingPongMap.end()) {
        writeVertexBufferID = pingPongIter->second;
      }

      pingPongIter = VAOPingPongMap.find(vertexBufferID.vertexArrayID);
      if (pingPongIter != VAOPingPongMap.end()) {
        writeVertexArrayID = pingPongIter->second;
      }

    }

    glBindVertexArray((GLuint)writeVertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, writeVertexBufferID); // todo: why is this necessary? shouldn't this be part of the vao?

    //glBufferData(GL_ARRAY_BUFFER, verticesDataSize * sizeof(float), vertices, GL_DYNAMIC_DRAW);

//    glBufferData(GL_ARRAY_BUFFER, verticesDataSize * sizeof(float), NULL, GL_DYNAMIC_DRAW);

//    if (!pingPong) {
      // i don't think we can use glMapBufferRange with GL_MAP_UNSYNCHRONIZED_BIT in a non-pingponged situation, but may want to test this sometimes
//      glBufferSubData(GL_ARRAY_BUFFER, 0, verticesDataSize * sizeof(float), vertices);
//    } else {
      //glBufferData(GL_ARRAY_BUFFER, verticesDataSize * sizeof(float), NULL, GL_STREAM_DRAW);
      //float *ptr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

      // the next 2 statements, as well as the invalidate_bit stuff in the 3rd statement, should do the same thing: orphaning the vertexbuffer.
      // however, on my current AMD (HD 6850), only the first seems to actually work! is this an AMD driver bug? can't find anything about it on the interwebz..

      glBufferData(GL_ARRAY_BUFFER, verticesDataSize * sizeof(float), NULL, GL_DYNAMIC_DRAW);
      //glInvalidateBufferData(vertexBufferID.bufferID);
      float *ptr = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0, verticesDataSize * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT); // GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
      memcpy(ptr, vertices, verticesDataSize * sizeof(float));
      glUnmapBuffer(GL_ARRAY_BUFFER);

//      glBufferData(GL_ARRAY_BUFFER, verticesDataSize * sizeof(float), vertices, GL_DYNAMIC_DRAW);

//    }

    // this fence doesn't seem necessary. the frame swap will glflush things anyway, so as long as you don't update the mesh and render stuff in the same frame, you should be okay.
    // ^ investigate if this is correct thinking. not sure. hairstyles seem to be late, so there is something wrong with omitting the fence.
    // if it's true, users of the engine will need to make sure to not update and render in parallel. maybe make this an option? maybe put something about it in a future manual?
    //GLsync syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
/*fenceoff
    // delete possible previous sync object
    std::map<int, GLsync>::iterator syncIter = VAOfence.find(writeVertexArrayID);
    if (syncIter != VAOfence.end()) {
      glDeleteSync(syncIter->second);
      VAOfence.erase(syncIter);
    }
*/
    //VAOfence.insert(std::pair<int, GLsync>(writeVertexArrayID, syncObj));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void OpenGLRenderer3D::DeleteVertexBuffer(VertexBufferID vertexBufferID) {
    GLuint glVertexBufferID = vertexBufferID.bufferID;
    glDeleteBuffers(1, &glVertexBufferID);

    GLuint glElementArrayID = vertexBufferID.elementArrayID;
    glDeleteBuffers(1, &glElementArrayID);

    GLuint glVertexArrayID = vertexBufferID.vertexArrayID;
    glDeleteVertexArrays(1, &glVertexArrayID);

    std::map<int, int>::iterator pingPongIter = VBOPingPongMap.find(vertexBufferID.bufferID);
    if (pingPongIter != VBOPingPongMap.end()) {
      glVertexBufferID = pingPongIter->second;
      glDeleteBuffers(1, &glVertexBufferID);
      VBOPingPongMap.erase(pingPongIter);
    }

    pingPongIter = VAOPingPongMap.find(vertexBufferID.vertexArrayID);
    if (pingPongIter != VAOPingPongMap.end()) {
      glVertexArrayID = pingPongIter->second;
      glDeleteVertexArrays(1, &glVertexArrayID);
      VAOPingPongMap.erase(pingPongIter);
    }

    pingPongIter = VAOReadIndex.find(vertexBufferID.vertexArrayID);
    if (pingPongIter != VAOReadIndex.end()) {
      VAOReadIndex.erase(pingPongIter);
    }

/*fenceoff
    // delete sync object
    std::map<int, GLsync>::iterator syncIter = VAOfence.find(vertexBufferID.vertexArrayID);
    if (syncIter != VAOfence.end()) {
      glDeleteSync(syncIter->second);
      VAOfence.erase(syncIter);
    }
*/
  }

  void DrawBufferChunk(int startIndex, int count) {
    // draw buffer
    if (count > 0) {
      #define BUFFER_OFFSET( i ) ((char *)NULL + (i))
      glDrawRangeElements(GL_TRIANGLES, startIndex, startIndex + count, count, GL_UNSIGNED_INT, BUFFER_OFFSET(startIndex * sizeof(unsigned int)));
    }
  }

  void OpenGLRenderer3D::RenderVertexBuffer(const std::deque<VertexBufferQueueEntry> &vertexBufferQueue, e_RenderMode renderMode) {

    /*
    CPrecisionTimer myTimer;
    boost::this_thread::yield();
    myTimer.Start();

    printf("it took %f milliseconds\n", myTimer.Stop() * 1000.0);
    */

    //glMatrixMode(GL_MODELVIEW);

    VertexBuffer *vertexBuffer;

    if (renderMode != e_RenderMode_GeometryOnly) {
      glEnable(GL_TEXTURE_2D);
    } else {
      glDisable(GL_TEXTURE_2D);
    }
    // if (renderMode == e_RenderMode_GeometryOnly || renderMode == e_RenderMode_Diffuse) {
    //   glDisable(GL_LIGHTING);
    // } else {
    //   glEnable(GL_LIGHTING);
    // }

    signed int currentDiffuseTextureID = -1;
    signed int currentNormalTextureID = -1;
    signed int currentSpecularTextureID = -1;
    signed int currentIlluminationTextureID = -1;
    signed int currentBoundBuffer = -1;

    //int bufferSize = vertexBufferQueue.size();
    int bufferSwitches = 0;

    //glPolygonMode(GL_FRONT, GL_FILL);
    //glColor3f(1, 1, 1);

    //glEnableClientState(GL_VERTEX_ARRAY);

   	Matrix4 transform;

    std::deque<VertexBufferQueueEntry>::const_iterator vertexBufferQueueIter = vertexBufferQueue.begin();
    while (vertexBufferQueueIter != vertexBufferQueue.end()) {
      const VertexBufferQueueEntry *queueEntry = &(*vertexBufferQueueIter);

      vertexBuffer = queueEntry->vertexBuffer->GetResource();
      if (currentBoundBuffer != vertexBuffer->GetVaoID()) {
        bufferSwitches++;

        int vaoID = vertexBuffer->GetVaoID();
        int pingPongReadSwitch = 0;
        std::map<int, int>::iterator pingPongIter = VAOReadIndex.find(vaoID);
        if (pingPongIter != VAOReadIndex.end()) {
          pingPongReadSwitch = pingPongIter->second;
        }

        if (pingPongReadSwitch == 1) {
          std::map<int, int>::iterator pingPongIter = VAOPingPongMap.find(vaoID);
          if (pingPongIter != VAOPingPongMap.end()) {
            vaoID = pingPongIter->second;
          }
        }

/*
        // block on VAO update
        std::map<int, GLsync>::iterator syncIter = VAOfence.find(vaoID);
        if (syncIter != VAOfence.end()) {
          //glFlush();
          //glWaitSync(syncIter->second, 0, GL_TIMEOUT_IGNORED);
          GLenum returnVal = GL_WAIT_FAILED;
          while (returnVal != GL_ALREADY_SIGNALED && returnVal != GL_CONDITION_SATISFIED) {
            returnVal = glClientWaitSync(syncIter->second, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
            //returnVal = glClientWaitSync(syncIter->second, 0, 1000);
            //if (returnVal == GL_TIMEOUT_EXPIRED) printf("TIMEOUT EXPIRED!\n");
            assert(returnVal != GL_INVALID_VALUE);
          }
        }
*/

        currentBoundBuffer = vaoID;
        glBindVertexArray(vaoID);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->GetElementID());
/*
        if (renderMode != e_RenderMode_GeometryOnly) {
          if (renderMode == e_RenderMode_Full) {
          }
        }
*/

      }

      //glPushMatrix();

      // slow, why?
      transform.Construct(queueEntry->position, Vector3(1, 1, 1), queueEntry->rotation);
      //transform.Transpose();
      //glMultMatrixf((float*)transform.elements);
      SetMatrix("modelMatrix", transform);

      bool sequential = true; // buffer vertexbuffer chunks until a change happens (in texture or index, for example)
      struct BufferChunk {
        BufferChunk() {
          startIndex = 0;
          count = 0;
        }
        int startIndex;
        int count;
      };
      BufferChunk bufferChunk;

      std::deque<VertexBufferIndex>::const_iterator vertexBufferIter = queueEntry->vertexBufferIndices.begin();
      while (vertexBufferIter != queueEntry->vertexBufferIndices.end()) {

        //VertexBufferIndex *vbIndex = (*vertexBufferIter).get();
        const VertexBufferIndex *vbIndex = &(*vertexBufferIter);

        if (renderMode != e_RenderMode_GeometryOnly) {
          int diffuseTextureID = 0;
          int normalTextureID = 0;
          int specularTextureID = 0;
          int illuminationTextureID = 0;
          if (vbIndex->material.diffuseTexture) {
            diffuseTextureID = vbIndex->material.diffuseTexture->GetResource()->GetID();
          }

          if (renderMode == e_RenderMode_Full) {
            if (vbIndex->material.normalTexture) {
              normalTextureID = vbIndex->material.normalTexture->GetResource()->GetID();
            }
            if (vbIndex->material.specularTexture) {
              specularTextureID = vbIndex->material.specularTexture->GetResource()->GetID();
            }
            if (vbIndex->material.illuminationTexture) {
              illuminationTextureID = vbIndex->material.illuminationTexture->GetResource()->GetID();
            }
          }

          if (diffuseTextureID != currentDiffuseTextureID ||
              normalTextureID != currentNormalTextureID ||
              specularTextureID != currentSpecularTextureID ||
              illuminationTextureID != currentIlluminationTextureID) {

            if (sequential) {
              DrawBufferChunk(bufferChunk.startIndex, bufferChunk.count);
              sequential = false;
            }

            bool has_normal = false;
            bool has_specular = false;
            bool has_illumination = false;
            if (normalTextureID != 0) has_normal = true;
            if (specularTextureID != 0) has_specular = true;
            if (illuminationTextureID != 0) has_illumination = true;

            if (renderMode == e_RenderMode_Full) {
              if (has_normal && normalTextureID != currentNormalTextureID) {
                SetTextureUnit(1);
                glBindTexture(GL_TEXTURE_2D, normalTextureID);
              }
              if (has_specular && specularTextureID != currentSpecularTextureID) {
                SetTextureUnit(2);
                glBindTexture(GL_TEXTURE_2D, specularTextureID);
              }
              if (has_illumination && illuminationTextureID != currentIlluminationTextureID) {
                SetTextureUnit(3);
                glBindTexture(GL_TEXTURE_2D, illuminationTextureID);
              }
            }

            SetTextureUnit(0);
            glBindTexture(GL_TEXTURE_2D, diffuseTextureID);

            if (renderMode == e_RenderMode_Full) {
              //SetUniformFloat("simple", "shininess", vbIndex->material.shininess * 60 + 1);
              //SetUniformFloat("simple", "specular", vbIndex->material.specular_amount);
              //SetUniformFloat("simple", "self_illumination", vbIndex->material.self_illumination.coords[0]);
              //SetUniformFloat3("simple", "materialparams", vbIndex->material.shininess * 60 + 1, vbIndex->material.specular_amount, vbIndex->material.self_illumination.coords[0]);
              SetUniformFloat3("simple", "materialparams", vbIndex->material.shininess, vbIndex->material.specular_amount, vbIndex->material.self_illumination.coords[0]);

              //SetUniformInt("simple", "has_normal", (int)has_normal);
              //SetUniformInt("simple", "has_specular", (int)has_specular);
              //SetUniformInt("simple", "has_illumination", (int)has_illumination);
              SetUniformFloat3("simple", "materialbools", (int)has_normal, (int)has_specular, (int)has_illumination);
            }

            currentDiffuseTextureID = diffuseTextureID;
            currentNormalTextureID = normalTextureID;
            currentSpecularTextureID = specularTextureID;
            currentIlluminationTextureID = illuminationTextureID;
          }
        } // if !GeometryOnly

        int start = vbIndex->startIndex;
        int count = vbIndex->size;

        //if (currentShader->first.compare("zphase") == 0) printf("drawing %i elements in zphase mode\n", count);

        if (sequential) if (bufferChunk.startIndex + bufferChunk.count != start) {
          DrawBufferChunk(bufferChunk.startIndex, bufferChunk.count);
          sequential = false;
        }

        //if (sequential) if (vertexBufferIter + 1 == queueEntry->vertexBufferIndices.end()) sequential = false; // data end
        if (sequential) {
          // add to buffer
          bufferChunk.count += count;
        } else {
          // reset buffer
          bufferChunk.startIndex = start;
          bufferChunk.count = count;
          sequential = true;
        }

        vertexBufferIter++;

        if (sequential) if (vertexBufferIter == queueEntry->vertexBufferIndices.end()) {
          DrawBufferChunk(bufferChunk.startIndex, bufferChunk.count);
          sequential = false;
        }

      }

      vertexBufferQueueIter++;
    }

    //printf("%i buffer switches\n", bufferSwitches);

    if (renderMode != e_RenderMode_GeometryOnly) {
      if (renderMode == e_RenderMode_Full) {
        SetTextureUnit(1);
        glBindTexture(GL_TEXTURE_2D, 0);
        SetTextureUnit(2);
        glBindTexture(GL_TEXTURE_2D, 0);
        SetTextureUnit(3);
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      SetTextureUnit(0);
      glBindTexture(GL_TEXTURE_2D, 0);
      glDisable(GL_TEXTURE_2D);
    }

    glBindVertexArray(0);
  }

  void OpenGLRenderer3D::RenderAABB(std::list<VertexBufferQueueEntry> &vertexBufferQueue) {

    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    glColor3f(0, 1, 0);

    glBegin(GL_LINES);

    std::list<VertexBufferQueueEntry>::iterator vertexBufferQueueIter = vertexBufferQueue.begin();
    while (vertexBufferQueueIter != vertexBufferQueue.end()) {
      VertexBufferQueueEntry *queueEntry = &(*vertexBufferQueueIter);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);


      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);


      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      vertexBufferQueueIter++;
    }

    glEnd();

  }

  void OpenGLRenderer3D::RenderAABB(std::list<LightQueueEntry> &lightQueue) {

    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    glColor3f(0, 0, 1);

    glBegin(GL_LINES);

    std::list<LightQueueEntry>::iterator lightQueueIter = lightQueue.begin();
    while (lightQueueIter != lightQueue.end()) {
      LightQueueEntry *queueEntry = &(*lightQueueIter);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);


      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);


      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.minxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.minxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.minxyz.coords[2]);
      glVertex3f(queueEntry->aabb.maxxyz.coords[0], queueEntry->aabb.maxxyz.coords[1], queueEntry->aabb.maxxyz.coords[2]);

      lightQueueIter++;
    }

    glEnd();
  }


  // lights

  void OpenGLRenderer3D::SetLight(const Vector3 &position, const Vector3 &color, float radius) {

    Vector3 pos = position;

    //printf("%f %f %f\n", cameraPos.coords[0], cameraPos.coords[1], cameraPos.coords[2]);

    GLfloat positionF[4] = { pos.coords[0], pos.coords[1], pos.coords[2], 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, positionF);
    SetUniformFloat3(currentShader->first, "lightPosition", pos.coords[0], pos.coords[1], pos.coords[2]);

    GLfloat diffuseF[4] = { color.coords[0], color.coords[1], color.coords[2], radius };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseF);

    GLfloat specular[3] = { 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glEnable(GL_LIGHT0);
  }


  // textures

  GLenum GetGLPixelFormat(e_PixelFormat pixelFormat) {
    GLenum format = GL_RGB;

    switch (pixelFormat) {

      case e_PixelFormat_Alpha:          format = GL_ALPHA; break;
      case e_PixelFormat_RGB:            format = GL_RGB; break;
      case e_PixelFormat_RGBA:           format = GL_RGBA; break; // todo: BGRA preferred on windows?
      case e_PixelFormat_DepthComponent: format = GL_DEPTH_COMPONENT; break;
      case e_PixelFormat_Luminance:      format = GL_LUMINANCE; break;
    }

    return format;
  }

  GLenum GetGLInternalPixelFormat(e_InternalPixelFormat pixelFormat) {
    GLenum format = GL_RGB8;

    switch (pixelFormat) {

      case e_InternalPixelFormat_RGB8:      format = GL_RGB8; break;
      case e_InternalPixelFormat_SRGB8:     format = GL_SRGB8; break;
      case e_InternalPixelFormat_RGB16:     format = GL_RGB16; break;
      case e_InternalPixelFormat_RGBA4:     format = GL_RGBA4; break;
      case e_InternalPixelFormat_RGBA8:     format = GL_RGBA8; break;
      case e_InternalPixelFormat_SRGBA8:    format = GL_SRGB8_ALPHA8; break;
      case e_InternalPixelFormat_RGBA16:    format = GL_RGBA16; break;
      case e_InternalPixelFormat_RGBA16F:   format = GL_RGBA16F; break;
      case e_InternalPixelFormat_RGBA32F:   format = GL_RGBA32F; break;
      case e_InternalPixelFormat_RGB5_A1:   format = GL_RGB5_A1; break;
      case e_InternalPixelFormat_DepthComponent:    format = GL_DEPTH_COMPONENT; break;
      case e_InternalPixelFormat_DepthComponent16:  format = GL_DEPTH_COMPONENT16; break;
      case e_InternalPixelFormat_DepthComponent24:  format = GL_DEPTH_COMPONENT24; break;
      case e_InternalPixelFormat_DepthComponent32:  format = GL_DEPTH_COMPONENT32; break;
      case e_InternalPixelFormat_DepthComponent32F: format = GL_DEPTH_COMPONENT32F; break;
      case e_InternalPixelFormat_StencilIndex8: format = GL_STENCIL_INDEX8; break;
    }

    return format;
  }

  int OpenGLRenderer3D::CreateTexture(e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, int width, int height, bool alpha, bool repeat, bool mipmaps, bool filter, bool multisample, bool compareDepth) {

/*
    if (pot(source->w) != source->w || pot(source->h) != source->h) {
      Log(e_Warning, "OpenGLRenderer3D", "CreateTexture", "Non-power-of-2 texture!");
    }
*/

    GLuint texID;
    glGenTextures(1, &texID);
    BindTexture(texID);

    GLint filter_min, filter_mag;

    if (filter == true) {
      filter_min = (mipmaps) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
      filter_mag = GL_LINEAR;
    } else {
      filter_min = (mipmaps) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      filter_mag = GL_NEAREST;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);
    if (internalPixelFormat == e_InternalPixelFormat_DepthComponent16 ||
        internalPixelFormat == e_InternalPixelFormat_DepthComponent24 ||
        internalPixelFormat == e_InternalPixelFormat_DepthComponent32 ||
        internalPixelFormat == e_InternalPixelFormat_DepthComponent32F) {
      //XXglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      if (compareDepth) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      }
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      // Post-GLSL 1.3, DEPTH_TEXTURE_MODE is deprecated and GLSL behaves as if its always set to LUMINANCE
      //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
      //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
    }

    if (filter) {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    }

    if (!multisample) {
      glTexImage2D(GL_TEXTURE_2D, 0, GetGLInternalPixelFormat(internalPixelFormat), width, height, 0, GetGLPixelFormat(pixelFormat), GL_UNSIGNED_BYTE, NULL);
    } else {
      // todo: needs opengl 3.2 libs!
      //glTexImage2DMultiSample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 5, GetGLInternalPixelFormat(internalPixelFormat), width, height, GL_TRUE);
    }

/* test if this actually helps
    GLclampf prior = (width * height) / 1048576.0; // 1024*1024 tex = max priority
    //printf("%f\n", (pot(source->w) * pot(source->h)) / 1048576.0);
    glPrioritizeTextures(1, (const GLuint*)&texID, &prior);
*/
    BindTexture(0);

    return texID;
  }

  void OpenGLRenderer3D::ResizeTexture(int textureID, SDL_Surface *source, e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, bool alpha, bool mipmaps) {

    BindTexture(textureID);

    bool repeat = false;
    bool filter = true;
    if (!mipmaps) filter = false; // <-- todo: hack
    GLint filter_min, filter_mag;

    if (filter == true) {
      filter_min = (mipmaps) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
      filter_mag = GL_LINEAR;
      //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    } else {
      filter_min = (mipmaps) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      filter_mag = GL_NEAREST;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);
    if (internalPixelFormat == e_InternalPixelFormat_DepthComponent16 ||
        internalPixelFormat == e_InternalPixelFormat_DepthComponent24 ||
        internalPixelFormat == e_InternalPixelFormat_DepthComponent32 ||
        internalPixelFormat == e_InternalPixelFormat_DepthComponent32F) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    }

    if (mipmaps) {
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }

    int x = 0;
    int y = 0;
    int width = source->w;
    int height = source->h;

    SDL_LockSurface(source);
    glTexImage2D(GL_TEXTURE_2D, 0, GetGLInternalPixelFormat(internalPixelFormat), width, height, 0, GetGLPixelFormat(pixelFormat), GL_UNSIGNED_BYTE, source->pixels);
    SDL_UnlockSurface(source);

    //GLclampf prior = (width * height) / 1048576.0; // 1024*1024 tex = max priority
    //printf("%f\n", (pot(source->w) * pot(source->h)) / 1048576.0);
    //glPrioritizeTextures(1, (const GLuint*)&textureID, &prior);

    BindTexture(0);
  }

  void OpenGLRenderer3D::UpdateTexture(int textureID, SDL_Surface *source, bool alpha, bool mipmaps) {

    glBindTexture(GL_TEXTURE_2D, textureID);

    GLint type1 = GL_RGBA8;
    GLint type2 = (alpha) ? GL_RGBA : GL_RGB;

    int x = 0;
    int y = 0;
    int w = source->w;
    int h = source->h;

    GLint filter_min, filter_mag;

    bool filter = true;

    if (filter == true) {
      filter_min = (mipmaps) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
      filter_mag = GL_LINEAR;
      //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    } else {
      filter_min = (mipmaps) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      filter_mag = GL_NEAREST;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);

    if (mipmaps) {
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    SDL_LockSurface(source);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, type2, GL_UNSIGNED_BYTE, source->pixels);

    SDL_UnlockSurface(source);

    BindTexture(0);
  }

  void OpenGLRenderer3D::DeleteTexture(int textureID) {
    GLuint texID = textureID;
    //printf("deleting tex id: %i\n", textureID);
    glDeleteTextures(1, &texID);
  }

  void OpenGLRenderer3D::CopyFrameBufferToTexture(int textureID, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
  }

  void OpenGLRenderer3D::BindTexture(int textureID) {
    glBindTexture(GL_TEXTURE_2D, (GLuint)textureID);
  }

  void OpenGLRenderer3D::SetTextureUnit(int textureUnit) {
    if (textureUnit != _cache_activeTextureUnit) {
      //assert(glActiveTexture);
      glActiveTexture(GL_TEXTURE0 + (GLuint)textureUnit);
      _cache_activeTextureUnit = textureUnit;
    }
  }

  void OpenGLRenderer3D::SetClientTextureUnit(int textureUnit) {
    //assert(glClientActiveTexture);
    glClientActiveTexture(GL_TEXTURE0 + (GLuint)textureUnit);
  }


  // frame buffer objects

  GLenum GetGLTargetAttachment(e_TargetAttachment targetAttachment) {
    GLenum target = GL_FRONT;

    switch (targetAttachment) {

      case e_TargetAttachment_None:    target = GL_NONE; break;
      case e_TargetAttachment_Front:   target = GL_FRONT_LEFT; break;
      case e_TargetAttachment_Back:    target = GL_BACK_LEFT; break;
      case e_TargetAttachment_Depth:   target = GL_DEPTH_ATTACHMENT; break;
      case e_TargetAttachment_Stencil: target = GL_STENCIL_ATTACHMENT; break;
      case e_TargetAttachment_Color0:  target = GL_COLOR_ATTACHMENT0; break;
      case e_TargetAttachment_Color1:  target = GL_COLOR_ATTACHMENT1; break;
      case e_TargetAttachment_Color2:  target = GL_COLOR_ATTACHMENT2; break;
      case e_TargetAttachment_Color3:  target = GL_COLOR_ATTACHMENT3; break;
    }

    return target;
  }

  int OpenGLRenderer3D::CreateFrameBuffer() {
    GLuint id;
    glGenFramebuffers(1, &id);
    return id;
  }

  void OpenGLRenderer3D::DeleteFrameBuffer(int fbID) {
    const GLuint id = fbID;
    glDeleteFramebuffers(1, &id);
  }

  void OpenGLRenderer3D::BindFrameBuffer(int fbID) {
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)fbID);
  }

  void OpenGLRenderer3D::SetFrameBufferRenderBuffer(e_TargetAttachment targetAttachment, int rbID) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GetGLTargetAttachment(targetAttachment), GL_RENDERBUFFER, rbID);
  }

  void OpenGLRenderer3D::SetFrameBufferTexture2D(e_TargetAttachment targetAttachment, int texID) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GetGLTargetAttachment(targetAttachment), GL_TEXTURE_2D, texID, 0);
  }

  bool OpenGLRenderer3D::CheckFrameBufferStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
      Log(e_Notice, "OpenGLRenderer3D", "CheckFrameBufferStatus", "Framebuffer state #" + int_to_str(status));
      return true;
    } else {
      Log(e_FatalError, "OpenGLRenderer3D", "CheckFrameBufferStatus", "Framebuffer error state #" + int_to_str(status));
      return false;
    }
  }


  // render buffers

  int OpenGLRenderer3D::CreateRenderBuffer() {
    GLuint id;
    glGenRenderbuffers(1, &id);
    return id;
  }

  void OpenGLRenderer3D::DeleteRenderBuffer(int rbID) {
    GLuint glrbID = rbID;
    glDeleteRenderbuffers(1, &glrbID);
  }

  void OpenGLRenderer3D::BindRenderBuffer(int rbID) {
    glBindRenderbuffer(GL_RENDERBUFFER, (GLuint)rbID);
  }

  void OpenGLRenderer3D::SetRenderBufferStorage(e_InternalPixelFormat internalPixelFormat, int width, int height) {
    glRenderbufferStorage(GL_RENDERBUFFER, GetGLInternalPixelFormat(internalPixelFormat), width, height);
  }


  // render targets

  void OpenGLRenderer3D::SetRenderTargets(std::vector<e_TargetAttachment> targetAttachments) {
    GLenum targets[targetAttachments.size()];
    for (int i = 0; i < (signed int)targetAttachments.size(); i++) {
      targets[i] = GetGLTargetAttachment(targetAttachments.at(i));
    }
    glDrawBuffers(targetAttachments.size(), targets);
  }


  // utility

  void OpenGLRenderer3D::SetFOV(float angle) {
    FOV = angle;
  }

  void OpenGLRenderer3D::PushAttribute(int attr) {
    glPushAttrib((GLbitfield)attr);
  }

  void OpenGLRenderer3D::PopAttribute() {
    glPopAttrib();
  }

  void OpenGLRenderer3D::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
  }

  void OpenGLRenderer3D::GetContextSize(int &width, int &height, int &bpp) {
    width = context_width;
    height = context_height;
    bpp = context_bpp;
  }

  void OpenGLRenderer3D::SetPolygonOffset(float scale, float bias) {
    glPolygonOffset(scale, bias); // hint: doesn't work with glsl since i'm using my own calculations
  }

  // shaders

  void LoadGLShader(GLuint shaderID, const std::string &filename) {
    std::vector<std::string> source;
    file_to_vector(filename, source);

    std::string source_flat;
    for (int i = 0; i < (signed int)source.size(); i++) {
      source_flat.append(source.at(i).c_str());
      source_flat.append("\n");
    }

   	const char *sourceChar = source_flat.c_str();
   	glShaderSource(shaderID, 1, &sourceChar, NULL);
    glCompileShader(shaderID);

    GLint compiled;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
      //printf("shader compilation error (%s)\n", filename.c_str());
      Log(e_Warning, "", "LoadGLShader", "shader compilation error (" + filename + ")");
    }

    GLint blen = 0;
    GLsizei slen = 0;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH , &blen);
    if (blen > 1) {
      GLchar* compiler_log = (GLchar*)malloc(blen);
      glGetInfoLogARB(shaderID, blen, &slen, compiler_log);
      //printf("shader compilation info: %s\n", compiler_log);
      Log(e_Warning, "", "LoadGLShader", "shader compilation info (" + filename + "):\n" + std::string(compiler_log));
      free(compiler_log);
    }
  }

  void GeneratePoissonKernel(float *kernel, unsigned int kernelSize) {

    bool usePrecalculatedSet = false;

    if (!usePrecalculatedSet || kernelSize != 32) {
      unsigned int candidateSize = 32;

      Vector3 samples[kernelSize];
      Vector3 candidates[candidateSize];

      for (unsigned int i = 0; i < kernelSize; i++) {


        // first, create a set of candidates. all these are on the 'hull' of a hemisphere.

        for (unsigned int candidate = 0; candidate < candidateSize; candidate++) {
          //candidates[candidate] = Vector3(random(-1.0f, 1.0f), random(-1.0f, 1.0f), random(0.001f, 1.0f));
          //candidates[candidate] = candidates[candidate].GetNormalized(0) * random(0.001f, 1.0f);

          // http://www.pouet.net/topic.php?which=9613&page=1 (evenly distributed points on hemisphere)
          // "To generate uniformly distributed random points on a sphere, you generate them on a unit cylinder (but not the caps), and project onto the sphere. While it's not intuitive to see, this actually gives uniform distribution."
          // (also see page 2 for corrections!)
          float s = random(0.0f, 2.0f * pi);
          float t = random(-1.0f, 1.0f);
          float m = sqrt(1.0f - t * t);
          candidates[candidate] = Vector3(sin(s) * m, cos(s) * m, fabs(t)); // fabs for hemi
        }


        // second, pick the candidate furthest away from the samples already there. this will distribute them evenly all over the hull.

        int furthestCandidateID = 0;
        float furthestCandidateDistance = 0.0f;
        for (unsigned int candidate = 0; candidate < candidateSize; candidate++) {
          float distanceToClosestSample = 1.0f;
          for (unsigned int x = 0; x < i; x++) {
            float distance = candidates[candidate].GetDistance(samples[x]);
            if (distance < distanceToClosestSample) distanceToClosestSample = distance;
          }
          if (distanceToClosestSample > furthestCandidateDistance) {
            furthestCandidateDistance = distanceToClosestSample;
            furthestCandidateID = candidate;
          }
        }

        samples[i] = candidates[furthestCandidateID];
      }

      for (unsigned int i = 0; i < kernelSize; i++) {


        // third, randomize the length, so the samples fill the volume.

        float minDistance = 0.14f;
        samples[i] = samples[i].GetNormalized(0) * random(minDistance, 1.0f);

        // information around the normal vec is most useful
        samples[i] = samples[i] * 0.75f + Vector3(0, 0, 1) * 0.25f;

        kernel[i * 3 + 0] = samples[i].coords[0];
        kernel[i * 3 + 1] = samples[i].coords[1];
        kernel[i * 3 + 2] = samples[i].coords[2];

        //printf("samples[%i].Set(%f, %f, %f);\n", i, samples[i].coords[0], samples[i].coords[1], samples[i].coords[2]);
      }

    } else { // PRECALCULATED SET

      Vector3 samples[kernelSize];

      // these samples seem relatively close to z = 0 (much 'ground effect' on flat surface)
      samples[0].Set(-0.164502, 0.198563, 0.847836);
      samples[1].Set(-0.371169, -0.311732, 0.271810);
      samples[2].Set(0.395387, -0.169534, 0.356410);
      samples[3].Set(0.199861, 0.154104, 0.106548);
      samples[4].Set(0.003958, -0.331189, 0.151894);
      samples[5].Set(-0.128020, 0.276624, 0.133732);
      samples[6].Set(-0.576721, 0.032381, 0.472200);
      samples[7].Set(0.042701, 0.189697, 0.215682);
      samples[8].Set(0.095873, -0.029585, 0.376675);
      samples[9].Set(-0.018931, -0.559727, 0.723850);
      samples[10].Set(0.354553, -0.531634, 0.250052);
      samples[11].Set(0.211686, 0.082632, 0.250673);
      samples[12].Set(-0.586181, -0.060375, 0.086480);
      samples[13].Set(0.529458, -0.009977, 0.084485);
      samples[14].Set(-0.452887, -0.303541, 0.721515);
      samples[15].Set(-0.223906, 0.180655, 0.100284);
      samples[16].Set(-0.278485, -0.709826, 0.082907);
      samples[17].Set(0.195230, 0.204728, 0.573875);
      samples[18].Set(-0.426917, 0.460452, 0.610940);
      samples[19].Set(0.115942, -0.134919, 0.290054);
      samples[20].Set(-0.089691, -0.266767, 0.910242);
      samples[21].Set(0.259113, 0.638836, 0.159923);
      samples[22].Set(0.621548, -0.384182, 0.089235);
      samples[23].Set(0.261258, 0.331540, 0.306923);
      samples[24].Set(0.170094, -0.678424, 0.471292);
      samples[25].Set(-0.398759, -0.300976, 0.082431);
      samples[26].Set(-0.291338, 0.080905, 0.159125);
      samples[27].Set(-0.391219, 0.082053, 0.638896);
      samples[28].Set(0.051567, 0.476222, 0.765788);
      samples[29].Set(-0.181921, 0.545340, 0.430360);
      samples[30].Set(0.542024, 0.161835, 0.173717);
      samples[31].Set(-0.059420, 0.778527, 0.233026);

      Vector3 average;
      for (unsigned int i = 0; i < kernelSize; i++) {
        average += samples[i];
      }
      average /= kernelSize;
      printf("SSAO kernel average position: ");
      average.Print();
      Vector3 newAverage;
      for (unsigned int i = 0; i < kernelSize; i++) {
        samples[i] -= average.Get2D();
        kernel[i * 3 + 0] = samples[i].coords[0];
        kernel[i * 3 + 1] = samples[i].coords[1];
        kernel[i * 3 + 2] = samples[i].coords[2];
        newAverage += samples[i];
      }
      newAverage /= kernelSize;
      printf("SSAO kernel average position (corrected): ");
      newAverage.Print();
    }

  }

  void OpenGLRenderer3D::LoadShader(const std::string &name, const std::string &filename) {
    GLint info;

    Shader shader;
    shader.name = filename;

    shader.vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    LoadGLShader(shader.vertexShaderID, filename + ".vert");
    glGetShaderiv(shader.vertexShaderID, GL_COMPILE_STATUS, &info);
    if (info != GL_TRUE || glIsShader(shader.vertexShaderID) != GL_TRUE) {
      Log(e_FatalError, "OpenGLRenderer3D", "LoadShader", "Could not compile vertex program: " + name);
    }

    shader.fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    LoadGLShader(shader.fragmentShaderID, filename + ".frag");
    glGetShaderiv(shader.fragmentShaderID, GL_COMPILE_STATUS, &info);
    if (info != GL_TRUE || glIsShader(shader.fragmentShaderID) != GL_TRUE) {
      Log(e_FatalError, "OpenGLRenderer3D", "LoadShader", "Could not compile fragment program: " + name);
    }

    shader.programID = glCreateProgram();
    glAttachShader(shader.programID, shader.vertexShaderID);
    glAttachShader(shader.programID, shader.fragmentShaderID);

    shaders.insert(std::pair<std::string, Shader>(name, shader));

    if (name == "zphase") {
      glBindAttribLocation(shader.programID, 0, "position");
      glBindFragDataLocation(shader.programID, 0, "stdout");
    }
    if (name == "simple") {
      glBindAttribLocation(shader.programID, 0, "position");
      glBindAttribLocation(shader.programID, 1, "normal");
      glBindAttribLocation(shader.programID, 2, "texcoord");
      glBindAttribLocation(shader.programID, 3, "tangent");
      glBindAttribLocation(shader.programID, 4, "bitangent");
      glBindFragDataLocation(shader.programID, 0, "stdout0");
      glBindFragDataLocation(shader.programID, 1, "stdout1");
      glBindFragDataLocation(shader.programID, 2, "stdout2");
    }
    if (name == "ambient") {
      glBindAttribLocation(shader.programID, 0, "position");
      glBindFragDataLocation(shader.programID, 0, "stdout");
    }
    if (name == "lighting") {
      glBindFragDataLocation(shader.programID, 0, "stdout0");
      glBindFragDataLocation(shader.programID, 1, "stdout1");
    }
    if (name == "postprocess") {
      glBindAttribLocation(shader.programID, 0, "position");
      glBindFragDataLocation(shader.programID, 0, "stdout");
    }

    glLinkProgram(shader.programID);

    glUseProgram(shader.programID);

    GLint location;
    if (name == "simple") {
      SetUniformInt("simple", "map_albedo", 0);
      SetUniformInt("simple", "map_normal", 1);
      SetUniformInt("simple", "map_specular", 2);
      SetUniformInt("simple", "map_illumination", 3);
    }
    if (name == "ambient") {
      SetUniformInt("ambient", "map_albedo", 0);
      SetUniformInt("ambient", "map_normal", 1);
      SetUniformInt("ambient", "map_depth", 2); // recently swapped with aux
      SetUniformInt("ambient", "map_aux", 3);
      SetUniformInt("ambient", "map_noise", 4);

      SetUniformFloat("ambient", "contextX", (float)0.0);
      SetUniformFloat("ambient", "contextY", (float)0.0);
      SetUniformFloat("ambient", "contextWidth", (float)context_width);
      SetUniformFloat("ambient", "contextHeight", (float)context_height);

      unsigned int kernelSize = 32;
      //SetUniformInt("ambient", "SSAO_kernelSize", kernelSize);
      float SSAO_kernel[kernelSize * 3];
      GeneratePoissonKernel(&SSAO_kernel[0], kernelSize);
      SetUniformFloat3Array("ambient", "SSAO_kernel", kernelSize, &SSAO_kernel[0]);
    }
    if (name == "lighting") {
      // on (at least older) nvidia cards, we can't 'skip' id's, or so it seems. so make them consecutive
      // edit ^ not sure after all if that is the case
      SetUniformInt("lighting", "map_albedo", 0);
      SetUniformInt("lighting", "map_normal", 1);
      SetUniformInt("lighting", "map_depth", 2); // was: 3
      //SetUniformInt("lighting", "map_aux", 3); // was: 2
      SetUniformInt("lighting", "map_shadow", 7);

      SetUniformFloat("lighting", "contextX", (float)0.0);
      SetUniformFloat("lighting", "contextY", (float)0.0);
      SetUniformFloat("lighting", "contextWidth", (float)context_width);
      SetUniformFloat("lighting", "contextHeight", (float)context_height);
    }
    if (name == "postprocess") {
      SetUniformInt("postprocess", "map_accumulation", 0);
      SetUniformInt("postprocess", "map_modifier", 1);
      SetUniformInt("postprocess", "map_depth", 2);

      SetUniformFloat("postprocess", "contextX", (float)0.0);
      SetUniformFloat("postprocess", "contextY", (float)0.0);
      SetUniformFloat("postprocess", "contextWidth", (float)context_width);
      SetUniformFloat("postprocess", "contextHeight", (float)context_height);
    }

    SetTextureUnit(0);

    UseShader("");
  }

  void OpenGLRenderer3D::SetFramebufferGammaCorrection(bool onOff) {
    if (onOff) {
      glEnable(GL_FRAMEBUFFER_SRGB);
    } else {
      glDisable(GL_FRAMEBUFFER_SRGB);
    }
  }

  void OpenGLRenderer3D::UseShader(const std::string &name) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(name);
    if (shaderIter != shaders.end()) {

      currentShader = shaderIter;

      glUseProgram((*shaderIter).second.programID);
    } else {
      glUseProgram(0);
    }
  }

  void OpenGLRenderer3D::SetUniformInt(const std::string &shaderName, const std::string &varName, int value) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformInt", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    glUniform1i(location, value);
  }

  void OpenGLRenderer3D::SetUniformInt3(const std::string &shaderName, const std::string &varName, int value1, int value2, int value3) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformInt3", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    glUniform3i(location, value1, value2, value3);
  }

  void OpenGLRenderer3D::SetUniformFloat(const std::string &shaderName, const std::string &varName, float value) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformFloat", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    glUniform1f(location, value);
  }

  void OpenGLRenderer3D::SetUniformFloat2(const std::string &shaderName, const std::string &varName, float value1, float value2) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformFloat2", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    glUniform2f(location, value1, value2);
  }

  void OpenGLRenderer3D::SetUniformFloat3(const std::string &shaderName, const std::string &varName, float value1, float value2, float value3) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformFloat3", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    glUniform3f(location, value1, value2, value3);
  }

  void OpenGLRenderer3D::SetUniformFloat3Array(const std::string &shaderName, const std::string &varName, int count, float *values) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformFloat3Array", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    glUniform3fv(location, count, values);
  }

  void OpenGLRenderer3D::SetUniformMatrix4(const std::string &shaderName, const std::string &varName, const Matrix4 &mat) {
    std::map<std::string, Shader>::iterator shaderIter = shaders.find(shaderName);
    assert(shaderIter != shaders.end());

    std::map<std::string, GLint>::iterator iter = uniformCache.find(shaderName + "_var_" + varName);
    GLint location;
    if (iter != uniformCache.end()) {
      location = iter->second;
    } else {
      location = glGetUniformLocation((*shaderIter).second.programID, varName.c_str());
      if (location == -1) Log(e_Error, "OpenGLRenderer3D", "SetUniformMatrix4", "Uniform location for shader '" + shaderName + "' not found: " + varName);
      uniformCache.insert(std::pair<std::string, GLint>(shaderName + "_var_" + varName, location));
    }
    //glUniformMatrix4fv(location, 1, false, (float*)mat.GetTransposed().elements);
    glUniformMatrix4fv(location, 1, true, (float*)mat.elements); // true == transposed
  }

  void OpenGLRenderer3D::HDRCaptureOverallBrightness() {
    float brightness = 0.0;
    int count = 0;
    for (int i = 0; i < 2; i++) {
//    for (int x = context_width * 0.2; x <= context_width * 0.8; x += context_width * 0.01) {
//      for (int y = context_height * 0.2; y <= context_height * 0.8; y += context_height * 0.01) {
      int x = random(context_width * 0.2f, context_width * 0.8f);
      int y = random(context_height * 0.2f, context_height * 0.8f);
      GLubyte pixel[3];
      glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);
      brightness += (pixel[0] + pixel[1] + pixel[2]) / 3.0f;
      count++;
    }
    brightness /= (float)count;
    //brightness /= 256.0;

    overallBrightness = overallBrightness * 0.99f + brightness * 0.01f;

    //printf("overall brightness: %f (%f)\n", overallBrightness, brightness);
  }

  float OpenGLRenderer3D::HDRGetOverallBrightness() {
    return overallBrightness;
  }


  // thread main loop

  void OpenGLRenderer3D::operator()() {
    Log(e_Notice, "OpenGLRenderer3D", "operator()()", "Starting OpenGLRenderer3D thread");

    SDL_Init(SDL_INIT_VIDEO);

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int inited = IMG_Init(flags);

    if ((inited & flags) != flags) {
      printf("IMG_Init: Failed to init required jpg and png support!\n");
      printf("IMG_Init: %s\n", IMG_GetError());
    }

    // we want the unicode stuff on keyboard keypresses (needs to be in ogl context or sdl doesn't get it)
    SDL_EnableUNICODE(SDL_ENABLE);

    SDL_Event event;

    bool quit = false;
    while (!quit) {

      // process messages

      while (SDL_PollEvent(&event)) {

        // context losing/gaining focus
        if (event.type == SDL_ACTIVEEVENT)
          if (event.active.state & SDL_APPINPUTFOCUS) event.active.gain == 0 ? contextIsActive = false : contextIsActive = true;

        switch (event.type) {
          case SDL_QUIT: EnvironmentManager::GetInstance().SignalQuit(); break;
          case SDL_KEYDOWN:
            switch(event.key.keysym.sym) {
              case SDLK_F12:
                EnvironmentManager::GetInstance().SignalQuit();
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }

        if (contextIsActive) { // context must be active
          UserEventManager::GetInstance().InputSDLEvent(event);
        }

      }

      // todo: manual joy handling? see joy init in usereventmanager
      // SDL_JoystickUpdate();

      bool isMessage;
      boost::intrusive_ptr<Command> message = messageQueue.WaitForMessage(isMessage, 1);
      if (isMessage) {
        if (!message->Handle(this)) quit = true;
        message.reset();
      }

    }

    Exit();

    IMG_Quit();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    Log(e_Notice, "OpenGLRenderer3D", "operator()()", "Shutting down OpenGLRenderer3D thread");

    if (messageQueue.GetPending() > 0) Log(e_Error, "OpenGLRenderer3D", "operator()()", int_to_str(messageQueue.GetPending()) + " messages left on quit!");
  }

}
