// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT_JOINT
#define _HPP_OBJECT_JOINT

#include "defines.hpp"
#include "scene/object.hpp"
#include "types/interpreter.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/vector3.hpp"

namespace blunted {

  class Geometry;

  class Joint : public Object {

    public:
      Joint(std::string name);
      virtual ~Joint();

      virtual void Exit();

      virtual void Connect(boost::intrusive_ptr<Geometry> object1, boost::intrusive_ptr<Geometry> object2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2);

      virtual void SetStops(radian lowStop, radian highStop, int paramNum = 1);
      // desired velocity
      virtual void SetVelocity(float velocity, int paramNum = 1);
      // maximum force to reach the desired velocity
      virtual void SetMaxForce(float velocity, int paramNum = 1);
      virtual void SetConstraintForceMixing(float value, int paramNum = 1);
      virtual void SetErrorCorrection(float value, int paramNum = 1);
      // only on hinge2 joints
      virtual void SetSuspensionConstraintForceMixing(float value);
      virtual void SetSuspensionErrorReduction(float value);

      virtual void Poke(e_SystemType targetSystemType);

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

    protected:

  };

  class IJointInterpreter : public Interpreter {

    public:
      virtual void OnLoad(boost::intrusive_ptr<Joint> joint, boost::intrusive_ptr<Geometry> object1, boost::intrusive_ptr<Geometry> object2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2) = 0;
      virtual void OnUnload() = 0;

      virtual void SetStops(radian lowStop, radian highStop, int paramNum = 1) = 0;
      // desired velocity
      virtual void SetVelocity(float velocity, int paramNum = 1) = 0;
      // maximum force to reach the desired velocity
      virtual void SetMaxForce(float force, int paramNum = 1) = 0;
      virtual void SetConstraintForceMixing(float value, int paramNum = 1) = 0;
      virtual void SetErrorCorrection(float value, int paramNum = 1) = 0;
      // only on hinge2 joints
      virtual void SetSuspensionConstraintForceMixing(float value) = 0;
      virtual void SetSuspensionErrorReduction(float value) = 0;

      virtual void OnPoke() = 0;

    protected:

  };

}

#endif
