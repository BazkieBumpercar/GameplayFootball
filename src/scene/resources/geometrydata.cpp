// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "geometrydata.hpp"

#include "systems/isystemobject.hpp"

#include "base/geometry/trianglemeshutils.hpp"

namespace blunted {

  GeometryData::GeometryData() {
    aabb.aabb.Reset();
    aabb.dirty = false;
    isDynamic = false;
  }

  GeometryData::~GeometryData() {
    //printf("ANNIHILATING TMESH\n");
    for (unsigned int i = 0; i < triangleMeshes.size(); i++) {
      delete [] triangleMeshes.at(i).vertices;
    }
  }

  GeometryData::GeometryData(const GeometryData &src) {
    for (unsigned int i = 0; i < src.triangleMeshes.size(); i++) {
      // shallow copy
      MaterializedTriangleMesh mesh = src.triangleMeshes.at(i);

      // 'deepen' vertices
      mesh.vertices = new float[src.triangleMeshes.at(i).verticesDataSize];
      memcpy(mesh.vertices, src.triangleMeshes.at(i).vertices, src.triangleMeshes.at(i).verticesDataSize * sizeof(float));

      triangleMeshes.push_back(mesh);
    }
    aabb.aabb = src.GetAABB();
    aabb.dirty = false;
  }

  bool SortTriangleMeshes(const MaterializedTriangleMesh &tm1, const MaterializedTriangleMesh &tm2) {
    return tm1.material.diffuseTexture->GetIdentString() < tm2.material.diffuseTexture->GetIdentString();
  }


  void GeometryData::DeleteTriangleMeshes() {
    for (unsigned int i = 0; i < triangleMeshes.size(); i++) {
      delete [] triangleMeshes.at(i).vertices;
      triangleMeshes.clear();
    }
  }

  void GeometryData::SetTriangleMesh(Material material, float *vertices, int verticesDataSize, std::vector<unsigned int> indices) {
    DeleteTriangleMeshes();
    AddTriangleMesh(material, vertices, verticesDataSize, indices);
  }

  void GeometryData::AddTriangleMesh(Material material, float *vertices, int verticesDataSize, std::vector<unsigned int> indices) {
    assert(indices.size() % 3 == 0);
    MaterializedTriangleMesh mesh;
    mesh.material = material;
    mesh.vertices = vertices;
    mesh.verticesDataSize = verticesDataSize;
    mesh.indices = indices;

    triangleMeshes.push_back(mesh);
    aabb.aabb.Reset();
    aabb.dirty = true;
  }

  std::vector < MaterializedTriangleMesh > GeometryData::GetTriangleMeshes() {
    return triangleMeshes;
  }

  std::vector < MaterializedTriangleMesh > &GeometryData::GetTriangleMeshesRef() {
    return triangleMeshes;
  }

  AABB GeometryData::GetAABB() const {
    if (aabb.dirty) {
      aabb.aabb.Reset();
      for (int i = 0; i < (signed int)triangleMeshes.size(); i++) {
        aabb.aabb += GetTriangleMeshAABB(triangleMeshes.at(i).vertices, triangleMeshes.at(i).verticesDataSize, triangleMeshes.at(i).indices);
      }
      aabb.dirty = false;
    }
    return aabb.aabb;
  }

}
