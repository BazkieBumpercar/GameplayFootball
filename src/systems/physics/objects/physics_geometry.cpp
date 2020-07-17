// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "physics_geometry.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "../physics_scene.hpp"

namespace blunted {

  PhysicsGeometry::PhysicsGeometry(PhysicsScene *physicsScene) : PhysicsObject(physicsScene), actor(0), mesh(0) {
  }

  PhysicsGeometry::~PhysicsGeometry() {
  }

  boost::intrusive_ptr<Interpreter> PhysicsGeometry::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Geometry) {
      boost::intrusive_ptr<PhysicsGeometry_GeometryInterpreter> geometryInterpreter(new PhysicsGeometry_GeometryInterpreter(this));
      return geometryInterpreter;
    }
    Log(e_FatalError, "PhysicsGeometry", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<PhysicsGeometry_GeometryInterpreter>();
  }

  void PhysicsGeometry::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 PhysicsGeometry::GetPosition() const {
    return position;
  }

  void PhysicsGeometry::SetRotation(const Quaternion &newRotation) {
    rotation = newRotation;
  }

  Quaternion PhysicsGeometry::GetRotation() const {
    return rotation;
  }


  // geometry interpreter

  PhysicsGeometry_GeometryInterpreter::PhysicsGeometry_GeometryInterpreter(PhysicsGeometry *caller) : caller(caller) {
  }

  void PhysicsGeometry_GeometryInterpreter::OnLoad(boost::intrusive_ptr<Geometry> geometry) {
    boost::intrusive_ptr < Resource<GeometryData> > resource = geometry->GetGeometryData();

    IPhysicsWrapper *physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();


    // actor

    bool movable = geometry->GetProperties().GetBool("movable");
    if (movable == true) {
      caller->actor = physics->CreateActor(caller->GetPhysicsScene()->worldID);
    }


    // collision mesh

    if (geometry->GetProperty("physics_collision_type") == "box") {
      float x = geometry->GetProperties().GetReal("physics_collision_box_x");
      float y = geometry->GetProperties().GetReal("physics_collision_box_y");
      float z = geometry->GetProperties().GetReal("physics_collision_box_z");
      caller->mesh = physics->CreateCollisionBox(caller->GetPhysicsScene()->spaceID, Vector3(x, y, z));

      if (movable == true) physics->ActorSetMassBox(caller->actor, Vector3(x, y, z), geometry->GetProperties().GetReal("mass"));
    }

    else if (geometry->GetProperty("physics_collision_type") == "sphere") {
      float radius = geometry->GetProperties().GetReal("physics_collision_sphere_radius");
      caller->mesh = physics->CreateCollisionSphere(caller->GetPhysicsScene()->spaceID, radius);

      if (movable == true) physics->ActorSetMassSphere(caller->actor, radius, geometry->GetProperties().GetReal("mass"));
    }

    else if (geometry->GetProperty("physics_collision_type") == "plane") {
      float x = geometry->GetProperties().GetReal("physics_collision_plane_normal_x");
      float y = geometry->GetProperties().GetReal("physics_collision_plane_normal_y");
      float z = geometry->GetProperties().GetReal("physics_collision_plane_normal_z");
      float d = geometry->GetProperties().GetReal("physics_collision_plane_d");
      caller->mesh = physics->CreateCollisionPlane(caller->GetPhysicsScene()->spaceID, Vector3(x, y, z), d);

      // planes don't move, so no actor
      if (caller->actor) Log(e_FatalError, "PhysicsGeometry_GeometryInterpreter", "OnLoad", "Planes are unable to move (no pun intended :p)");
    }

    else if (geometry->GetProperty("physics_collision_type") == "mesh") {
/* TODO
      std::vector < MaterializedTriangleMesh > triangleMeshes = resource->GetResource()->GetTriangleMeshes();
      std::vector<Triangle*> triangles;

      int startIndex = 0;
      int currentSize = 0;
      Vector3 pos = caller->GetPosition();

      for (int i = 0; i < (signed int)triangleMeshes.size(); i++) {
        // mesh
        std::vector<Triangle*> subTriangles = triangleMeshes.at(i).triangleMesh->GetTriangles();
        startIndex += currentSize;
        currentSize = 0;
        for (int t = 0; t < (signed int)subTriangles.size(); t++) {
          triangles.push_back(subTriangles.at(t));
          //triangles.at(t)->Translate(pos.coords[0], pos.coords[1], pos.coords[2]);
        }
        currentSize = subTriangles.size();
      }

      // put vertex data into array
      float *vertices = new float[triangles.size() * 3 * 3];

      std::vector<Triangle*>::iterator tris_iter = triangles.begin();
      int counter = 0;
      while (tris_iter != triangles.end()) {
        for (int c = 0; c < 3; c++) {
          vertices[counter * 9 + c * 3]     = (*tris_iter)->GetVertex(c).coords[0];
          vertices[counter * 9 + c * 3 + 1] = (*tris_iter)->GetVertex(c).coords[1];
          vertices[counter * 9 + c * 3 + 2] = (*tris_iter)->GetVertex(c).coords[2];
        }
        tris_iter++;
        counter++;
      }

      // build collision mesh
      caller->mesh = physics->CreateCollisionTriMesh(caller->GetPhysicsScene()->spaceID, vertices, triangles.size() * 3);

      delete [] vertices;

      //if (movable == true) physics->ActorSetMassTriMesh(caller->actor, radius, geometry->GetProperties().GetReal("weight"));
*/
    }

    else {
      Log(e_FatalError, "PhysicsGeometry_GeometryInterpreter", "OnLoad", "You need to set the physics_collision_type in object '" + geometry->GetName() + "'s properties");
    }

    if (movable == true) {
      physics->ActorSetPosition(caller->actor, geometry->GetPosition());
      physics->ActorSetRotation(caller->actor, geometry->GetRotation());
      physics->CollisionMeshSetActor(caller->mesh, caller->actor);

      // move center of mass?
      if (geometry->PropertyExists("center-of-mass")) {
        Vector3 centerOfMass = GetVectorFromString(geometry->GetProperty("center-of-mass"));
        physics->ActorSetMassPosition(caller->actor, centerOfMass);
      }

    } else {
      //caller->GetPosition().Print();
      physics->CollisionMeshSetPosition(caller->mesh, geometry->GetDerivedPosition());
      physics->CollisionMeshSetRotation(caller->mesh, geometry->GetDerivedRotation());
    }

    caller->movable = movable;

    physics->CollisionMeshSetData(caller->mesh, &geometry->GetProperties());

  }
  
  void PhysicsGeometry_GeometryInterpreter::OnUnload() {
    IPhysicsWrapper *physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();

    if (caller->mesh) physics->DeleteCollisionMesh(caller->mesh);
    if (caller->actor) physics->DeleteActor(caller->actor);
    delete caller;
    caller = 0;
  }

  void PhysicsGeometry_GeometryInterpreter::OnMove(const Vector3 &position) {
    caller->SetPosition(position);

    IPhysicsWrapper *physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();

    if (caller->movable == true) {
      if (caller->actor) physics->ActorSetPosition(caller->actor, position);
    } else {
      if (caller->mesh) physics->CollisionMeshSetPosition(caller->mesh, position);
    }
  }

  void PhysicsGeometry_GeometryInterpreter::OnRotate(const Quaternion &rotation) {
    caller->SetRotation(rotation);
    IPhysicsWrapper *physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();

    if (caller->movable == true) {
      //if (caller->actor) physics->ActorSetRotation(caller->actor, rotation);
    } else {
      if (caller->mesh) physics->CollisionMeshSetRotation(caller->mesh, rotation);
    }
  }

  void PhysicsGeometry_GeometryInterpreter::OnPoke() {
    IPhysicsWrapper *physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();

    if (caller->actor) {
      Geometry *subject = static_cast<Geometry*>(subjectPtr);

      // todo: locking the physics engine by querying it might have to block on the physics' steptime update
      // so cache the positions/rotations after steptime
      // edit: maybe they are serial anyway, so this might be untrue
      Vector3 pos = physics->CollisionMeshGetPosition(caller->mesh);
      Quaternion rot = physics->ActorGetRotation(caller->actor);
      subject->SetPosition(pos, false); // false == don't update spatialdata to system objects; would cause a deadlock
      subject->SetRotation(rot, false);
      MustUpdateSpatialData update;
      update.haveTo = true;
      update.excludeSystem = e_SystemType_Physics;
      subject->updateSpatialDataAfterPoke.SetData(update);

      if (subject->RequestPropertyExists("velocity")) {
        subject->SetRequestProperty("velocity", real_to_str(physics->ActorGetVelocity(caller->actor)).c_str());
      }
      if (subject->RequestPropertyExists("movement")) {
        Vector3 movement = physics->ActorGetMovement(caller->actor);
        subject->SetRequestProperty("movement", (real_to_str(movement.coords[0]) + "," + real_to_str(movement.coords[1]) + "," + real_to_str(movement.coords[2])).c_str());
      }

    }

  }

  void PhysicsGeometry_GeometryInterpreter::ApplyForceAtRelativePosition(float force, const Vector3 &direction, const Vector3 &position) {
    IPhysicsWrapper *physics = caller->GetPhysicsScene()->GetPhysicsSystem()->GetPhysicsWrapper();

    assert(caller->actor);
    physics->ActorApplyForceAtRelativePosition(caller->actor, force, direction, position);
  }

  IPhysicsActor *PhysicsGeometry_GeometryInterpreter::GetActor() {
    return caller->actor;
  }

}
