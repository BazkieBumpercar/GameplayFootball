// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEM_AUDIO_RESOURCE_SOUNDBUFFER
#define _HPP_SYSTEM_AUDIO_RESOURCE_SOUNDBUFFER

#include "defines.hpp"

#include "../rendering/interface_audiorenderer.hpp"

#include "scene/resources/soundbuffer.hpp"

namespace blunted {

  class Renderer3D;

  class AudioSoundBuffer {

    public:
      AudioSoundBuffer();
      virtual ~AudioSoundBuffer();

      void SetAudioRenderer(AudioRenderer *renderer);

      int CreateAudioSoundBuffer(const WavData *wavData);
      void DeleteAudioSoundBuffer();

      void SetID(int value);
      int GetID();

    protected:
      int audioSoundBufferID;
      AudioRenderer *renderer;

  };

}

#endif
