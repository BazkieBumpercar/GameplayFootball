// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SYSTEMS_AUDIO_SCENE
#define _HPP_SYSTEMS_AUDIO_SCENE

#include "defines.hpp"

#include "systems/isystemscene.hpp"

#include "scene/scene3d/scene3d.hpp"
#include "scene/scene2d/scene2d.hpp"

namespace blunted {

  class AudioSystem;

  class AudioScene : public ISystemScene {

    public:
      AudioScene(AudioSystem *audioSystem);
      virtual ~AudioScene();

      virtual AudioSystem *GetAudioSystem();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

      virtual boost::intrusive_ptr<ISceneInterpreter> GetInterpreter(e_SceneType sceneType);

    protected:
      AudioSystem *audioSystem;

  };


  class AudioScene_Scene3DInterpreter : public IScene3DInterpreter {

    public:
      AudioScene_Scene3DInterpreter(AudioScene *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Audio; }
      virtual void OnLoad();
      virtual void OnUnload();

      virtual void SetGravity(const Vector3 &gravity) {}
      virtual void SetErrorCorrection(float value) {}
      virtual void SetConstraintForceMixing(float value) {}

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

    protected:
      AudioScene *caller;

  };

  class AudioScene_Scene2DInterpreter : public IScene2DInterpreter {

    public:
      AudioScene_Scene2DInterpreter(AudioScene *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Audio; }
      virtual void OnLoad();
      virtual void OnUnload();

      virtual ISystemObject *CreateSystemObject(boost::intrusive_ptr<Object> object);

    protected:
      AudioScene *caller;

  };

}

#endif
