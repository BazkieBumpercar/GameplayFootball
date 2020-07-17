// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_PHYSICSSYSTEM_OBJECT_JOINT
#define _HPP_PHYSICSSYSTEM_OBJECT_JOINT

#include "base/math/vector3.hpp"
#include "scene/objects/joint.hpp"
#include "../physics_system.hpp"
#include "../physics_object.hpp"
#include "../wrappers/interface_physics.hpp"

namespace blunted {

  class PhysicsJoint_JointInterpreter;

  class PhysicsJoint : public PhysicsObject {

    public:
      PhysicsJoint(PhysicsScene *physicsScene);
      virtual ~PhysicsJoint();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      Vector3 GetPosition() const;
      virtual void SetRotation(const Quaternion &newRotation);
      Quaternion GetRotation() const;

      IPhysicsJoint *joint;
      e_JointType jointType;

    protected:
      Vector3 position;
      Quaternion rotation;

  };

  class PhysicsJoint_JointInterpreter : public IJointInterpreter {

    public:
      PhysicsJoint_JointInterpreter(PhysicsJoint *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Physics; }

      virtual void OnLoad(boost::intrusive_ptr<Joint> joint, boost::intrusive_ptr<Geometry> object1, boost::intrusive_ptr<Geometry> object2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2);
      virtual void OnUnload();
      //virtual void OnChange(boost::shared_ptr < Resource<TriangleMesh> > surface);
      //inline virtual void OnMove(const Vector3 &position);
      //inline virtual void OnRotate(const Quaternion &rotation);

      virtual void SetStops(radian lowStop, radian highStop, int paramNum = 1);
      // desired velocity
      virtual void SetVelocity(float velocity, int paramNum = 1);
      // maximum force to reach the desired velocity
      virtual void SetMaxForce(float force, int paramNum = 1);
      virtual void SetConstraintForceMixing(float value, int paramNum = 1);
      virtual void SetErrorCorrection(float value, int paramNum = 1);
      // only on hinge2 joints
      virtual void SetSuspensionConstraintForceMixing(float value);
      virtual void SetSuspensionErrorReduction(float value);

      inline virtual void OnPoke();

    protected:
      PhysicsJoint *caller;
      IPhysicsWrapper *physics;

  };
}

#endif
