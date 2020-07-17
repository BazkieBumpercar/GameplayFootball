// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "audio_scene.hpp"
#include "audio_system.hpp"
#include "audio_object.hpp"

#include "managers/systemmanager.hpp"

#include "objects/audio_audioreceiver.hpp"
#include "objects/audio_sound.hpp"

#include "rendering/audio_messages.hpp"

namespace blunted {

  AudioScene::AudioScene(AudioSystem *audioSystem) : audioSystem(audioSystem) {
  }

  AudioScene::~AudioScene() {
  }

  AudioSystem *AudioScene::GetAudioSystem() {
    return audioSystem;
  }

  boost::intrusive_ptr<ISceneInterpreter> AudioScene::GetInterpreter(e_SceneType sceneType) {
    if (sceneType == e_SceneType_Scene2D) {
      boost::intrusive_ptr<AudioScene_Scene2DInterpreter> scene2DInterpreter(new AudioScene_Scene2DInterpreter(this));
      return scene2DInterpreter;
    }
    if (sceneType == e_SceneType_Scene3D) {
      boost::intrusive_ptr<AudioScene_Scene3DInterpreter> scene3DInterpreter(new AudioScene_Scene3DInterpreter(this));
      return scene3DInterpreter;
    }
    Log(e_FatalError, "AudioScene", "GetInterpreter", "No appropriate interpreter found for this SceneType");
    return boost::intrusive_ptr<AudioScene_Scene3DInterpreter>();
  }

  ISystemObject *AudioScene::CreateSystemObject(boost::intrusive_ptr<Object> object) {
    assert(object.get());

    // todo: system scene shouldn't know about object types, i guess? maybe via a Command
    if (object->GetObjectType() == e_ObjectType_AudioReceiver) {
      AudioAudioReceiver *audioObject = new AudioAudioReceiver(this);
      object->Attach(audioObject->GetInterpreter(e_ObjectType_AudioReceiver));
      return audioObject;
    }
    if (object->GetObjectType() == e_ObjectType_Sound) {
      AudioSound *audioObject = new AudioSound(this);
      object->Attach(audioObject->GetInterpreter(e_ObjectType_Sound));
      return audioObject;
    }

    return NULL;
  }


  // Scene3D interpreter

  AudioScene_Scene3DInterpreter::AudioScene_Scene3DInterpreter(AudioScene *caller) : caller(caller) {
  }

  void AudioScene_Scene3DInterpreter::OnLoad() {
  }

  void AudioScene_Scene3DInterpreter::OnUnload() {
    delete caller;
    caller = 0;
  }

  ISystemObject *AudioScene_Scene3DInterpreter::CreateSystemObject(boost::intrusive_ptr<Object> object) {
    return caller->CreateSystemObject(object);
  }


  // Scene2D interpreter

  AudioScene_Scene2DInterpreter::AudioScene_Scene2DInterpreter(AudioScene *caller) : caller(caller) {
  }

  void AudioScene_Scene2DInterpreter::OnLoad() {
  }

  void AudioScene_Scene2DInterpreter::OnUnload() {
    delete caller;
    caller = 0;
  }

  ISystemObject *AudioScene_Scene2DInterpreter::CreateSystemObject(boost::intrusive_ptr<Object> object) {
    return caller->CreateSystemObject(object);
  }

}
