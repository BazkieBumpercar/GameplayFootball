// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "graphics_camera.hpp"

#include "systems/graphics/rendering/r3d_messages.hpp"

#include "../graphics_scene.hpp"
#include "../graphics_system.hpp"

#include "graphics_geometry.hpp"
#include "graphics_light.hpp"

namespace blunted {

  GraphicsCamera::GraphicsCamera(GraphicsScene *graphicsScene) : GraphicsObject(graphicsScene) {
    fov = 45;
  }

  GraphicsCamera::~GraphicsCamera() {
  }

  boost::intrusive_ptr<Interpreter> GraphicsCamera::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Camera) {
      boost::intrusive_ptr<GraphicsCamera_CameraInterpreter> CameraInterpreter(new GraphicsCamera_CameraInterpreter(this));
      return CameraInterpreter;
    }
    Log(e_FatalError, "GraphicsCamera", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<GraphicsCamera_CameraInterpreter>();
  }

  void GraphicsCamera::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 GraphicsCamera::GetPosition() const {
    return position;
  }

  void GraphicsCamera::SetRotation(const Quaternion &newRotation) {
    rotation = newRotation;
  }

  Quaternion GraphicsCamera::GetRotation() const {
    return rotation;
  }

  void GraphicsCamera::SetSize(float x_percent, float y_percent, float width_percent, float height_percent) {
    this->x_percent = x_percent;
    this->y_percent = y_percent;
    this->width_percent = width_percent;
    this->height_percent = height_percent;
  }


  GraphicsCamera_CameraInterpreter::GraphicsCamera_CameraInterpreter(GraphicsCamera *caller) : caller(caller) {
  }

  void GraphicsCamera_CameraInterpreter::OnLoad(const Properties &properties) {
    caller->SetSize(properties.GetReal("x_percent", 0), properties.GetReal("y_percent", 0), properties.GetReal("width_percent", 100), properties.GetReal("height_percent", 100));

    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    boost::intrusive_ptr<Renderer3DMessage_CreateView> createView(new Renderer3DMessage_CreateView(caller->x_percent, caller->y_percent, caller->width_percent, caller->height_percent));
    renderer3D->messageQueue.PushMessage(createView);
    createView->Wait();
    caller->viewID = createView->viewID;
  }

  void GraphicsCamera_CameraInterpreter::OnUnload() {
    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    boost::intrusive_ptr<Renderer3DMessage_DeleteView> deleteView(new Renderer3DMessage_DeleteView(caller->viewID));
    renderer3D->messageQueue.PushMessage(deleteView);
    deleteView->Wait();

    delete caller;
    caller = 0;
  }

  void GraphicsCamera_CameraInterpreter::SetFOV(float fov) {
    caller->fov = fov;
  }

  void GraphicsCamera_CameraInterpreter::SetCapping(float nearCap, float farCap) {
    caller->nearCap = nearCap;
    caller->farCap = farCap;
  }

  void GraphicsCamera_CameraInterpreter::OnSpatialChange(const Vector3 &position, const Quaternion &rotation) {
    caller->SetPosition(position);
    caller->SetRotation(rotation);
  }

  void GraphicsCamera_CameraInterpreter::EnqueueView(const std::string &camName, std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes) {

    ViewBuffer *buffer = &caller->viewBuffer;


    // geometry

    std::deque < boost::intrusive_ptr<Geometry> >::iterator visibleGeometryIter = visibleGeometry.begin();
    while (visibleGeometryIter != visibleGeometry.end()) {
      boost::intrusive_ptr<GraphicsGeometry_GeometryInterpreter> interpreter = static_pointer_cast<GraphicsGeometry_GeometryInterpreter>((*visibleGeometryIter)->GetInterpreter(e_SystemType_Graphics));

      (*visibleGeometryIter)->LockSubject();

      // add buffers to visible geometry queue
      interpreter->GetVertexBufferQueue(buffer->visibleGeometry);

      (*visibleGeometryIter)->UnlockSubject();

      std::deque<VertexBufferQueueEntry>::iterator visibleGeometryBufferIter = buffer->visibleGeometry.end();
      visibleGeometryBufferIter--;
      (*visibleGeometryBufferIter).aabb = (*visibleGeometryIter)->GetAABB();

      visibleGeometryIter++;
    }


    // lights

    std::deque < boost::intrusive_ptr<Light> >::iterator visibleLightIter = visibleLights.begin();
    while (visibleLightIter != visibleLights.end()) {
      LightQueueEntry entry;

      boost::intrusive_ptr<GraphicsLight_LightInterpreter> interpreter = static_pointer_cast<GraphicsLight_LightInterpreter>((*visibleLightIter)->GetInterpreter(e_SystemType_Graphics));

      (*visibleLightIter)->LockSubject();
      if (interpreter->GetShadow()) {
        ShadowMap shadowMap = interpreter->GetShadowMap(camName);
        if (shadowMap.cameraName != "") {
          entry.shadowMapTexture = shadowMap.texture;
          entry.lightProjectionMatrix = shadowMap.lightProjectionMatrix;
          entry.lightViewMatrix = shadowMap.lightViewMatrix;
          entry.hasShadow = true;
        } else {
          entry.hasShadow = false;
        }
      } else {
        entry.hasShadow = false;
      }
      (*visibleLightIter)->UnlockSubject();

      // todo: less locking
      entry.position = (*visibleLightIter)->GetDerivedPosition();
      entry.type = (*visibleLightIter)->GetType() == e_LightType_Directional ? 0 : 1;
      entry.shadow = (*visibleLightIter)->GetShadow();
      entry.color = (*visibleLightIter)->GetColor();
      entry.radius = (*visibleLightIter)->GetRadius();
      entry.aabb = (*visibleLightIter)->GetAABB();
      buffer->visibleLights.push_back(entry);

      visibleLightIter++;
    }


    // skyboxes

    std::deque < boost::intrusive_ptr<Skybox> >::iterator skyboxIter = skyboxes.begin();
    while (skyboxIter != skyboxes.end()) {
      boost::intrusive_ptr<GraphicsGeometry_SkyboxInterpreter> interpreter = static_pointer_cast<GraphicsGeometry_SkyboxInterpreter>((*skyboxIter)->GetInterpreter(e_SystemType_Graphics));

      (*skyboxIter)->LockSubject();

      // add buffers to skybox queue
      interpreter->GetVertexBufferQueue(buffer->skyboxes);

      (*skyboxIter)->UnlockSubject();

      skyboxIter++;
    }


    // camera matrix

    buffer->cameraMatrix.ConstructInverse(caller->GetPosition(), Vector3(1, 1, 1), caller->GetRotation());
    //buffer->projectionMatrix.ConstructProjection(caller->fov, aspect, caller->nearCap, caller->farCap);
    buffer->cameraFOV = caller->fov;
    buffer->cameraNearCap = caller->nearCap;
    buffer->cameraFarCap = caller->farCap;
  }

  void GraphicsCamera_CameraInterpreter::OnPoke() {

    boost::intrusive_ptr<Renderer3DMessage_RenderView> renderView(new Renderer3DMessage_RenderView(caller->viewID, caller->viewBuffer));
    caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D()->messageQueue.PushMessage(renderView, true);

    // why do we have to wait? crashes otherwise.. todo: find out why!
    renderView->Wait();
  }

}
