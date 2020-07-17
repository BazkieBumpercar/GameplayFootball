// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "vertexbuffer.hpp"

#include "systems/graphics/rendering/interface_renderer3d.hpp"
#include "systems/graphics/rendering/r3d_messages.hpp"

namespace blunted {

  VertexBuffer::VertexBuffer() : vertexBufferID(-1), vertexCount(0) {
    //printf("CREATING VertexBufferID\n");
  }

  VertexBuffer::~VertexBuffer() {
    //printf("ERASING VERTEXBUFFER ID #%i\n", vertexBufferID);

    if (vertexBufferID != -1) {
      boost::intrusive_ptr<Renderer3DMessage_DeleteVertexBuffer> deleteVertexBuffer(new Renderer3DMessage_DeleteVertexBuffer(vertexBufferID));
      renderer3D->messageQueue.PushMessage(deleteVertexBuffer);

      deleteVertexBuffer->Wait();
    }
  }

  int VertexBuffer::CreateVertexBuffer(Renderer3D *renderer3D, std::vector<Triangle*> triangles) {
    this->renderer3D = renderer3D;

    boost::intrusive_ptr<Renderer3DMessage_CreateVertexBuffer> createVertexBuffer(new Renderer3DMessage_CreateVertexBuffer(triangles));
    renderer3D->messageQueue.PushMessage(createVertexBuffer);
    createVertexBuffer->Wait();

    vertexBufferID = createVertexBuffer->vertexBufferID;
    vertexCount = triangles.size() * 3;

    return createVertexBuffer->vertexBufferID;
  }

  void VertexBuffer::SetID(int value) {
    vertexBufferID = value;
  }

  int VertexBuffer::GetID() {
    return vertexBufferID;
  }

  int VertexBuffer::GetVertexCount() {
    return vertexCount;
  }

}
