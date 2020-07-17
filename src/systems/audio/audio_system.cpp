// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "audio_system.hpp"

#include "audio_scene.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "rendering/audio_messages.hpp"

namespace blunted {

  AudioSystem::AudioSystem() : systemType(e_SystemType_Audio) {
    rendererTask = NULL;
  }

  AudioSystem::~AudioSystem() {
  }

  void AudioSystem::Initialize(const Properties &config) {

    audioSoundBufferResourceManager = boost::shared_ptr < ResourceManager<AudioSoundBuffer> > (new ResourceManager<AudioSoundBuffer>("audiosoundbuffer"));
    ResourceManagerPool::GetInstance().RegisterManager(e_ResourceType_AudioSoundBuffer, audioSoundBufferResourceManager);

    // start thread for renderer
    if (config.Get("audio_renderer", "openal") == "openal") rendererTask = new OpenALRenderer();
    rendererTask->Run();

    boost::intrusive_ptr<AudioRendererMessage_CreateContext> createContext(new AudioRendererMessage_CreateContext());
    rendererTask->messageQueue.PushMessage(createContext);
    createContext->Wait();

    if (!createContext->success) {
      Log(e_FatalError, "AudioSystem", "Initialize", "Could not create context");
    } else {
      Log(e_Notice, "AudioSystem", "Initialize", "Created context");
    }
  }

  void AudioSystem::Exit() {

    audioSoundBufferResourceManager.reset();

    // shutdown renderer thread
    boost::intrusive_ptr<Message_Shutdown> RendererShutdown(new Message_Shutdown());
    rendererTask->messageQueue.PushMessage(RendererShutdown);
    RendererShutdown->Wait();

    rendererTask->Join();
    delete rendererTask;
    rendererTask = NULL;
  }

  e_SystemType AudioSystem::GetSystemType() const {
    return systemType;
  }

  ISystemScene *AudioSystem::CreateSystemScene(boost::shared_ptr<IScene> scene) {
    if (scene->GetSceneType() == e_SceneType_Scene2D) {
      AudioScene *audioScene = new AudioScene(this);
      scene->Attach(audioScene->GetInterpreter(e_SceneType_Scene2D));
      return audioScene;
    }
    if (scene->GetSceneType() == e_SceneType_Scene3D) {
      AudioScene *audioScene = new AudioScene(this);
      scene->Attach(audioScene->GetInterpreter(e_SceneType_Scene3D));
      return audioScene;
    }
    return NULL;
  }

  ISystemTask *AudioSystem::GetTask() {
    return 0;
  }

  AudioRenderer *AudioSystem::GetAudioRenderer() {
    return rendererTask;
  }

}
