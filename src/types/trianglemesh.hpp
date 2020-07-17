// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_TRIANGLEMESH
#define _HPP_TRIANGLEMESH

#include "defines.hpp"
#include "base/geometry/triangle.hpp"

namespace blunted {

  class TriangleMesh {

    public:
      TriangleMesh();
      virtual ~TriangleMesh();
      TriangleMesh(const TriangleMesh &src);

      // todo: what about acquisition is ownership?
      void AddTriangle(Triangle *triangle);
      const std::vector<Triangle*> &GetTriangles();
      Triangle *GetTriangle(int id);
      int GetTriangleCount() const;
      AABB GetAABB() const;

    protected:
      mutable AABB aabb;
      mutable bool dirtyAABB;

      std::vector<Triangle*> triangles;

  };

}

#endif
