// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "physics_object.hpp"

#include "physics_scene.hpp"

namespace blunted {

  PhysicsObject::PhysicsObject(PhysicsScene *physicsScene) : physicsScene(physicsScene) {
  }

  PhysicsObject::~PhysicsObject() {
  }

  PhysicsScene *PhysicsObject::GetPhysicsScene() {
    return physicsScene;
  }

}
