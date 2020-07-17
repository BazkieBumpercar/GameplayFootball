// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "audio_sound.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "../audio_scene.hpp"
#include "../audio_system.hpp"

#include "../rendering/audio_messages.hpp"

namespace blunted {

  AudioSound::AudioSound(AudioScene *audioScene) : AudioObject(audioScene) {
    radius = 512;
    gain = 1.0;
    pitch = 1.0;
    loop = false;
  }

  AudioSound::~AudioSound() {
  }

  boost::intrusive_ptr<Interpreter> AudioSound::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Sound) {
      boost::intrusive_ptr<AudioSound_SoundInterpreter> soundInterpreter(new AudioSound_SoundInterpreter(this));
      return soundInterpreter;
    }
    Log(e_FatalError, "AudioSound", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<AudioSound_SoundInterpreter>();
  }

  void AudioSound::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 AudioSound::GetPosition() const {
    return position;
  }

  void AudioSound::SetRadius(float radius) {
    this->radius = radius;
  }

  float AudioSound::GetRadius() const {
    return radius;
  }



  AudioSound_SoundInterpreter::AudioSound_SoundInterpreter(AudioSound *caller) : caller(caller) {
  }

  void AudioSound_SoundInterpreter::OnLoad(boost::intrusive_ptr < Resource<SoundBuffer> > resource) {

    bool alreadyThere = false;
    // todo: use the resource mutex ??
    caller->audioSoundBuffer =
      ResourceManagerPool::GetInstance().GetManager<AudioSoundBuffer>(e_ResourceType_AudioSoundBuffer)->
        Fetch(resource->GetIdentString(), false, alreadyThere, true); // false == don't try to use loader
    AudioRenderer *renderer = caller->GetAudioScene()->GetAudioSystem()->GetAudioRenderer();

    if (!alreadyThere) {
      caller->audioSoundBuffer->GetResource()->SetAudioRenderer(renderer);
      caller->audioSoundBuffer->GetResource()->CreateAudioSoundBuffer(resource->GetResource()->GetData());
    }
  }

  void AudioSound_SoundInterpreter::OnUnload() {

    caller->audioSoundBuffer.reset();
    delete caller;
    caller = 0;
  }

  void AudioSound_SoundInterpreter::OnMove(const Vector3 &position) {
    caller->SetPosition(position);
  }

  void AudioSound_SoundInterpreter::OnPoke() {
    AudioRenderer *renderer = caller->GetAudioScene()->GetAudioSystem()->GetAudioRenderer();
    caller->audioSoundBuffer->resourceMutex.lock();
    boost::intrusive_ptr<AudioRendererMessage_PlayAudioSoundBuffer> playMsg(new AudioRendererMessage_PlayAudioSoundBuffer(caller->audioSoundBuffer->GetResource()->GetID()));
    caller->audioSoundBuffer->resourceMutex.unlock();
    renderer->messageQueue.PushMessage(playMsg);
  }

  void AudioSound_SoundInterpreter::UpdateParams() {
    AudioRenderer *renderer = caller->GetAudioScene()->GetAudioSystem()->GetAudioRenderer();
    caller->audioSoundBuffer->resourceMutex.lock();
    boost::intrusive_ptr<AudioRendererMessage_ConfigAudioSoundBuffer> updateMsg(new AudioRendererMessage_ConfigAudioSoundBuffer(caller->audioSoundBuffer->GetResource()->GetID(), caller->GetGain(), caller->GetPitch(), caller->GetLoop()));
    caller->audioSoundBuffer->resourceMutex.unlock();
    renderer->messageQueue.PushMessage(updateMsg);
  }

  void AudioSound_SoundInterpreter::SetRadius(float radius) {
    caller->SetRadius(radius);
    UpdateParams();
  }

  void AudioSound_SoundInterpreter::SetLoop(bool loop) {
    caller->SetLoop(loop);
    UpdateParams();
  }

  void AudioSound_SoundInterpreter::SetGain(float gain) {
    caller->SetGain(gain);
    UpdateParams();
  }

  void AudioSound_SoundInterpreter::SetPitch(float pitch) {
    caller->SetPitch(pitch);
    UpdateParams();
  }

}
