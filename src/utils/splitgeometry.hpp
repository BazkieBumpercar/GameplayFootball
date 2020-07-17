// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_UTILS_SPLITGEOMETRY
#define _HPP_UTILS_SPLITGEOMETRY

#include "defines.hpp"

#include "scene/scene3d/scene3d.hpp"
#include "scene/scene3d/node.hpp"

#include "scene/objects/geometry.hpp"

namespace blunted {

  boost::intrusive_ptr<Node> SplitGeometry(boost::shared_ptr<Scene3D> scene3D, boost::intrusive_ptr<Geometry> source, float gridSize = 1.0);

}

#endif
