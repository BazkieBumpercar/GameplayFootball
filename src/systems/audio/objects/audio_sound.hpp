// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_AUDIOSYSTEM_OBJECT_SOUND
#define _HPP_AUDIOSYSTEM_OBJECT_SOUND

#include "base/math/vector3.hpp"
#include "scene/objects/sound.hpp"

#include "../audio_object.hpp"

#include "../resources/audio_soundbuffer.hpp"

#include "../rendering/interface_audiorenderer.hpp"

namespace blunted {

  class AudioSound_SoundInterpreter;

  class AudioSound : public AudioObject {

    public:
      AudioSound(AudioScene *audioScene);
      virtual ~AudioSound();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      virtual Vector3 GetPosition() const;

      virtual void SetRadius(float radius);
      virtual float GetRadius() const;

      virtual void SetLoop(bool loop) { this->loop = loop; }
      virtual bool GetLoop() const { return loop; }

      virtual void SetGain(float gain) { this->gain = gain; }
      virtual float GetGain() const { return gain; }

      virtual void SetPitch(float pitch) { this->pitch = pitch; }
      virtual float GetPitch() const { return pitch; }

      boost::intrusive_ptr < Resource<AudioSoundBuffer> > audioSoundBuffer;

    protected:
      Vector3 position;
      float radius;
      bool loop;
      float gain;
      float pitch;

  };

  class AudioSound_SoundInterpreter : public ISoundInterpreter {

    public:
      AudioSound_SoundInterpreter(AudioSound *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Audio; }
      virtual void OnLoad(boost::intrusive_ptr < Resource<SoundBuffer> > resource);
      virtual void OnUnload();
      inline virtual void OnMove(const Vector3 &position);
      inline virtual void OnPoke();

      void UpdateParams();
      virtual void SetRadius(float radius);
      virtual void SetLoop(bool loop);
      virtual void SetGain(float gain);
      virtual void SetPitch(float pitch);

    protected:
      AudioSound *caller;

  };

}

#endif
