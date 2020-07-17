// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT_AUDIORECEIVER
#define _HPP_OBJECT_AUDIORECEIVER

#include "defines.hpp"

#include "scene/object.hpp"

#include "types/interpreter.hpp"

#include "base/math/quaternion.hpp"
#include "base/math/vector3.hpp"

namespace blunted {

  class AudioReceiver : public Object {

    public:
      AudioReceiver(std::string name);
      virtual ~AudioReceiver();

      virtual void Init();
      virtual void Exit();

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

    protected:

  };

  class IAudioReceiverInterpreter : public Interpreter {

    public:
      virtual void OnLoad() = 0;
      virtual void OnUnload() = 0;
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation) = 0;

    protected:

  };

}

#endif
