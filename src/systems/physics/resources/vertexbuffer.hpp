// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEM_PHYSICS_RESOURCE_VERTEXBUFFER
#define _HPP_SYSTEM_PHYSICS_RESOURCE_VERTEXBUFFER

#include "defines.hpp"

#include "base/geometry/triangle.hpp"

namespace blunted {

  class Renderer3D;

  class VertexBuffer {

    public:
      VertexBuffer();
      virtual ~VertexBuffer();

      int CreateVertexBuffer(Renderer3D *renderer3D, std::vector<Triangle*> triangles);

      void SetID(int value);
      int GetID();

      int GetVertexCount();

    protected:
      int vertexBufferID;
      int vertexCount;
      Renderer3D *renderer3D;

  };

}

#endif
