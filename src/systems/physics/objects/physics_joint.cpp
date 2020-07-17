// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "physics_joint.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "../physics_scene.hpp"

#include "physics_geometry.hpp"

namespace blunted {

  PhysicsJoint::PhysicsJoint(PhysicsScene *physicsScene) : PhysicsObject(physicsScene), joint(0) {
  }

  PhysicsJoint::~PhysicsJoint() {
  }

  boost::intrusive_ptr<Interpreter> PhysicsJoint::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Joint) {
      boost::intrusive_ptr<PhysicsJoint_JointInterpreter> jointInterpreter(new PhysicsJoint_JointInterpreter(this));
      return jointInterpreter;
    }
    Log(e_FatalError, "PhysicsJoint", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<PhysicsJoint_JointInterpreter>();
  }

  void PhysicsJoint::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 PhysicsJoint::GetPosition() const {
    return position;
  }

  void PhysicsJoint::SetRotation(const Quaternion &newRotation) {
    rotation = newRotation;
  }

  Quaternion PhysicsJoint::GetRotation() const {
    return rotation;
  }


  // joint interpreter

  PhysicsJoint_JointInterpreter::PhysicsJoint_JointInterpreter(PhysicsJoint *caller) : caller(caller) {
    physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();
  }

  void PhysicsJoint_JointInterpreter::OnLoad(boost::intrusive_ptr<Joint> joint, boost::intrusive_ptr<Geometry> object1, boost::intrusive_ptr<Geometry> object2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2) {
    // find actors
    boost::intrusive_ptr<PhysicsGeometry_GeometryInterpreter> interpreter = static_pointer_cast<PhysicsGeometry_GeometryInterpreter>(object1->GetInterpreter(e_SystemType_Physics));
    object1->LockSubject();
    IPhysicsActor *actor1 = interpreter->GetActor();
    object1->UnlockSubject();
    // todo: what if object1 is exited right now? wouldn't actor1 be invalidated?
    // hmm... well probably the game programmer is responsible for not deleting his objects while joint'ing them

    interpreter = static_pointer_cast<PhysicsGeometry_GeometryInterpreter>(object2->GetInterpreter(e_SystemType_Physics));
    object2->LockSubject();
    IPhysicsActor *actor2 = interpreter->GetActor();
    object2->UnlockSubject();

    std::string jointTypeStr = joint->GetProperty("joint_type");

    if      (jointTypeStr == "hinge")  caller->jointType = e_JointType_Hinge;
    else if (jointTypeStr == "hinge2") caller->jointType = e_JointType_Hinge2;
    else Log(e_FatalError, "PhysicsJoint_JointInterpreter", "OnLoad", "JointType " + int_to_str(caller->jointType) + " not supported");

    caller->joint = physics->CreateJoint(caller->GetPhysicsScene()->worldID, caller->jointType, actor1, actor2, anchor, axis1, axis2);
  }

  void PhysicsJoint_JointInterpreter::OnUnload() {
    if (caller->joint) physics->DeleteJoint(caller->joint);
    delete caller;
    caller = 0;
  }

/*
  void PhysicsJoint_JointInterpreter::OnMove(const Vector3 &position) {
    //caller->SetPosition(position);
  }

  void PhysicsJoint_JointInterpreter::OnRotate(const Quaternion &rotation) {
    //caller->SetRotation(rotation);
  }
*/

  void PhysicsJoint_JointInterpreter::SetStops(radian lowStop, radian highStop, int paramNum) {
    physics->JointSetStops(caller->joint, lowStop, highStop, paramNum);
  }

  // desired velocity
  void PhysicsJoint_JointInterpreter::SetVelocity(float velocity, int paramNum) {
    physics->JointSetVelocity(caller->joint, velocity, paramNum);
  }

  // maximum force to reach the desired velocity
  void PhysicsJoint_JointInterpreter::SetMaxForce(float force, int paramNum) {
    physics->JointSetMaxForce(caller->joint, force, paramNum);
  }

  void PhysicsJoint_JointInterpreter::SetConstraintForceMixing(float value, int paramNum) {
    physics->JointSetConstraintForceMixing(caller->joint, value, paramNum);
  }

  void PhysicsJoint_JointInterpreter::SetErrorCorrection(float value, int paramNum) {
    physics->JointSetErrorCorrection(caller->joint, value, paramNum);
  }

  // only on hinge2 joints
  void PhysicsJoint_JointInterpreter::SetSuspensionConstraintForceMixing(float value) {
    physics->JointSetSuspensionConstraintForceMixing(caller->joint, value);
  }

  void PhysicsJoint_JointInterpreter::SetSuspensionErrorReduction(float value) {
    physics->JointSetSuspensionErrorReduction(caller->joint, value);
  }

  void PhysicsJoint_JointInterpreter::OnPoke() {

    if (caller->joint) {

      Joint *subject = static_cast<Joint*>(subjectPtr);
      if (subject->RequestPropertyExists("angle1_rate")) {
        subject->SetRequestProperty("angle1_rate", real_to_str(physics->JointGetAngleRate(caller->joint, 1)).c_str());
      }
      if (subject->RequestPropertyExists("angle2_rate")) {
        subject->SetRequestProperty("angle2_rate", real_to_str(physics->JointGetAngleRate(caller->joint, 2)).c_str());
      }

    }

  }

}
