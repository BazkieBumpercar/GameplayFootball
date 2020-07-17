// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "vertexbuffer.hpp"

#include "../rendering/interface_renderer3d.hpp"
#include "../rendering/r3d_messages.hpp"

namespace blunted {

  VertexBuffer::VertexBuffer() : vertices(0), verticesDataSize(0), vertexCount(0), dynamicBuffer(false), sizeChanged(false) {
    //printf("CREATING VertexBufferID\n");
    assert(vertexBufferID.bufferID == -1);
  }

  VertexBuffer::~VertexBuffer() {
    //printf("ERASING VERTEXBUFFER ID #%i.. ", vertexBufferID);

    if (vertexBufferID.bufferID != -1) {
      boost::intrusive_ptr<Renderer3DMessage_DeleteVertexBuffer> deleteVertexBuffer(new Renderer3DMessage_DeleteVertexBuffer(vertexBufferID));
      renderer3D->messageQueue.PushMessage(deleteVertexBuffer);
      deleteVertexBuffer->Wait();

      if (vertices) delete [] vertices;
    }

    //printf("[done]\n");
  }

  void VertexBuffer::SetTriangleMesh(float *vertices, unsigned int verticesDataSize, std::vector<unsigned int> indices) {
    if (this->vertices) delete [] this->vertices;
    this->vertices = vertices;
    TriangleMeshWasUpdatedExternally(verticesDataSize, indices);
  }

  void VertexBuffer::TriangleMeshWasUpdatedExternally(unsigned int verticesDataSize, std::vector<unsigned int> indices) {
    //printf("%i == %i ?  %i == %i ?\n", this->indices.size(), indices.size(), vertexCount, verticesDataSize / GetTriangleMeshElementCount() / 3);
    if (indices.size() > 0) {
      if (indices.size() != this->indices.size()) sizeChanged = true;
      this->indices = indices;
    }
    this->verticesDataSize = verticesDataSize;
    int tmpVertexCount = verticesDataSize / GetTriangleMeshElementCount() / 3;
    if (tmpVertexCount != vertexCount) sizeChanged = true;
    vertexCount = tmpVertexCount;
  }

  VertexBufferID VertexBuffer::CreateOrUpdateVertexBuffer(Renderer3D *renderer3D, bool dynamicBuffer) {
    this->renderer3D = renderer3D;

    // changed size? delete vbo first!
    if (vertexBufferID.bufferID != -1 && sizeChanged) {
      boost::intrusive_ptr<Renderer3DMessage_DeleteVertexBuffer> deleteVertexBuffer(new Renderer3DMessage_DeleteVertexBuffer(vertexBufferID));
      renderer3D->messageQueue.PushMessage(deleteVertexBuffer);
      deleteVertexBuffer->Wait();
      vertexBufferID.bufferID = -1;

      //printf("deletele\n");
    }

    if (vertexBufferID.bufferID == -1) {
      this->dynamicBuffer = dynamicBuffer;
      //if (dynamicBuffer == true) printf("DYNAMIEK OLE!\n\n\n");
      e_VertexBufferUsage usage = e_VertexBufferUsage_StaticDraw;
      //if (dynamicBuffer) usage = e_VertexBufferUsage_StreamDraw;
      if (dynamicBuffer) usage = e_VertexBufferUsage_DynamicDraw;

      assert(vertices != 0);
      boost::intrusive_ptr<Renderer3DMessage_CreateVertexBuffer> createVertexBuffer(new Renderer3DMessage_CreateVertexBuffer(vertices, verticesDataSize, indices, usage));
      renderer3D->messageQueue.PushMessage(createVertexBuffer);
      createVertexBuffer->Wait();

      vertexBufferID = createVertexBuffer->vertexBufferID;

      //printf("creerle\n");
    } else {
      assert(vertices != 0);
      boost::intrusive_ptr<Renderer3DMessage_UpdateVertexBuffer> updateVertexBuffer(new Renderer3DMessage_UpdateVertexBuffer(vertexBufferID, vertices, verticesDataSize));
      renderer3D->messageQueue.PushMessage(updateVertexBuffer);
      updateVertexBuffer->Wait(); // todo: make the vertices* stuff lockable so we don't have to wait here

      //printf("updatele\n");
    }

    sizeChanged = false;

    return vertexBufferID;
  }

  float *VertexBuffer::GetTriangleMesh() {
    return vertices;
  }

  int VertexBuffer::GetID() {
    return vertexBufferID.bufferID;
  }

  int VertexBuffer::GetVaoID() {
    return vertexBufferID.vertexArrayID;
  }

  int VertexBuffer::GetElementID() {
    return vertexBufferID.elementArrayID;
  }

  int VertexBuffer::GetVertexCount() {
    return vertexCount;
  }

  int VertexBuffer::GetVerticesDataSize() {
    return verticesDataSize;
  }

}
