// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_AUDIORENDERER_MESSAGES
#define _HPP_AUDIORENDERER_MESSAGES

#include "interface_audiorenderer.hpp"

#include "scene/resources/soundbuffer.hpp"

namespace blunted {

  // messages

  class AudioRendererMessage_CreateContext : public Command {

    public:
      AudioRendererMessage_CreateContext() : Command("audiomsg_CreateContext") {};

      // return values
      bool success;

    protected:
      virtual bool Execute(void *caller = NULL) {
        success = static_cast<AudioRenderer*>(caller)->CreateContext();
        return true;
      }

  };

  class AudioRendererMessage_DeleteContext : public Command {

    public:
      AudioRendererMessage_DeleteContext() : Command("audiomsg_DeleteContext") {};

      // return values
      bool success;

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<AudioRenderer*>(caller)->Exit();
        return true;
      }

  };

  class AudioRendererMessage_CreateAudioSoundBuffer : public Command {

    public:
      AudioRendererMessage_CreateAudioSoundBuffer(const WavData *wavData) : Command("audiomsg_CreateAudioSoundBuffer"), wavData(wavData) {};

      int audioSoundBufferID;

    protected:
      virtual bool Execute(void *caller = NULL);

      const WavData *wavData;

  };

  class AudioRendererMessage_DeleteAudioSoundBuffer : public Command {

    public:
      AudioRendererMessage_DeleteAudioSoundBuffer(int audioSoundBufferID) : Command("audiomsg_DeleteAudioSoundBuffer"), audioSoundBufferID(audioSoundBufferID) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int audioSoundBufferID;

  };

  class AudioRendererMessage_PlayAudioSoundBuffer : public Command {

    public:
      AudioRendererMessage_PlayAudioSoundBuffer(int audioSoundBufferID) : Command("audiomsg_PlayAudioSoundBuffer"), audioSoundBufferID(audioSoundBufferID) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int audioSoundBufferID;

  };

  class AudioRendererMessage_ConfigAudioSoundBuffer : public Command {

    public:
      AudioRendererMessage_ConfigAudioSoundBuffer(int audioSoundBufferID, float gain = 0.1, float pitch = 1.0, bool loop = false) : Command("audiomsg_ConfigAudioSoundBuffer"), audioSoundBufferID(audioSoundBufferID), gain(gain), pitch(pitch), loop(loop) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int audioSoundBufferID;
      float gain;
      float pitch;
      bool loop;

  };

}

#endif
