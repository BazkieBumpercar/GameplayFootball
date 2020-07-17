// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "node.hpp"

#include "scene3d.hpp"

#include "scene/objectfactory.hpp"

namespace blunted {

  Node::Node(const std::string &name) : Spatial(name) {
    aabb.data.aabb.Reset();
    aabb.data.dirty = false;
  }

  Node::Node(const Node &source, const std::string &postfix, boost::shared_ptr<Scene3D> scene3D) : Spatial(source) {
    SetName(source.GetName() + postfix);
    std::vector < boost::intrusive_ptr<Node> > gatherNodes;
    source.GetNodes(gatherNodes);
    for (int i = 0; i < (signed int)gatherNodes.size(); i++) {
      boost::intrusive_ptr<Node> copy(new Node(*gatherNodes.at(i).get(), postfix, scene3D));
      AddNode(copy);
    }

    std::list < boost::intrusive_ptr<Object> > gatherObjects;
    source.GetObjects(gatherObjects, false);
    std::list < boost::intrusive_ptr<Object> >::iterator objectIter = gatherObjects.begin();
    while (objectIter != gatherObjects.end()) {
      boost::intrusive_ptr<Object> objCopy = ObjectFactory::GetInstance().CopyObject((*objectIter), postfix);
      scene3D->CreateSystemObjects(objCopy);
      objCopy->Synchronize();
      AddObject(objCopy);

      objectIter++;
    }
  }

  Node::~Node() {
  }

  void Node::Exit() {
    //printf("node::exit exiting node %s\n", GetName().c_str());
    objects.Lock();
    int objCount = objects.data.size();
    for (int i = 0; i < objCount; i++) {
      //printf("node::exit exiting object %s\n", objects.data.at(i)->GetName().c_str());
      objects.data.at(i)->Exit();
    }
    objects.data.clear();
    objects.Unlock();

    nodes.Lock();
    int nodeCount = nodes.data.size();
    for (int i = 0; i < nodeCount; i++) {
      nodes.data.at(i)->Exit();
    }
    nodes.data.clear();
    nodes.Unlock();
  }

  void Node::AddNode(boost::intrusive_ptr<Node> node) {
    nodes.Lock();
    nodes.data.push_back(node);
    node->SetParent(this);
    //printf("adding node: %s\n", node->GetName().c_str());

    node->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

    nodes.Unlock();

    InvalidateBoundingVolume();
  }

  void Node::DeleteNode(boost::intrusive_ptr<Node> node) {
    nodes.Lock();
    std::vector < boost::intrusive_ptr<Node> >::iterator nodeIter = find(nodes.data.begin(), nodes.data.end(), node);
    if (nodeIter != nodes.data.end()) {
      (*nodeIter)->Exit();
      nodes.data.erase(nodeIter);
    }
    nodes.Unlock();

    InvalidateBoundingVolume();
  }

  void Node::GetNodes(std::vector < boost::intrusive_ptr<Node> > &gatherNodes, bool recurse) const {
    nodes.Lock();
    int nodesSize = nodes.data.size();
    for (int i = 0; i < nodesSize; i++) {
      gatherNodes.push_back(nodes.data.at(i));
      if (recurse) nodes.data.at(i)->GetNodes(gatherNodes, recurse);
    }
    nodes.Unlock();
  }

  boost::intrusive_ptr<Node> Node::GetNode(const std::string &name) {
    boost::mutex::scoped_lock blah(nodes.mutex);
    std::vector < boost::intrusive_ptr<Node> >::iterator nodeIter = nodes.data.begin();
    while (nodeIter != nodes.data.end()) {
      if ((*nodeIter)->GetName() == name) {
        return (*nodeIter);
      } else {
        nodeIter++;
      }
    }
    return boost::intrusive_ptr<Node>();
  }

  void Node::AddObject(boost::intrusive_ptr<Object> object) {
    assert(object.get());
    objects.Lock();
    objects.data.push_back(object);
    object->SetParent(this);

    object->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
    //printf("adding object: %s\n", object->GetName().c_str());

    objects.Unlock();

    InvalidateBoundingVolume();
  }

  boost::intrusive_ptr<Object> Node::GetObject(const std::string &name) {
    boost::mutex::scoped_lock blah(objects.mutex);
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = objects.data.begin();
    while (objIter != objects.data.end()) {
      if ((*objIter)->GetName() == name) {
        return (*objIter);
      } else {
        objIter++;
      }
    }
    return boost::intrusive_ptr<Object>();
  }

  void Node::DeleteObject(const std::string &name, bool exitObject) {
    objects.Lock();
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = objects.data.begin();
    while (objIter != objects.data.end()) {
      if ((*objIter)->GetName() == name) {
        if (exitObject) (*objIter)->Exit();
        (*objIter)->SetParent(0);
        objects.data.erase(objIter);
        // cancel search
        objIter = objects.data.end();
      } else {
        objIter++;
      }
    }

    objects.Unlock();

    InvalidateBoundingVolume();
  }

  void Node::DeleteObject(boost::intrusive_ptr<Object> object, bool exitObject) {
    objects.Lock();
    // verbose printf("deleting object %s\n", object->GetName().c_str());
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = find(objects.data.begin(), objects.data.end(), object);
    if (objIter != objects.data.end()) {
      // verbose printf("found!\n");
      if (exitObject) (*objIter)->Exit();
      (*objIter)->SetParent(0);
      objects.data.erase(objIter);
    } else Log(e_Error, "Node", "DeleteObject", "Object " + object->GetName() + " not found among node " + GetName() + "'s children!");
    objects.Unlock();

    aabb.Lock();
    aabb.data.dirty = true;
    aabb.Unlock();
  }

