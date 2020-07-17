// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_IFACE_SCENE
#define _HPP_SYSTEMS_IFACE_SCENE

#include "defines.hpp"

#include "types/observer.hpp"
#include "scene/object.hpp"
#include "types/command.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  class ISystemScene {

    public:
      virtual ~ISystemScene() {}

    protected:

  };

}

#endif
