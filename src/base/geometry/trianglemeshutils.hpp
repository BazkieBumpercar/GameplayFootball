// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_TRIANGLEMESHUTILS
#define _HPP_TRIANGLEMESHUTILS

#include "defines.hpp"

namespace blunted {

  class AABB;

  AABB GetTriangleMeshAABB(float *vertices, int verticesDataSize, const std::vector<unsigned int> &indices);
  int GetTriangleMeshElementCount();

}

#endif
