// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_ANIMATIONEXTENSION
#define _HPP_ANIMATIONEXTENSION

#include "base/math/vector3.hpp"

namespace blunted {

  class Animation;

  class AnimationExtension {

    public:
      AnimationExtension(Animation *parent) : parent(parent) {};
      virtual ~AnimationExtension() { parent = 0; };

      virtual void Shift(int fromFrame, int offset) = 0;
      virtual void Rotate2D(radian angle) = 0;
      virtual void Mirror() = 0;

      virtual bool GetKeyFrame(int frame, Quaternion &orientation, Vector3 &position, float &power) const = 0;
      virtual void SetKeyFrame(int frame, const Quaternion &orientation, const Vector3 &position = Vector3(0, 0, 0), float power = 1.0) = 0;
      virtual void DeleteKeyFrame(int frame) = 0;

      virtual void Load(std::vector<std::string> &tokenizedLine) = 0;
      virtual void Save(FILE *file) = 0;

    protected:
      Animation *parent;

  };

}

#endif
