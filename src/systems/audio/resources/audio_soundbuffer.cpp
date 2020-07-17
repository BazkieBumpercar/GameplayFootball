// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "audio_soundbuffer.hpp"

#include "../rendering/audio_messages.hpp"

namespace blunted {

  AudioSoundBuffer::AudioSoundBuffer() : audioSoundBufferID(-1) {
    this->renderer = 0;
  }

  AudioSoundBuffer::~AudioSoundBuffer() {
    DeleteAudioSoundBuffer();
  }

  void AudioSoundBuffer::SetAudioRenderer(AudioRenderer *renderer) {
    this->renderer = renderer;
  }

  int AudioSoundBuffer::CreateAudioSoundBuffer(const WavData *wavData) {
    assert(renderer);
    assert(wavData);

    boost::intrusive_ptr<AudioRendererMessage_CreateAudioSoundBuffer> createAudioSoundBuffer(new AudioRendererMessage_CreateAudioSoundBuffer(wavData));
    renderer->messageQueue.PushMessage(createAudioSoundBuffer);
    createAudioSoundBuffer->Wait();

    audioSoundBufferID = createAudioSoundBuffer->audioSoundBufferID;

    return audioSoundBufferID;
  }

  void AudioSoundBuffer::DeleteAudioSoundBuffer() {
    if (audioSoundBufferID != -1) {
      assert(renderer);
      boost::intrusive_ptr<AudioRendererMessage_DeleteAudioSoundBuffer> deleteAudioSoundBuffer(new AudioRendererMessage_DeleteAudioSoundBuffer(audioSoundBufferID));
      renderer->messageQueue.PushMessage(deleteAudioSoundBuffer);
      deleteAudioSoundBuffer->Wait();

      audioSoundBufferID = -1;
    }
  }

  void AudioSoundBuffer::SetID(int value) {
    audioSoundBufferID = value;
  }

  int AudioSoundBuffer::GetID() {
    return audioSoundBufferID;
  }

}
