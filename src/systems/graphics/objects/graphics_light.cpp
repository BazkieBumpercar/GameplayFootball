// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "graphics_light.hpp"

#include "systems/graphics/rendering/r3d_messages.hpp"
#include "managers/resourcemanagerpool.hpp"

#include "../graphics_scene.hpp"
#include "../graphics_system.hpp"

#include "graphics_geometry.hpp"

namespace blunted {

  GraphicsLight::GraphicsLight(GraphicsScene *graphicsScene) : GraphicsObject(graphicsScene) {
    radius = 512;
    color.Set(1, 1, 1);
    lightType = e_LightType_Point;
    shadow = false;
  }

  GraphicsLight::~GraphicsLight() {
  }

  boost::intrusive_ptr<Interpreter> GraphicsLight::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Light) {
      boost::intrusive_ptr<GraphicsLight_LightInterpreter> LightInterpreter(new GraphicsLight_LightInterpreter(this));
      return LightInterpreter;
    }
    Log(e_FatalError, "GraphicsLight", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<GraphicsLight_LightInterpreter>();
  }

  void GraphicsLight::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 GraphicsLight::GetPosition() const {
    return position;
  }

  void GraphicsLight::SetColor(const Vector3 &newColor) {
    color = newColor;
  }

  Vector3 GraphicsLight::GetColor() const {
    return color;
  }

  void GraphicsLight::SetRadius(float radius) {
    this->radius = radius;
  }

  float GraphicsLight::GetRadius() const {
    return radius;
  }

  void GraphicsLight::SetType(e_LightType lightType) {
    this->lightType = lightType;
  }

  e_LightType GraphicsLight::GetType() const {
    return lightType;
  }

  void GraphicsLight::SetShadow(bool shadow) {
    this->shadow = shadow;
  }

  bool GraphicsLight::GetShadow() const {
    return shadow;
  }




  GraphicsLight_LightInterpreter::GraphicsLight_LightInterpreter(GraphicsLight *caller) : caller(caller) {
  }

  void GraphicsLight_LightInterpreter::OnUnload() {

    // todo IMPORTANT: find a way to dynamically unload shadow maps if a camera is removed
    // maybe a timer? if not used for x seconds, unload? would take a away the need to let camera keep record of lights

    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    std::vector<ShadowMap>::iterator iter = caller->shadowMaps.begin();
    while (iter != caller->shadowMaps.end()) {

      // verbose printf("erasing shadowmap %s (framebuffer)\n", (*iter).cameraName.c_str());

      // delete framebuffer
      boost::intrusive_ptr<Renderer3DMessage_DeleteFrameBuffer> deleteFrameBuffer(new Renderer3DMessage_DeleteFrameBuffer((*iter).frameBufferID, e_TargetAttachment_Depth));
      renderer3D->messageQueue.PushMessage(deleteFrameBuffer);
      deleteFrameBuffer->Wait();

      // happens automagically when noone links to texture anymore.. disabled here so we don't delete the texture twice
      /*
      (*iter).texture->resourceMutex.lock();
      boost::intrusive_ptr<Renderer3DMessage_DeleteTexture> deleteTexture(new Renderer3DMessage_DeleteTexture((*iter).texture->GetResource()->GetID()));
      renderer3D->messageQueue.PushMessage(deleteTexture);
      deleteTexture->Wait();
      (*iter).texture->resourceMutex.unlock();
      */

      // verbose printf("erasing shadowmap %s (links)\n", (*iter).cameraName.c_str());

      (*iter).visibleGeometry.clear();
      (*iter).texture.reset();

      iter = caller->shadowMaps.erase(iter);
    }

    delete caller;
    caller = 0;
  }

  void GraphicsLight_LightInterpreter::SetValues(const Vector3 &color, float radius) {
    caller->SetColor(color);
    caller->SetRadius(radius);
  }

  void GraphicsLight_LightInterpreter::SetType(e_LightType lightType) {
    caller->SetType(lightType);
  }

  void GraphicsLight_LightInterpreter::SetShadow(bool shadow) {
    caller->SetShadow(shadow);
  }

  bool GraphicsLight_LightInterpreter::GetShadow() {
    return caller->GetShadow();
  }

  void GraphicsLight_LightInterpreter::OnSpatialChange(const Vector3 &position, const Quaternion &rotation) {
    caller->SetPosition(position);
    //caller->SetRotation(rotation);
  }

  bool GLLI_SortVertexBufferQueueEntries(const VertexBufferQueueEntry &vb1, const VertexBufferQueueEntry &vb2) {
    return vb1.vertexBuffer->GetResource()->GetID() < vb2.vertexBuffer->GetResource()->GetID();
  }

  void GraphicsLight_LightInterpreter::EnqueueShadowMap(boost::intrusive_ptr<Camera> camera, std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry) {
    if (!caller->GetShadow()) return;

    int index = -1;
    for (int i = 0; i < (signed int)caller->shadowMaps.size(); i++) {
      if (caller->shadowMaps.at(i).cameraName == camera->GetName()) { // todo: what if cam name changes? this is hardly ideal
        index = i;
      }
    }

    if (index == -1) {

      // does not yet exist

      ShadowMap map;
      map.texture = ResourceManagerPool::GetInstance().GetManager<Texture>(e_ResourceType_Texture)->
                      Fetch(std::string(camera->GetName() + int_to_str(intptr_t(this))), false, false); // false == don't try to use loader
      if (map.texture->GetResource()->GetID() == -1) {
        Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();
        map.texture->GetResource()->SetRenderer3D(renderer3D);
        //map.texture->GetResource()->CreateTexture(e_InternalPixelFormat_DepthComponent32, e_PixelFormat_DepthComponent, 1024, 1024, false, false, false, true, true);
        map.texture->GetResource()->CreateTexture(e_InternalPixelFormat_DepthComponent16, e_PixelFormat_DepthComponent, 2048, 2048, false, false, false, true, true);
        //map.texture->GetResource()->CreateTexture(e_InternalPixelFormat_DepthComponent16, e_PixelFormat_DepthComponent, 4096, 4096, false, false, false, true, true); // filter on to get hardware shadowmap AA in shader: http://stackoverflow.com/questions/22419682/glsl-sampler2dshadow-and-shadow2d-clarification
        //map.texture->GetResource()->CreateTexture(e_InternalPixelFormat_DepthComponent32, e_PixelFormat_DepthComponent, 4096, 4096, false, false, false, true, true);
        //map.texture->GetResource()->CreateTexture(e_InternalPixelFormat_DepthComponent16, e_PixelFormat_DepthComponent, 8192, 8192, false, false, false, true, true);

        // create framebuffer for shadowmap
        boost::intrusive_ptr<Renderer3DMessage_CreateFrameBuffer> createFrameBuffer(new Renderer3DMessage_CreateFrameBuffer(e_TargetAttachment_Depth, map.texture->GetResource()->GetID()));
        renderer3D->messageQueue.PushMessage(createFrameBuffer);
        createFrameBuffer->Wait();
        map.frameBufferID = createFrameBuffer->frameBufferID;
      }

      map.cameraName = camera->GetName();

      caller->shadowMaps.push_back(map);
      index = caller->shadowMaps.size() - 1;
    }


    // add geometry

    std::deque < boost::intrusive_ptr<Geometry> >::iterator visibleGeometryIter = visibleGeometry.begin();
    while (visibleGeometryIter != visibleGeometry.end()) {
      boost::intrusive_ptr<GraphicsGeometry_GeometryInterpreter> interpreter = static_pointer_cast<GraphicsGeometry_GeometryInterpreter>((*visibleGeometryIter)->GetInterpreter(e_SystemType_Graphics));

      (*visibleGeometryIter)->LockSubject();

      // add buffers to visible geometry queue
      interpreter->GetVertexBufferQueue(caller->shadowMaps.at(index).visibleGeometry);

      // unlocksubject used to be here

      std::deque<VertexBufferQueueEntry>::iterator visibleGeometryBufferIter = caller->shadowMaps.at(index).visibleGeometry.end();
      visibleGeometryBufferIter--;
      (*visibleGeometryBufferIter).aabb = (*visibleGeometryIter)->GetAABB();

      (*visibleGeometryIter)->UnlockSubject();

      visibleGeometryIter++;
    }

    //std::sort(caller->shadowMaps.at(index).visibleGeometry.begin(), caller->shadowMaps.at(index).visibleGeometry.end(), GLLI_SortVertexBufferQueueEntries);


    // all this is way too hardcoded and should somehow heed the camera bounding box better

    // projection matrix
    float left = -65; //65
    float right = 65;
    float bottom = -65;
    float top = 65;

    left = -75;
    right = 75;
    bottom = -75;
    top = 75;

    float nearCap = -150;//-80;
    float farCap = 150;//80;

    Matrix4 proj;
    proj.elements[0] = 2 / (right - left);
    proj.elements[5] = 2 / (top - bottom);
    proj.elements[10] = -2 / (farCap - nearCap);
    proj.elements[3] = -((right + left) / (right - left));
    proj.elements[7] = -((top + bottom) / (top - bottom));
    proj.elements[11] = -((farCap + nearCap) / (farCap - nearCap));
    proj.elements[15] = 1;
    Matrix4 identity = Matrix4(MATRIX4_IDENTITY);
    caller->shadowMaps.at(index).lightProjectionMatrix = identity * proj;

    // view matrix
    Quaternion ident(QUATERNION_IDENTITY);
    Vector3 pos = (caller->GetPosition().GetNormalized(Vector3(0, 0, -1))) + camera->GetDerivedPosition().Get2D() + Vector3(0, 70, 0);
    //Vector3 pos = caller->GetPosition().GetNormalized();
    Quaternion rot; rot = caller->GetPosition().GetNormalized(Vector3(0, 0, -1));
    caller->shadowMaps.at(index).lightViewMatrix.ConstructInverse(pos, Vector3(1, 1, 1), rot);
  }

  ShadowMap GraphicsLight_LightInterpreter::GetShadowMap(const std::string &camName) {
    for (unsigned int i = 0; i < caller->shadowMaps.size(); i++) {
      if (caller->shadowMaps.at(i).cameraName == camName) {
        return caller->shadowMaps.at(i);
      }
    }

    ShadowMap empty;
    return empty;
  }

  void GraphicsLight_LightInterpreter::OnPoke() {
    if (!caller->GetShadow()) return;

    std::vector<ShadowMap>::iterator iter = caller->shadowMaps.begin();

    while (iter != caller->shadowMaps.end()) {
      boost::intrusive_ptr<Renderer3DMessage_RenderShadowMap> renderShadowMap(new Renderer3DMessage_RenderShadowMap(*iter));
      caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D()->messageQueue.PushMessage(renderShadowMap);
      renderShadowMap->Wait();

      (*iter).visibleGeometry.clear();

      iter++;
    }
  }

}
