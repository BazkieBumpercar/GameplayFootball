// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "trianglemesh.hpp"

namespace blunted {

  TriangleMesh::TriangleMesh() {
    //printf("CREATING TriangleMesh\n");
    aabb.Reset();
    dirtyAABB = false;
  }

  TriangleMesh::~TriangleMesh() {
    //printf("OBLITERATING TRIANGLEMESH\n");
    for (int i = 0; i < (signed int)triangles.size(); i++) {
      delete triangles.at(i);
    }
    triangles.clear();
  }

  TriangleMesh::TriangleMesh(const TriangleMesh &src) {
    for (int i = 0; i < (signed int)src.triangles.size(); i++) {
      triangles.push_back(new Triangle(*src.triangles.at(i)));
    }
    aabb = src.GetAABB();
    dirtyAABB = false;
  }

  const std::vector<Triangle*> &TriangleMesh::GetTriangles() {
    return triangles;
  }

  Triangle *TriangleMesh::GetTriangle(int id) {
    return triangles[id];
  }

  void TriangleMesh::AddTriangle(Triangle *triangle) {
    triangles.push_back(triangle);
    dirtyAABB = true;
  }

  int TriangleMesh::GetTriangleCount() const {
    return triangles.size();
  }

  AABB TriangleMesh::GetAABB() const {
    if (dirtyAABB) {
      aabb.Reset();
      for (int i = 0; i < (signed int)triangles.size(); i++) {
        aabb += triangles.at(i)->GetAABB();
      }
      dirtyAABB = false;
    }
    return aabb;
  }

}
