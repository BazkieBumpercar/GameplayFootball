// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_AUDIORENDERER
#define _HPP_AUDIORENDERER

#include "types/thread.hpp"
#include "types/resource.hpp"

#include "base/math/vector3.hpp"

#include "scene/resources/soundbuffer.hpp"

namespace blunted {

  enum e_AudioRenderer_SourceParameter {
    e_AudioRenderer_SourceParameter_Loop,
    e_AudioRenderer_SourceParameter_Gain,
    e_AudioRenderer_SourceParameter_Pitch
  };

  class AudioRenderer : public Thread {

    public:
      virtual ~AudioRenderer() {};

      // init & exit
      virtual bool CreateContext() = 0;
      virtual void Exit() = 0;

      virtual int CreateAudioSoundBuffer(const WavData *wavData) = 0;
      virtual void DeleteAudioSoundBuffer(int audioSoundBufferID) = 0;
      virtual void PlayAudioSoundBuffer(int audioSoundBufferID) = 0;

      virtual void SetListenerParameters(const Vector3 &position, const Vector3 &velocity, const Quaternion &orientation) = 0;

      virtual void SetSourceParameter(int audioSoundBufferID, e_AudioRenderer_SourceParameter parameter, float value) = 0;

      void operator()() = 0;

    protected:

  };

}

#endif
