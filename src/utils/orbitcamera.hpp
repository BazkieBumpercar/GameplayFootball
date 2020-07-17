// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _UTILS_ORBITCAMERA
#define _UTILS_ORBITCAMERA

#include "types/command.hpp"
#include "types/iusertask.hpp"

#include "scene/objects/camera.hpp"
#include "scene/scene3d/scene3d.hpp"
#include "scene/objects/light.hpp"

namespace blunted {

  class OrbitCamera : public IUserTask {

    public:
      OrbitCamera(boost::shared_ptr<Scene3D> scene3D, const Vector3 &position);
      virtual ~OrbitCamera();

      boost::intrusive_ptr<Node> GetCameraNode();

      virtual void GetPhase();
      virtual void ProcessPhase();
      virtual void PutPhase();

    protected:
      boost::intrusive_ptr<Camera> orbitCamera;
      boost::intrusive_ptr<Light> light;
      boost::intrusive_ptr<Light> light2;
      boost::intrusive_ptr<Node> orbitCameraNode;

      radian mouseVert, mouseHoriz;

      Vector3 orbitCameraPosition;
      Quaternion newRotation;
      Vector3 newPosition;

      float time;
      float prevTime;

  };

}

#endif
