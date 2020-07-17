// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SCENE3D_NODE
#define _HPP_SCENE3D_NODE

#include "scene/object.hpp"

#include "types/spatial.hpp"

#include "base/geometry/aabb.hpp"

namespace blunted {

  class Scene3D;

  class Node : public Spatial {

    public:
      Node(const std::string &name);
      Node(const Node &source, const std::string &postfix, boost::shared_ptr<Scene3D> scene3D);
      virtual ~Node();

      virtual void Exit();

      void AddNode(boost::intrusive_ptr<Node> node);
      void DeleteNode(boost::intrusive_ptr<Node> node);
      void GetNodes(std::vector < boost::intrusive_ptr<Node> > &gatherNodes, bool recurse = false) const;
      boost::intrusive_ptr<Node> GetNode(const std::string &name);

      void AddObject(boost::intrusive_ptr<Object> object);
      boost::intrusive_ptr<Object> GetObject(const std::string &name);
      void DeleteObject(const std::string &name, bool exitObject = true);
      void DeleteObject(boost::intrusive_ptr<Object> object, bool exitObject = true);
      void DeleteAllObjects(bool exitObjects = true);
      void RemoveObject(const std::string &name);
      void RemoveObject(boost::intrusive_ptr<Object> object);
      void RemoveAllObjects();

      void GetSpatials(std::list < boost::intrusive_ptr<Spatial> > &gatherSpatials, bool recurse = true, int depth = 0) const;

      void GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, bool recurse = true, int depth = 0) const;
      void GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse = true, int depth = 0) const;

      void GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse = true, int depth = 0) const;

      template <class T>
      inline void GetObjects(e_ObjectType targetObjectType, std::list < boost::intrusive_ptr<T> > &gatherObjects, bool recurse = true, int depth = 0) const {
        objects.Lock();
        int objectsSize = objects.data.size();
        for (int i = 0; i < objectsSize; i++) {
          if (objects.data.at(i)->GetObjectType() == targetObjectType) {
            gatherObjects.push_back(static_pointer_cast<T>(objects.data.at(i)));
          }
        }
        objects.Unlock();

        if (recurse) {
          nodes.Lock();
          int nodesSize = nodes.data.size();
          for (int i = 0; i < nodesSize; i++) {
            nodes.data.at(i)->GetObjects<T>(targetObjectType, gatherObjects, recurse, depth + 1);
          }
          nodes.Unlock();
        }
      }

      template <class T>
      inline void GetObjects(e_ObjectType targetObjectType, std::list < boost::intrusive_ptr<T> > &gatherObjects, const vector_Planes &bounding, bool recurse = true, int depth = 0) const {
        objects.Lock();
        int objectsSize = objects.data.size();
        for (int i = 0; i < objectsSize; i++) {
          if (objects.data.at(i)->GetObjectType() == targetObjectType) {
            if (objects.data.at(i)->GetAABB().Intersects(bounding)) gatherObjects.push_back(static_pointer_cast<T>(objects.data.at(i)));
          }
        }
        objects.Unlock();

        if (recurse) {
          nodes.Lock();
          int nodesSize = nodes.data.size();
          for (int i = 0; i < nodesSize; i++) {
            if (nodes.data.at(i)->GetAABB().Intersects(bounding)) nodes.data.at(i)->GetObjects<T>(targetObjectType, gatherObjects, bounding, recurse, depth + 1);
          }
          nodes.Unlock();
        }
      }

      template <class T>
      inline void GetObjects(e_ObjectType targetObjectType, std::deque < boost::intrusive_ptr<T> > &gatherObjects, bool recurse = true, int depth = 0) const {
        objects.Lock();
        int objectsSize = objects.data.size();
        for (int i = 0; i < objectsSize; i++) {
          if (objects.data.at(i)->GetObjectType() == targetObjectType) {
            gatherObjects.push_back(static_pointer_cast<T>(objects.data.at(i)));
          }
        }
        objects.Unlock();

        if (recurse) {
          nodes.Lock();
          int nodesSize = nodes.data.size();
          for (int i = 0; i < nodesSize; i++) {
            nodes.data.at(i)->GetObjects<T>(targetObjectType, gatherObjects, recurse, depth + 1);
          }
          nodes.Unlock();
        }
      }

      template <class T>
      inline void GetObjects(e_ObjectType targetObjectType, std::deque < boost::intrusive_ptr<T> > &gatherObjects, const vector_Planes &bounding, bool recurse = true, int depth = 0) const {
        objects.Lock();
        int objectsSize = objects.data.size();
        for (int i = 0; i < objectsSize; i++) {
          if (objects.data.at(i)->GetObjectType() == targetObjectType) {
            if (objects.data.at(i)->GetAABB().Intersects(bounding)) gatherObjects.push_back(static_pointer_cast<T>(objects.data.at(i)));
          }
        }
        objects.Unlock();

        if (recurse) {
          nodes.Lock();
          int nodesSize = nodes.data.size();
          for (int i = 0; i < nodesSize; i++) {
            if (nodes.data.at(i)->GetAABB().Intersects(bounding)) nodes.data.at(i)->GetObjects<T>(targetObjectType, gatherObjects, bounding, recurse, depth + 1);
          }
          nodes.Unlock();
        }
      }

      void PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystem);

      void PrintTree(int recursionDepth = 0);
      virtual AABB GetAABB() const;

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

    protected:
      mutable Lockable < std::vector < boost::intrusive_ptr<Node> > > nodes;
      mutable Lockable < std::vector < boost::intrusive_ptr<Object> > > objects;

  };

}

#endif
