// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT_CAMERA
#define _HPP_OBJECT_CAMERA

#include "defines.hpp"

#include "scene/object.hpp"

#include "types/interpreter.hpp"

#include "base/math/quaternion.hpp"
#include "base/math/vector3.hpp"

namespace blunted {

  class Light;
  class Geometry;
  class Skybox;

  class Camera : public Object {

    public:
      Camera(std::string name);
      virtual ~Camera();

      virtual void Init();
      virtual void Exit();

      virtual void SetFOV(float fov);
      virtual float GetFOV() const { boost::mutex::scoped_lock blah(subjectMutex); return fov; }
      virtual void SetCapping(float nearCap, float farCap);
      virtual void GetCapping(float &nearCap, float &farCap) const { boost::mutex::scoped_lock blah(subjectMutex); nearCap = this->nearCap; farCap = this->farCap; }

      // todo: const lists?
      virtual void EnqueueView(std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes);
      virtual void Poke(e_SystemType targetSystemType);

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

    protected:
      float fov;
      float nearCap;
      float farCap;

  };

  class ICameraInterpreter : public Interpreter {

    public:
      virtual void OnLoad(const Properties &properties) = 0;
      virtual void OnUnload() = 0;
      virtual void SetFOV(float fov) = 0;
      virtual void SetCapping(float nearCap, float farCap) = 0;
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation) = 0;
      // todo: const lists?
      virtual void EnqueueView(const std::string &camName, std::deque < boost::intrusive_ptr<Geometry> > &visibleGeometry, std::deque < boost::intrusive_ptr<Light> > &visibleLights, std::deque < boost::intrusive_ptr<Skybox> > &skyboxes) = 0;
      virtual void OnPoke() = 0;

    protected:

  };

}

#endif
