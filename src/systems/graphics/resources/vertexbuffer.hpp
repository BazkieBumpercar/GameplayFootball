#ifndef _HPP_SYSTEM_GRAPHICS_RESOURCE_VERTEXBUFFER
#define _HPP_SYSTEM_GRAPHICS_RESOURCE_VERTEXBUFFER

#include "defines.hpp"

#include "base/geometry/trianglemeshutils.hpp"

namespace blunted {

  class Renderer3D;

  struct VertexBufferID {
    VertexBufferID() {
      bufferID = -1;
    }
    int bufferID; // -1 if uninitialized
    unsigned int vertexArrayID;
    unsigned int elementArrayID;
  };

  class VertexBuffer {

    public:
      VertexBuffer();
      virtual ~VertexBuffer();

      void SetTriangleMesh(float *vertices, unsigned int verticesDataSize, std::vector<unsigned int> indices);
      void TriangleMeshWasUpdatedExternally(unsigned int verticesDataSize, std::vector<unsigned int> indices);
      VertexBufferID CreateOrUpdateVertexBuffer(Renderer3D *renderer3D, bool dynamicBuffer);

      float *GetTriangleMesh();

      int GetID();
      int GetVaoID();
      int GetElementID();

      int GetVertexCount();
      int GetVerticesDataSize();

    protected:
      float *vertices;
      int verticesDataSize;
      std::vector<unsigned int> indices;
      VertexBufferID vertexBufferID;
      int vertexCount;
      Renderer3D *renderer3D;
      bool dynamicBuffer;

      bool sizeChanged;

  };

}

#endif
