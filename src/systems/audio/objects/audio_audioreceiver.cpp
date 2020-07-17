// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "audio_audioreceiver.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "../audio_scene.hpp"
#include "../audio_system.hpp"

namespace blunted {

  AudioAudioReceiver::AudioAudioReceiver(AudioScene *audioScene) : AudioObject(audioScene) {
  }

  AudioAudioReceiver::~AudioAudioReceiver() {
  }

  boost::intrusive_ptr<Interpreter> AudioAudioReceiver::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_AudioReceiver) {
      boost::intrusive_ptr<AudioAudioReceiver_AudioReceiverInterpreter> audioReceiverInterpreter(new AudioAudioReceiver_AudioReceiverInterpreter(this));
      return audioReceiverInterpreter;
    }
    Log(e_FatalError, "AudioAudioReceiver", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<AudioAudioReceiver_AudioReceiverInterpreter>();
  }

  void AudioAudioReceiver::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 AudioAudioReceiver::GetPosition() const {
    return position;
  }

  void AudioAudioReceiver::SetRotation(const Quaternion &newRotation) {
    rotation = newRotation;
  }

  Quaternion AudioAudioReceiver::GetRotation() const {
    return rotation;
  }




  AudioAudioReceiver_AudioReceiverInterpreter::AudioAudioReceiver_AudioReceiverInterpreter(AudioAudioReceiver *caller) : caller(caller) {
  }

  void AudioAudioReceiver_AudioReceiverInterpreter::OnLoad() {
  }

  void AudioAudioReceiver_AudioReceiverInterpreter::OnUnload() {
  }

  void AudioAudioReceiver_AudioReceiverInterpreter::OnSpatialChange(const Vector3 &position, const Quaternion &rotation) {
    caller->SetPosition(position);
    caller->SetRotation(rotation);
  }

}
