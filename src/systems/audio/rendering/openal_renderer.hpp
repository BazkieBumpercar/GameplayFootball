// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_AUDIO_OPENAL
#define _HPP_AUDIO_OPENAL

#include "interface_audiorenderer.hpp"

#include <AL/al.h>
#include <AL/alc.h>

namespace blunted {

  class OpenALRenderer : public AudioRenderer {

    public:
      OpenALRenderer();
      virtual ~OpenALRenderer();

      // init & exit
      virtual bool CreateContext();
      virtual void Exit();

      virtual int CreateAudioSoundBuffer(const WavData *wavData);
      virtual void DeleteAudioSoundBuffer(int audioSoundBufferID);
      virtual void PlayAudioSoundBuffer(int audioSoundBufferID);

      virtual void SetListenerParameters(const Vector3 &position, const Vector3 &velocity, const Quaternion &orientation);

      virtual void SetSourceParameter(int audioSoundBufferID, e_AudioRenderer_SourceParameter parameter, float value);

      void operator()();

    protected:
      ALCcontext *context;

      std::map<ALuint, ALuint> bufferSourceMapping;

  };

}

#endif
