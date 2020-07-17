// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "sound.hpp"

#include "systems/isystemobject.hpp"

namespace blunted {

  Sound::Sound(std::string name) : Object(name, e_ObjectType_Sound) {
    radius = 512;
    gain = 1.0;
    pitch = 1.0;
    loop = false;
  }

  Sound::~Sound() {
  }

  void Sound::Exit() { // ATOMIC
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *SoundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      SoundInterpreter->OnUnload();
    }

    Object::Exit();

    if (soundBuffer) soundBuffer.reset();

    subjectMutex.unlock();
  }

  void Sound::SetRadius(float radius) {
    subjectMutex.lock();

    this->radius = radius;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      soundInterpreter->SetRadius(radius);
    }

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  float Sound::GetRadius() const {
    subjectMutex.lock();
    float rad = radius;
    subjectMutex.unlock();
    return rad;
  }

  void Sound::SetLoop(bool loop) {
    subjectMutex.lock();

    this->loop = loop;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      soundInterpreter->SetLoop(loop);
    }

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  bool Sound::GetLoop() const {
    subjectMutex.lock();
    bool tloop = loop;
    subjectMutex.unlock();
    return tloop;
  }

  void Sound::SetGain(float gain) {
    subjectMutex.lock();

    this->gain = gain;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      soundInterpreter->SetGain(gain);
    }

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  float Sound::GetGain() const {
    subjectMutex.lock();
    float tgain = gain;
    subjectMutex.unlock();
    return tgain;
  }

  void Sound::SetPitch(float pitch) {
    subjectMutex.lock();

    this->pitch = pitch;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      soundInterpreter->SetPitch(pitch);
    }

    subjectMutex.unlock();

    InvalidateBoundingVolume();
  }

  float Sound::GetPitch() const {
    subjectMutex.lock();
    float tpitch = pitch;
    subjectMutex.unlock();
    return tpitch;
  }

  void Sound::SetSoundBuffer(boost::intrusive_ptr < Resource<SoundBuffer> > soundBuffer) {
    subjectMutex.lock();

    this->soundBuffer = soundBuffer;

    soundBuffer->resourceMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      soundInterpreter->OnLoad(soundBuffer);
    }

    soundBuffer->resourceMutex.unlock();

    subjectMutex.unlock();
  }

  boost::intrusive_ptr < Resource<SoundBuffer> > Sound::GetSoundBuffer() {
    boost::mutex::scoped_lock blah(subjectMutex);
    return soundBuffer;
  }

  void Sound::Poke(e_SystemType targetSystemType) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
      if (soundInterpreter->GetSystemType() == targetSystemType) soundInterpreter->OnPoke();
    }

    subjectMutex.unlock();
  }

  void Sound::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {
    InvalidateSpatialData();
    InvalidateBoundingVolume();

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      if (observers.at(i)->GetSystemType() != excludeSystem) {
        ISoundInterpreter *soundInterpreter = static_cast<ISoundInterpreter*>(observers.at(i).get());
        soundInterpreter->OnMove(GetDerivedPosition());
      }
    }

    subjectMutex.unlock();
  }

  AABB Sound::GetAABB() const {
    aabb.Lock();
    if (aabb.data.dirty == true) {
      Vector3 pos = GetDerivedPosition();
      aabb.data.aabb.minxyz = pos - radius;
      aabb.data.aabb.maxxyz = pos + radius;
      aabb.data.aabb.MakeDirty();
      aabb.data.dirty = false;
    }
    AABB tmp = aabb.data.aabb;
    aabb.Unlock();
    return tmp;
  }

}
