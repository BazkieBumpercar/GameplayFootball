// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_PHYSICSSYSTEM_OBJECT_GEOMETRY
#define _HPP_PHYSICSSYSTEM_OBJECT_GEOMETRY

#include "base/math/vector3.hpp"
#include "scene/objects/geometry.hpp"
#include "../physics_system.hpp"
#include "../physics_object.hpp"
#include "../wrappers/interface_physics.hpp"

namespace blunted {

  class PhysicsGeometry_GeometryInterpreter;

  class PhysicsGeometry : public PhysicsObject {

    public:
      PhysicsGeometry(PhysicsScene *physicsScene);
      virtual ~PhysicsGeometry();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      Vector3 GetPosition() const;
      virtual void SetRotation(const Quaternion &newRotation);
      Quaternion GetRotation() const;

      //std::vector < boost::shared_ptr<VertexBufferIndex> > vertexBufferIndices;
      //boost::shared_ptr < Resource<VertexBuffer> > vertexBuffer;

      IPhysicsActor *actor;
      IPhysicsCollisionMesh *mesh;
      bool movable;

    protected:
      Vector3 position;
      Quaternion rotation;

  };

  class PhysicsGeometry_GeometryInterpreter : public IGeometryInterpreter {

    public:
      PhysicsGeometry_GeometryInterpreter(PhysicsGeometry *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Physics; }

      virtual void OnLoad(boost::intrusive_ptr<Geometry> geometry);
      virtual void OnUpdateGeometry(boost::intrusive_ptr<Geometry> geometry, bool updateMaterials) {}; // todo!
      virtual void OnUnload();
      inline virtual void OnMove(const Vector3 &position);
      inline virtual void OnRotate(const Quaternion &rotation);
      inline virtual void OnPoke();

      virtual void ApplyForceAtRelativePosition(float force, const Vector3 &direction, const Vector3 &position);

      virtual IPhysicsActor *GetActor();

    protected:
      PhysicsGeometry *caller;

  };
}

#endif
