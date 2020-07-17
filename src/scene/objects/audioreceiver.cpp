// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "audioreceiver.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  AudioReceiver::AudioReceiver(std::string name) : Object(name, e_ObjectType_AudioReceiver) {
  }

  AudioReceiver::~AudioReceiver() {
  }


  void AudioReceiver::Init() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IAudioReceiverInterpreter *AudioReceiverInterpreter = static_cast<IAudioReceiverInterpreter*>(observers.at(i).get());
      AudioReceiverInterpreter->OnLoad();
    }

    subjectMutex.unlock();
  }

  void AudioReceiver::Exit() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IAudioReceiverInterpreter *AudioReceiverInterpreter = static_cast<IAudioReceiverInterpreter*>(observers.at(i).get());
      AudioReceiverInterpreter->OnUnload();
    }

    Object::Exit();

    subjectMutex.unlock();
  }

  void AudioReceiver::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    InvalidateSpatialData();

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      if (observers.at(i)->GetSystemType() != excludeSystem) {
        IAudioReceiverInterpreter *audioReceiverInterpreter = static_cast<IAudioReceiverInterpreter*>(observers.at(i).get());
        audioReceiverInterpreter->OnSpatialChange(GetDerivedPosition(), GetDerivedRotation());
      }
    }

    subjectMutex.unlock();
  }

}
