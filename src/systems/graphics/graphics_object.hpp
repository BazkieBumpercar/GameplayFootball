// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_GRAPHICS_OBJECT
#define _HPP_SYSTEMS_GRAPHICS_OBJECT

#include "defines.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  class GraphicsScene;

  class GraphicsObject : public ISystemObject {

    public:
      GraphicsObject(GraphicsScene *graphicsScene);
      virtual ~GraphicsObject();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType) = 0;

      GraphicsScene *GetGraphicsScene();

    protected:
      GraphicsScene *graphicsScene;

  };

}

#endif
