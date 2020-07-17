// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALLANIMATIONEXTENSION
#define _HPP_FOOTBALLANIMATIONEXTENSION

#include "animationextension.hpp"

namespace blunted {

  struct FootballKeyFrame {
    Quaternion orientation;
    Vector3 position;
    float power;
  };

  class FootballAnimationExtension : public AnimationExtension {

    public:
      FootballAnimationExtension(Animation *parent);
      virtual ~FootballAnimationExtension();

      virtual void Shift(int fromFrame, int offset);
      virtual void Rotate2D(radian angle);
      virtual void Mirror();

      virtual bool GetKeyFrame(int frame, Quaternion &orientation, Vector3 &position, float &power) const;
      virtual void SetKeyFrame(int frame, const Quaternion &orientation, const Vector3 &position = Vector3(0, 0, 0), float power = 1.0);
      virtual void DeleteKeyFrame(int frame);

      virtual void Load(std::vector<std::string> &tokenizedLine);
      virtual void Save(FILE *file);

      virtual bool GetFirstTouch(Vector3 &position, int &frame);
      int GetTouchCount() const;
      virtual bool GetTouch(unsigned int num, Vector3 &position, int &frame);
      virtual bool GetTouchPos(int frame, Vector3 &position);

      std::map<int, FootballKeyFrame> &GetAnimation() { return animation; }

    protected:
      std::map<int, FootballKeyFrame> animation;

  };

}

#endif
