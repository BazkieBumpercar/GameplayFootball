// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "openal_renderer.hpp"

#include "base/log.hpp"
#include "types/command.hpp"

#include "../resources/audio_soundbuffer.hpp"

namespace blunted {

  OpenALRenderer::OpenALRenderer() : context(0) {
  };

  OpenALRenderer::~OpenALRenderer() {
  };


  // init & exit

  bool OpenALRenderer::CreateContext() {

    ALCdevice *device = alcOpenDevice(NULL);

    if (!device) return false;

    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    SetListenerParameters(Vector3(0), Vector3(0), Quaternion(QUATERNION_IDENTITY));

    return true;
  }

  void OpenALRenderer::Exit() {
    //printf("exiting openALrenderer\n");
    ALCdevice *device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
  }


  int OpenALRenderer::CreateAudioSoundBuffer(const WavData *wavData) {
    ALenum format;
    format = 0;
    if (wavData->bits == 8) {
      if (wavData->channels == 1)
        format = AL_FORMAT_MONO8;
      else if (wavData->channels == 2)
        format = AL_FORMAT_STEREO8;

    } else if (wavData->bits == 16) {
      if (wavData->channels == 1)
        format = AL_FORMAT_MONO16;
      else if (wavData->channels == 2)
        format = AL_FORMAT_STEREO16;
    }
    if (!format) {
      Log(e_FatalError, "OpenALRenderer", "CreateAudioSoundBuffer", "Incompatible format");
      return -1;
    }

    ALuint buffer_id;
    alGenBuffers(1, &buffer_id);
    alBufferData(buffer_id, format, wavData->data, wavData->size, wavData->frequency);

    ALuint source_id;
    alGenSources(1, &source_id);
    bufferSourceMapping.insert(std::pair<ALuint, ALuint>(buffer_id, source_id));

    alSourcei(source_id, AL_BUFFER,  buffer_id);
    alSourcef(source_id, AL_PITCH,   1.0f     );
    alSourcef(source_id, AL_GAIN,    0.1f     );
    alSourcei(source_id, AL_LOOPING, false    );

    Vector3 position; position.Set(0, 0, 0);
    Vector3 velocity; velocity.Set(0, 0, 0);
    ALfloat sourcePos[] = { position.coords[0], position.coords[1], position.coords[2] };
    ALfloat sourceVel[] = { velocity.coords[0], velocity.coords[1], velocity.coords[2] };
    alSourcefv(source_id, AL_POSITION, sourcePos);
    alSourcefv(source_id, AL_VELOCITY, sourceVel);

    return buffer_id;
  }

  void OpenALRenderer::DeleteAudioSoundBuffer(int audioSoundBufferID) {
    //printf("DELETING audio buffer\n");

    std::map<ALuint, ALuint>::iterator iter = bufferSourceMapping.find(audioSoundBufferID);
    if (iter == bufferSourceMapping.end()) Log(e_FatalError, "OpenALRenderer", "DeleteAudioSoundBuffer", "Source not found for buffer");
    ALuint sourceID = iter->second;
    bufferSourceMapping.erase(iter);

    alSourceStop(sourceID);
    alSourcei(sourceID, AL_BUFFER, 0);
    alDeleteSources(1, &sourceID);
    ALuint tmp = audioSoundBufferID;
    alDeleteBuffers(1, &tmp);
    //printf("done\n");
  }

  void OpenALRenderer::PlayAudioSoundBuffer(int audioSoundBufferID) {
    alSourcePlay(bufferSourceMapping.find(audioSoundBufferID)->second);
  }

  void OpenALRenderer::SetListenerParameters(const Vector3 &position, const Vector3 &velocity, const Quaternion &orientation) {
    ALfloat listenerPos[] = { position.coords[0], position.coords[1], position.coords[2] };
    ALfloat listenerVel[] = { velocity.coords[0], velocity.coords[1], velocity.coords[2] };
    Matrix3 ori;
    orientation.ConstructMatrix(ori);
    ori.Transpose();
    ALfloat listenerOri[] = { ori.elements[0], ori.elements[1], ori.elements[2], ori.elements[3], ori.elements[4], ori.elements[5] };

    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);
  }

  void OpenALRenderer::SetSourceParameter(int audioSoundBufferID, e_AudioRenderer_SourceParameter parameter, float value) {
    ALuint sourceID = bufferSourceMapping.find(audioSoundBufferID)->second;
    switch (parameter) {
      case e_AudioRenderer_SourceParameter_Loop:
//        printf("LOOP: %i\n", int(round(value)));
        alSourcei(sourceID, AL_LOOPING, int(round(value)));
        break;
      case e_AudioRenderer_SourceParameter_Gain:
//        printf("GAIN: %f\n", value);
        alSourcef(sourceID, AL_GAIN, value);
        break;
      case e_AudioRenderer_SourceParameter_Pitch:
//        printf("PITCH: %f\n", value);
        alSourcef(sourceID, AL_PITCH, value);
        break;
      default:
        break;
    }
  }


  // thread main loop

  void OpenALRenderer::operator()() {
    Log(e_Notice, "OpenALRenderer", "operator()()", "Starting OpenALRenderer thread");

    bool quit = false;
    while (!quit) {
      boost::intrusive_ptr<Command> message = messageQueue.WaitForMessage();
      if (!message->Handle(this)) quit = true;
      message.reset();
    }

    Exit();

    Log(e_Notice, "OpenALRenderer", "operator()()", "Shutting down OpenALRenderer thread");

    if (messageQueue.GetPending() > 0) Log(e_Error, "OpenALRenderer", "operator()()", int_to_str(messageQueue.GetPending()) + " messages left on quit!");
  }

}
