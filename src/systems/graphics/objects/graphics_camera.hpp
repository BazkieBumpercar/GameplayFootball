// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GRAPHICSSYSTEM_OBJECT_CAMERA
#define _HPP_GRAPHICSSYSTEM_OBJECT_CAMERA

#include "base/math/vector3.hpp"
#include "scene/objects/camera.hpp"
#include "scene/objects/geometry.hpp"
#include "scene/objects/skybox.hpp"
#include "../graphics_object.hpp"

#include "../rendering/r3d_messages.hpp"

namespace blunted {

  class GraphicsCamera_CameraInterpreter;

  class GraphicsCamera : public GraphicsObject {

    public:
      GraphicsCamera(GraphicsScene *graphicsScene);
      virtual ~GraphicsCamera();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      Vector3 GetPosition() const;
      virtual void SetRotation(const Quaternion &newRotation);
      Quaternion GetRotation() const;

      void SetSize(float x_percent, float y_percent, float width_percent, float height_percent);

      float x_percent, y_percent, width_percent, height_percent;
      int viewID;
      ViewBuffer viewBuffer;
      float fov;
      float nearCap;
      float farCap;

    protected:
      Vector3 position;
      Quaternion rotation;

  };

  class GraphicsCamera_CameraInterpreter : public ICameraInterpreter {

    public:
      GraphicsCamera_CameraInterpreter(GraphicsCamera *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnLoad(const Properties &properties);
      virtual void OnUnload();
      virtual void SetFOV(float fov);
      virtual void SetCapping(float nearCap, float farCap);
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation);
      virtual void EnqueueView(const std::string &camName, std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes);
      virtual void OnPoke();

    protected:
      GraphicsCamera *caller;

  };
}

#endif
