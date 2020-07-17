// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT_SKYBOX
#define _HPP_OBJECT_SKYBOX

#include "geometry.hpp"

namespace blunted {

  class Skybox : public Geometry {

    public:
      Skybox(std::string name);
      virtual ~Skybox();

    protected:

  };

  class ISkyboxInterpreter : public IGeometryInterpreter {

    public:
      virtual void OnLoad(boost::intrusive_ptr<Skybox> geom) = 0;
      virtual void OnUnload() = 0;
      virtual void OnMove(const Vector3 &position) = 0;
      virtual void OnRotate(const Quaternion &rotation) = 0;

      virtual void OnPoke() = 0;

    protected:

  };

}

#endif
