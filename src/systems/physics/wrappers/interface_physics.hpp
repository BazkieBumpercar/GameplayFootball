// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_PHYSICSWRAPPER
#define _HPP_PHYSICSWRAPPER

#include "defines.hpp"

#include "base/math/vector3.hpp"
#include "base/math/quaternion.hpp"

#include "base/properties.hpp"

namespace blunted {

  enum e_JointType {
    e_JointType_Ball,
    e_JointType_Hinge,
    e_JointType_Slider,
    e_JointType_Contact,
    e_JointType_Universal,
    e_JointType_Hinge2,
    e_JointType_PR,
    e_JointType_PU,
    e_JointType_Piston,
    e_JointType_Fixed,
    e_JointType_AMotor,
    e_JointType_LMotor,
    e_JointType_Plane2D
  };

  class IPhysicsActor;
  class IPhysicsCollisionMesh;
  class IPhysicsJoint;

  class IPhysicsWrapper {

    public:
      virtual ~IPhysicsWrapper() {};

      // the mother of all functions, returns: remainder in ms
      virtual int StepTime(int timediff_ms, int resolution_ms) = 0;

      // spacetime stuff
      virtual int CreateWorld() = 0;
      virtual void DeleteWorld(int worldID) = 0;

      virtual int CreateSpace() = 0;
      virtual void DeleteSpace(int spaceID) = 0;

      virtual void SetGravity(int worldID, const Vector3 &gravity) = 0;
      virtual void SetErrorCorrection(int worldID, float value) = 0;
      virtual void SetConstraintForceMixing(int worldID, float value) = 0;

      // actor functions
      virtual IPhysicsActor *CreateActor(int worldID) = 0;
      virtual void DeleteActor(IPhysicsActor *actor) = 0;

      virtual void ActorSetMassSphere(IPhysicsActor *actor, float radius, float weight) = 0;
      virtual void ActorSetMassBox(IPhysicsActor *actor, const Vector3 &sides, float weight) = 0;
      virtual void ActorSetMassPosition(IPhysicsActor *actor, const Vector3 &pos) = 0;

      virtual void ActorSetPosition(IPhysicsActor *actor, const Vector3 &pos) = 0;
      virtual void ActorSetRotation(IPhysicsActor *actor, const Quaternion &rot) = 0;

      virtual Vector3 ActorGetPosition(IPhysicsActor *actor) = 0;
      virtual Quaternion ActorGetRotation(IPhysicsActor *actor) = 0;

      virtual float ActorGetVelocity(IPhysicsActor *actor) = 0;
      virtual Vector3 ActorGetMovement(IPhysicsActor *actor) = 0;

      virtual void ActorApplyForceAtRelativePosition(IPhysicsActor *actor, float force, const Vector3 &direction, const Vector3 &position) = 0;

      // collision functions
      virtual IPhysicsCollisionMesh *CreateCollisionPlane(int spaceID, const Vector3 &normal, float d) = 0;
      virtual IPhysicsCollisionMesh *CreateCollisionSphere(int spaceID, float radius) = 0;
      virtual IPhysicsCollisionMesh *CreateCollisionBox(int spaceID, const Vector3 &sides) = 0;
      virtual IPhysicsCollisionMesh *CreateCollisionTriMesh(int spaceID, const float *vertices, int vertexCount) = 0;
      virtual void DeleteCollisionMesh(IPhysicsCollisionMesh *mesh) = 0;

      virtual void CollisionMeshSetActor(IPhysicsCollisionMesh *mesh, IPhysicsActor *actor) = 0;

      virtual void CollisionMeshSetPosition(IPhysicsCollisionMesh *mesh, const Vector3 &pos) = 0;
      virtual void CollisionMeshSetRotation(IPhysicsCollisionMesh *mesh, const Quaternion &rot) = 0;

      virtual Vector3 CollisionMeshGetPosition(IPhysicsCollisionMesh *mesh) = 0;
      virtual Quaternion CollisionMeshGetRotation(IPhysicsCollisionMesh *mesh) = 0;

      virtual void CollisionMeshSetData(IPhysicsCollisionMesh *mesh, const Properties *data) = 0;

      // joint functions
      virtual IPhysicsJoint *CreateJoint(int worldID, e_JointType jointType, IPhysicsActor *actor1, IPhysicsActor *actor2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2) = 0;
      virtual void DeleteJoint(IPhysicsJoint *joint) = 0;

      virtual void JointSetStops(IPhysicsJoint *joint, radian lowStop, radian highStop, int paramNum = 1) = 0;
      virtual void JointSetVelocity(IPhysicsJoint *joint, float velocity, int paramNum = 1) = 0; // desired velocity
      virtual void JointSetMaxForce(IPhysicsJoint *joint, float force, int paramNum = 1) = 0; // maximum force to reach the desired velocity
      virtual void JointSetConstraintForceMixing(IPhysicsJoint *joint, float value, int paramNum = 1) = 0;
      virtual void JointSetErrorCorrection(IPhysicsJoint *joint, float value, int paramNum = 1) = 0;
      virtual void JointSetSuspensionConstraintForceMixing(IPhysicsJoint *joint, float value) = 0; // only on hinge2 joints
      virtual void JointSetSuspensionErrorReduction(IPhysicsJoint *joint, float value) = 0; // only on hinge2 joints

      virtual float JointGetAngleRate(IPhysicsJoint *joint, int paramNum = 1) = 0;

    protected:

  };


  // containers for id's

  class IPhysicsActor {

    public:

    protected:

  };

  class IPhysicsCollisionMesh {

    public:

    protected:

  };

  class IPhysicsJoint {

    public:

    protected:

  };

}

#endif
