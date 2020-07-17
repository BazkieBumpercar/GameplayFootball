// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "physics_scene.hpp"
#include "physics_system.hpp"
#include "physics_object.hpp"

#include "managers/systemmanager.hpp"

#include "wrappers/interface_physics.hpp"

#include "scene/objects/geometry.hpp"
#include "scene/objects/joint.hpp"

#include "objects/physics_geometry.hpp"
#include "objects/physics_joint.hpp"

namespace blunted {

  PhysicsScene::PhysicsScene(PhysicsSystem *physicsSystem) : physicsSystem(physicsSystem) {
    physicsWrapper = physicsSystem->GetPhysicsWrapper();
  }

  PhysicsScene::~PhysicsScene() {
  }

  ISystemObject *PhysicsScene::CreateSystemObject(boost::intrusive_ptr<Object> object) {
    assert(object.get());

    // todo: system scene shouldn't know about object types, i guess? maybe via a Command
    if (object->GetObjectType() == e_ObjectType_Geometry) {
      if (boost::static_pointer_cast<Geometry>(object)->GetProperties().GetBool("physicable") == true) {
        PhysicsGeometry *physicsObject = new PhysicsGeometry(this);
        // physics system objects need to feedback to objects, so pass it as user data
        object->Attach(physicsObject->GetInterpreter(e_ObjectType_Geometry), object.get());
        return physicsObject;
      }
    }

    else if (object->GetObjectType() == e_ObjectType_Joint) {
      PhysicsJoint *physicsObject = new PhysicsJoint(this);
      // physics system objects need to feedback to objects, so pass it as user data
      object->Attach(physicsObject->GetInterpreter(e_ObjectType_Joint), object.get());
      return physicsObject;
    }

    return NULL;
  }

  boost::intrusive_ptr<ISceneInterpreter> PhysicsScene::GetInterpreter(e_SceneType sceneType) {
    if (sceneType == e_SceneType_Scene3D) {
      boost::intrusive_ptr<PhysicsScene_Scene3DInterpreter> scene3DInterpreter(new PhysicsScene_Scene3DInterpreter(this));
      return scene3DInterpreter;
    }
    Log(e_FatalError, "PhysicsScene", "GetInterpreter", "No appropriate interpreter found for this SceneType");
    return boost::intrusive_ptr<PhysicsScene_Scene3DInterpreter>();
  }

  PhysicsSystem *PhysicsScene::GetPhysicsSystem() {
    return physicsSystem;
  }

  IPhysicsWrapper *PhysicsScene::GetPhysicsWrapper() {
    return physicsWrapper;
  }


  // Scene3D interpreter

  PhysicsScene_Scene3DInterpreter::PhysicsScene_Scene3DInterpreter(PhysicsScene *caller) : caller(caller) {
  }

  void PhysicsScene_Scene3DInterpreter::OnLoad() {
    // world
    caller->worldID = caller->GetPhysicsWrapper()->CreateWorld();

    // set some default values
    caller->GetPhysicsWrapper()->SetErrorCorrection(caller->worldID, 0.2);
    caller->GetPhysicsWrapper()->SetConstraintForceMixing(caller->worldID, 1e-5f);
    caller->GetPhysicsWrapper()->SetGravity(caller->worldID, Vector3(0, 0, -9.81));

    // space
    caller->spaceID = caller->GetPhysicsWrapper()->CreateSpace();
  }

  void PhysicsScene_Scene3DInterpreter::OnUnload() {
    caller->GetPhysicsWrapper()->DeleteSpace(caller->spaceID);
    caller->GetPhysicsWrapper()->DeleteWorld(caller->worldID);
    delete caller;
    caller = 0;
  }

  ISystemObject *PhysicsScene_Scene3DInterpreter::CreateSystemObject(boost::intrusive_ptr<Object> object) {
    return caller->CreateSystemObject(object);
  }

  void PhysicsScene_Scene3DInterpreter::SetGravity(const Vector3 &gravity) {
    caller->GetPhysicsWrapper()->SetGravity(caller->worldID, gravity);
  }

  void PhysicsScene_Scene3DInterpreter::SetErrorCorrection(float value) {
    caller->GetPhysicsWrapper()->SetErrorCorrection(caller->worldID, value);
  }

  void PhysicsScene_Scene3DInterpreter::SetConstraintForceMixing(float value) {
    caller->GetPhysicsWrapper()->SetConstraintForceMixing(caller->worldID, value);
  }

}
