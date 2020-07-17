// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_AUDIO_OBJECT
#define _HPP_SYSTEMS_AUDIO_OBJECT

#include "defines.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  class AudioScene;

  class AudioObject : public ISystemObject {

    public:
      AudioObject(AudioScene *audioScene);
      virtual ~AudioObject();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType) = 0;

      AudioScene *GetAudioScene();

    protected:
      AudioScene *audioScene;

  };

}

#endif
