// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "splitgeometry.hpp"

#include "base/utils.hpp"
#include "scene/objectfactory.hpp"
#include "managers/resourcemanagerpool.hpp"
#include "base/geometry/trianglemeshutils.hpp"

namespace blunted {

  struct GeomIndex {
    float x;
    float y;
    boost::intrusive_ptr < Resource<GeometryData> > geomData;
  };

  boost::intrusive_ptr < Resource<GeometryData> > GetGridGeom(const std::string &name, boost::shared_ptr<Scene3D> scene3D, std::vector<GeomIndex> &geomVec, const AABB &aabb, float gridSize) {

    // calculate grid position

    Vector3 center;
    aabb.GetCenter(center);
    //center.Print();

    float x, y;

    float index = center.coords[0] / gridSize;
    int intIndex = int(round(index));
    x = gridSize * intIndex;

    index = center.coords[1] / gridSize;
    intIndex = int(round(index));
    y = gridSize * intIndex;


    // check if geom exists

    for (int i = 0; i < (signed int)geomVec.size(); i++) {
      if (geomVec.at(i).x == x && geomVec.at(i).y == y) {
        //printf("%f %f %f %f\n", geomVec.at(i).x, x, geomVec.at(i).y, y);
        return geomVec.at(i).geomData; // found! return geomdata
      }
    }


    // not found! create geomdata

    GeomIndex newIndex;
    newIndex.x = x;
    newIndex.y = y;
    newIndex.geomData = ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch(name + " gridGeomData @ " + int_to_str(x) + ", " + int_to_str(y), false, false);
    geomVec.push_back(newIndex);

    return newIndex.geomData;
  }

  boost::intrusive_ptr<Node> SplitGeometry(boost::shared_ptr<Scene3D> scene3D, boost::intrusive_ptr<Geometry> source, float gridSize) {

    boost::intrusive_ptr<Node> resultNode(new Node(source->GetName()));

    std::vector<GeomIndex> geomVec;

    // iterate trianglemeshes
    boost::intrusive_ptr < Resource<GeometryData> > geomData = source->GetGeometryData();

    geomData->resourceMutex.lock(); // lock all the time, else triangle meshes might get altered

    std::vector < MaterializedTriangleMesh > tmeshes = geomData->GetResource()->GetTriangleMeshes();

    //printf("%i subgeoms found\n", tmeshes.size());
    for (int i = 0; i < (signed int)tmeshes.size(); i++) {

      std::vector<unsigned int> indices; // empty == don't use indices
      AABB aabb = GetTriangleMeshAABB(tmeshes.at(i).vertices, tmeshes.at(i).verticesDataSize, indices);
      boost::intrusive_ptr < Resource<GeometryData> > subGeomData = GetGridGeom(source->GetName(), scene3D, geomVec, aabb, gridSize);

      float *newTMesh = new float[tmeshes.at(i).verticesDataSize];
      memcpy(newTMesh, tmeshes.at(i).vertices, tmeshes.at(i).verticesDataSize * sizeof(float));
      subGeomData->resourceMutex.lock();
      subGeomData->GetResource()->AddTriangleMesh(tmeshes.at(i).material, newTMesh, tmeshes.at(i).verticesDataSize, indices);
      subGeomData->resourceMutex.unlock();
    }

    geomData->resourceMutex.unlock();

    for (int i = 0; i < (signed int)geomVec.size(); i++) {
      float x = geomVec.at(i).x;
      float y = geomVec.at(i).y;
      boost::intrusive_ptr<Geometry> geom = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject(source->GetName() + " gridGeom @ " + int_to_str(x) + ", " + int_to_str(y), e_ObjectType_Geometry));
      scene3D->CreateSystemObjects(geom);
      geom->SetGeometryData(geomVec.at(i).geomData);
      resultNode->AddObject(geom);
    }

    return resultNode;
  }

}
