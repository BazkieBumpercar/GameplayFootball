// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SCENE3D
#define _HPP_SCENE3D

#include "defines.hpp"

#include "node.hpp"

#include "../scene.hpp"
#include "../object.hpp"
#include "types/lockable.hpp"

#include "base/utils.hpp"

#include "base/geometry/plane.hpp"

namespace blunted {

  class Scene3D : public Scene {

    public:
      Scene3D(std::string name);
      virtual ~Scene3D();

      virtual void Init();
      virtual void Exit(); // ATOMIC

      void PrintTree();

      void AddNode(boost::intrusive_ptr<Node> node);
      void DeleteNode(boost::intrusive_ptr<Node> node);
      void AddObject(boost::intrusive_ptr<Object> object);
      void DeleteObject(const std::string &name);
      void DeleteObject(boost::intrusive_ptr<Object> object);

      void SetGravity(const Vector3 &gravity);
      void SetErrorCorrection(float value);
      void SetConstraintForceMixing(float value);

      void GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects) const {
        hierarchyRoot->GetObjects(gatherObjects, true, 0);
      }

      void GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding) const {
        hierarchyRoot->GetObjects(gatherObjects, bounding, true, 0);
      }

      void GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding) const {
        hierarchyRoot->GetObjects(gatherObjects, bounding, true, 0);
      }

      template <class T>
      void GetObjects(e_ObjectType targetObjectType, std::list < boost::intrusive_ptr<T> > &gatherObjects) const {
        if (!SupportedObjectType(targetObjectType)) {
          Log(e_Error, "Scene3D", "GetObjects", "targetObjectType " + int_to_str(targetObjectType) + " is not supported by this scene");
          return;
        }

        hierarchyRoot->GetObjects<T>(targetObjectType, gatherObjects, true, 0);
      }

      template <class T>
      void GetObjects(e_ObjectType targetObjectType, std::list < boost::intrusive_ptr<T> > &gatherObjects, const vector_Planes &bounding) const {
        if (!SupportedObjectType(targetObjectType)) {
          Log(e_Error, "Scene3D", "GetObjects", "targetObjectType " + int_to_str(targetObjectType) + " is not supported by this scene");
          return;
        }

        hierarchyRoot->GetObjects<T>(targetObjectType, gatherObjects, bounding, true, 0);
      }

      template <class T>
      void GetObjects(e_ObjectType targetObjectType, std::deque < boost::intrusive_ptr<T> > &gatherObjects) const {
        if (!SupportedObjectType(targetObjectType)) {
          Log(e_Error, "Scene3D", "GetObjects", "targetObjectType " + int_to_str(targetObjectType) + " is not supported by this scene");
          return;
        }

        hierarchyRoot->GetObjects<T>(targetObjectType, gatherObjects, true, 0);
      }

      template <class T>
      void GetObjects(e_ObjectType targetObjectType, std::deque < boost::intrusive_ptr<T> > &gatherObjects, const vector_Planes &bounding) const {
        if (!SupportedObjectType(targetObjectType)) {
          Log(e_Error, "Scene3D", "GetObjects", "targetObjectType " + int_to_str(targetObjectType) + " is not supported by this scene");
          return;
        }

        hierarchyRoot->GetObjects<T>(targetObjectType, gatherObjects, bounding, true, 0);
      }

      void PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType);

    protected:
      boost::intrusive_ptr<Node> hierarchyRoot;

  };

  class IScene3DInterpreter : public ISceneInterpreter {

    public:
      virtual void OnLoad() = 0;
      virtual void OnUnload() = 0;

      virtual void SetGravity(const Vector3 &gravity) = 0;
      virtual void SetErrorCorrection(float value) = 0;
      virtual void SetConstraintForceMixing(float value) = 0;

    protected:

  };

}

#endif