  void Node::DeleteAllObjects(bool exitObjects) {
    objects.Lock();
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = objects.data.begin();
    while (objIter != objects.data.end()) {
      if (exitObjects) (*objIter)->Exit();
      (*objIter)->SetParent(0);
      objIter++;
    }
    objects.data.clear();

    objects.Unlock();

    aabb.Lock();
    aabb.data.dirty = true;
    aabb.Unlock();
  }

  void Node::RemoveObject(const std::string &name) {
    DeleteObject(name, false);
  }

  void Node::RemoveObject(boost::intrusive_ptr<Object> object) {
    DeleteObject(object, false);
  }

  void Node::RemoveAllObjects() {
    DeleteAllObjects(false);
  }

  void Node::GetSpatials(std::list < boost::intrusive_ptr<Spatial> > &gatherSpatials, bool recurse, int depth) const {
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      gatherSpatials.push_back(objects.data.at(i));
    }
    objects.Unlock();

    if (recurse) {
      nodes.Lock();
      int nodesSize = nodes.data.size();
      for (int i = 0; i < nodesSize; i++) {
        gatherSpatials.push_back(nodes.data.at(i));
        nodes.data.at(i)->GetSpatials(gatherSpatials, recurse, depth + 1);
      }
      nodes.Unlock();
    }
  }

  void Node::GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, bool recurse, int depth) const {
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      gatherObjects.push_back(objects.data.at(i));
    }
    objects.Unlock();

    if (recurse) {
      nodes.Lock();
      int nodesSize = nodes.data.size();
      for (int i = 0; i < nodesSize; i++) {
        nodes.data.at(i)->GetObjects(gatherObjects, recurse, depth + 1);
      }
      nodes.Unlock();
    }
  }

  void Node::GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse, int depth) const {
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.data.at(i)->GetAABB().Intersects(bounding)) gatherObjects.push_back(objects.data.at(i));
    }
    objects.Unlock();

    if (recurse) {
      nodes.Lock();
      int nodesSize = nodes.data.size();
      for (int i = 0; i < nodesSize; i++) {
        if (nodes.data.at(i)->GetAABB().Intersects(bounding)) nodes.data.at(i)->GetObjects(gatherObjects, bounding, recurse, depth + 1);
      }
      nodes.Unlock();
    }
  }

  void Node::GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse, int depth) const {
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.data.at(i)->GetAABB().Intersects(bounding)) gatherObjects.push_back(objects.data.at(i));
    }
    objects.Unlock();

    if (recurse) {
      nodes.Lock();
      int nodesSize = nodes.data.size();
      for (int i = 0; i < nodesSize; i++) {
        if (nodes.data.at(i)->GetAABB().Intersects(bounding)) nodes.data.at(i)->GetObjects(gatherObjects, bounding, recurse, depth + 1);
      }
      nodes.Unlock();
    }
  }

  void Node::PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) {
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.data.at(i)->IsEnabled()) if (objects.data.at(i)->GetObjectType() == targetObjectType) objects.data.at(i)->Poke(targetSystemType);
    }
    objects.Unlock();

    nodes.Lock();
    int nodesSize = nodes.data.size();
    for (int i = 0; i < nodesSize; i++) {
      nodes.data.at(i)->PokeObjects(targetObjectType, targetSystemType);
    }
    nodes.Unlock();
  }

  void Node::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {

    InvalidateSpatialData();
    InvalidateBoundingVolume();

    nodes.Lock();
    int nodesSize = nodes.data.size();
    for (int i = 0; i < nodesSize; i++) {
      // todo: not for localmode == absolute? doesn't seem to work, but why not?! commented out for now (maybe because it won't validate spatialdata/bounding volume of parents of these?)
      //if (nodes.data.at(i)->GetLocalMode() != e_LocalMode_Absolute)
      nodes.data.at(i)->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
    nodes.Unlock();

    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      //if (objects.data.at(i)->GetLocalMode() != e_LocalMode_Absolute)
      objects.data.at(i)->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
    objects.Unlock();
  }

  void Node::PrintTree(int recursionDepth) {
    nodes.Lock();
    int nodesSize = nodes.data.size();
    for (int i = 0; i < nodesSize; i++) {
      for (int space = 0; space < recursionDepth; space++) printf("|     ");
      printf("|-----[NODE] %s\n", nodes.data.at(i)->GetName().c_str());
      nodes.data.at(i)->PrintTree(recursionDepth + 1);
    }
    nodes.Unlock();
    objects.Lock();
    int objectsSize = objects.data.size();
    for (int i = 0; i < objectsSize; i++) {
      for (int space = 0; space < recursionDepth; space++) printf("|     ");
      printf("|-----%s\n", objects.data.at(i)->GetName().c_str());
    }
    objects.Unlock();
  }

  AABB Node::GetAABB() const {
    AABB tmp;

    aabb.Lock();

    if (aabb.data.dirty == true) {

      tmp.Reset();
      aabb.Unlock();

      nodes.Lock();
      int nodesSize = nodes.data.size();
      for (int i = 0; i < nodesSize; i++) {
        tmp += (nodes.data.at(i)->GetAABB());
      }
      nodes.Unlock();

      objects.Lock();
      int objectsSize = objects.data.size();
      for (int i = 0; i < objectsSize; i++) {
        tmp += (objects.data.at(i)->GetAABB());
      }
      objects.Unlock();

      aabb.Lock();
      aabb.data.dirty = false;
      aabb.data.aabb = tmp;
    } else {
      tmp = aabb.data.aabb;
    }

    aabb.Unlock();

    return tmp;
  }

}
