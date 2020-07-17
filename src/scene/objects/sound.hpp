// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT_SOUND
#define _HPP_OBJECT_SOUND

#include "defines.hpp"
#include "scene/object.hpp"
#include "types/interpreter.hpp"
#include "base/math/vector3.hpp"
#include "base/geometry/aabb.hpp"
#include "types/resource.hpp"

#include "../resources/soundbuffer.hpp"

namespace blunted {

  class Sound : public Object {

    public:
      Sound(std::string name);
      virtual ~Sound();

      virtual void Exit();

      virtual void SetRadius(float radius);
      virtual float GetRadius() const;

      virtual void SetLoop(bool loop);
      virtual bool GetLoop() const;

      virtual void SetGain(float gain);
      virtual float GetGain() const;

      virtual void SetPitch(float pitch);
      virtual float GetPitch() const;

      void SetSoundBuffer(boost::intrusive_ptr < Resource<SoundBuffer> > soundBuffer);
      boost::intrusive_ptr < Resource<SoundBuffer> > GetSoundBuffer();

      virtual void Poke(e_SystemType targetSystemType);

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

      virtual AABB GetAABB() const;

    protected:
      float radius;
      bool loop;
      float gain;
      float pitch;

      boost::intrusive_ptr < Resource<SoundBuffer> > soundBuffer;

  };

  class ISoundInterpreter : public Interpreter {

    public:
      virtual void OnLoad(boost::intrusive_ptr < Resource<SoundBuffer> > resource) = 0;
      virtual void OnUnload() = 0;
      virtual void OnMove(const Vector3 &position) = 0;
      virtual void OnPoke() = 0;

      virtual void SetRadius(float radius) = 0;
      virtual void SetLoop(bool loop) = 0;
      virtual void SetGain(float gain) = 0;
      virtual void SetPitch(float pitch) = 0;

    protected:

  };

}

#endif
