// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_PHYSICSWRAPPER_ODE
#define _HPP_PHYSICSWRAPPER_ODE

#include "interface_physics.hpp"

#include <ode/ode.h>

namespace blunted {

  static dJointGroupID collisionGroup;

  class OdePhysics : public IPhysicsWrapper {

    public:
      OdePhysics();
      virtual ~OdePhysics();

      // the mother of all functions
      virtual int StepTime(int timediff_ms, int resolution_ms);

      // spacetime stuff
      virtual int CreateWorld();
      virtual void DeleteWorld(int worldID);

      virtual int CreateSpace();
      virtual void DeleteSpace(int spaceID);

      virtual void SetGravity(int worldID, const Vector3 &gravity);
      virtual void SetErrorCorrection(int worldID, float value);
      virtual void SetConstraintForceMixing(int worldID, float value);

      // actor functions
      virtual IPhysicsActor *CreateActor(int worldID);
      virtual void DeleteActor(IPhysicsActor *actor);

      virtual void ActorSetMassSphere(IPhysicsActor *actor, float radius, float weight);
      virtual void ActorSetMassBox(IPhysicsActor *actor, const Vector3 &sides, float weight);
      virtual void ActorSetMassPosition(IPhysicsActor *actor, const Vector3 &pos);

      virtual void ActorSetPosition(IPhysicsActor *actor, const Vector3 &pos);
      virtual void ActorSetRotation(IPhysicsActor *actor, const Quaternion &rot);

      virtual Vector3 ActorGetPosition(IPhysicsActor *actor);
      virtual Quaternion ActorGetRotation(IPhysicsActor *actor);

      virtual float ActorGetVelocity(IPhysicsActor *actor);
      virtual Vector3 ActorGetMovement(IPhysicsActor *actor);

      virtual void ActorApplyForceAtRelativePosition(IPhysicsActor *actor, float force, const Vector3 &direction, const Vector3 &position);

      // collision functions
      virtual IPhysicsCollisionMesh *CreateCollisionPlane(int spaceID, const Vector3 &normal, float d);
      virtual IPhysicsCollisionMesh *CreateCollisionSphere(int spaceID, float radius);
      virtual IPhysicsCollisionMesh *CreateCollisionBox(int spaceID, const Vector3 &sides);
      virtual IPhysicsCollisionMesh *CreateCollisionTriMesh(int spaceID, const float *vertices, int vertexCount);
      virtual void DeleteCollisionMesh(IPhysicsCollisionMesh *mesh);

      virtual void CollisionMeshSetActor(IPhysicsCollisionMesh *mesh, IPhysicsActor *actor);

      virtual void CollisionMeshSetPosition(IPhysicsCollisionMesh *mesh, const Vector3 &pos);
      virtual void CollisionMeshSetRotation(IPhysicsCollisionMesh *mesh, const Quaternion &rot);

      virtual Vector3 CollisionMeshGetPosition(IPhysicsCollisionMesh *mesh);
      virtual Quaternion CollisionMeshGetRotation(IPhysicsCollisionMesh *mesh);

      virtual void CollisionMeshSetData(IPhysicsCollisionMesh *mesh, const Properties *data);

      // joint functions
      virtual IPhysicsJoint *CreateJoint(int worldID, e_JointType jointType, IPhysicsActor *actor1, IPhysicsActor *actor2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2);
      virtual void DeleteJoint(IPhysicsJoint *joint);

      virtual void JointSetStops(IPhysicsJoint *joint, radian lowStop, radian highStop, int paramNum = 1);
      virtual void JointSetVelocity(IPhysicsJoint *joint, float velocity, int paramNum = 1); // desired velocity
      virtual void JointSetMaxForce(IPhysicsJoint *joint, float force, int paramNum = 1); // maximum force to reach the desired velocity
      virtual void JointSetConstraintForceMixing(IPhysicsJoint *joint, float value, int paramNum = 1);
      virtual void JointSetErrorCorrection(IPhysicsJoint *joint, float value, int paramNum = 1);
      virtual void JointSetSuspensionConstraintForceMixing(IPhysicsJoint *joint, float value); // only on hinge2 joints
      virtual void JointSetSuspensionErrorReduction(IPhysicsJoint *joint, float value); // only on hinge2 joints

      virtual float JointGetAngleRate(IPhysicsJoint *joint, int paramNum = 1);

      // utility
      void JointSetParameter(IPhysicsJoint *joint, int parameter, float value, int paramNum = 1);
      dSpaceID GetOdeSpaceID(int spaceID);
      dWorldID GetOdeWorldID(int worldID);

    protected:
      boost::mutex mutex;

      std::map <int, dWorldID> worldMap;
      std::map <int, dSpaceID> spaceMap;

  };

  class OdePhysicsActor : public IPhysicsActor {

    public:
      OdePhysicsActor() {};
      virtual ~OdePhysicsActor() {};

      dBodyID id;

    protected:

  };

  class OdePhysicsCollisionMesh : public IPhysicsCollisionMesh {

    public:
      OdePhysicsCollisionMesh() : vertices(0), indices(0) {};
      virtual ~OdePhysicsCollisionMesh() {
        if (vertices) delete [] vertices;
        if (indices) delete [] indices;
      };

      dGeomID id;

      real *vertices;
      int *indices;

    protected:

  };

  class OdePhysicsJoint : public IPhysicsJoint {

    public:
      OdePhysicsJoint() {};
      virtual ~OdePhysicsJoint() {};

      dJointID id;
      e_JointType jointType;

    protected:

  };

}

#endif
