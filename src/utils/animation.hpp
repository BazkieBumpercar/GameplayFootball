// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_ANIMATION
#define _HPP_ANIMATION

#include "defines.hpp"

#include "scene/scene3d/node.hpp"

#include "animationextensions/animationextension.hpp"

#include "utils/xmlloader.hpp"

namespace blunted {

  struct CompareCharacterStrings
  {
    bool operator()(const char *a, const char *b) const {
      return std::strcmp(a, b) < 0;
    }
  };

  struct KeyFrame {
    Quaternion orientation;
    Vector3 position;
  };

  struct WeighedKey {
    KeyFrame keyFrame;
    float influence; // [0..1]
    int frame;
  };

  struct NodeAnimation {
    std::string nodeName;
    std::map<int, KeyFrame> animation; // frame, angles
  };

  enum e_Foot {
    e_Foot_Left,
    e_Foot_Right
  };

  struct BiasedOffset {
    BiasedOffset() {
      bias = 0.0f;
      isRelative = false;
    }
    float bias; // 0 .. 1
    Quaternion orientation;
    bool isRelative;
  };

  static std::map < std::string, BiasedOffset > emptyOffsets;

  struct MovementHistoryEntry {
    std::string nodeName;
    Vector3 position;
    Quaternion orientation;
    int timeDiff_ms;
  };

  typedef std::vector<MovementHistoryEntry> MovementHistory;


  // usage
  //
  // rules:
  //   - first node inserted (by using SetKeyFrame) should be root node of the animated object (for optimization purposes)

  class Animation {

    public:
      Animation();
      Animation(const Animation &src); // attention! this does not deep copy extensions!
      virtual ~Animation();

      void DirtyCache(); // hee hee

      int GetFrameCount() const;
      int GetEffectiveFrameCount() const { return GetFrameCount() - 1; }

      bool GetKeyFrame(std::string nodeName, int frame, Quaternion &orientation, Vector3 &position, bool getOrientation = true, bool getPosition = true) const;
      void SetKeyFrame(std::string nodeName, int frame, const Quaternion &orientation, const Vector3 &position = Vector3(0, 0, 0));
      void DeleteKeyFrame(std::string nodeName, int frame);
      void GetInterpolatedValues(const std::map<int, KeyFrame> &animation, int frame, Quaternion &orientation, Vector3 &position, bool getOrientation = true, bool getPosition = true) const;
      void ConvertToStartFacingForwardIfIdle();
      void Invert();
      void Apply(const std::map < const std::string, boost::intrusive_ptr<Node> > nodeMap, int frame, int timeOffset_ms = 0, bool smooth = true, float smoothFactor = 1.0f, /*const boost::shared_ptr<Animation> previousAnimation, int smoothFrames, */const Vector3 &basePos = Vector3(0), radian baseRot = 0, std::map < std::string, BiasedOffset > &offsets = emptyOffsets, MovementHistory *movementHistory = 0, int timeDiff_ms = 10, bool noPos = false, bool updateSpatial = true);
      void Shift(int fromFrame, int offset);

      // returns end position - start position
      Vector3 GetTranslation() const;
      Vector3 GetIncomingMovement() const;
      float GetIncomingVelocity() const;
      Vector3 GetOutgoingMovement() const;
      Vector3 GetRangedOutgoingMovement() const;
      Vector3 GetOutgoingDirection() const;
      Vector3 GetIncomingBodyDirection() const;
      Vector3 GetOutgoingBodyDirection() const;
      float GetOutgoingVelocity() const;
      radian GetOutgoingAngle() const;
      radian GetIncomingBodyAngle() const;
      radian GetOutgoingBodyAngle() const;
      e_Foot GetCurrentFoot() const { return currentFoot; }
      e_Foot GetOutgoingFoot() const;

      void Reset();
      void LoadData(std::vector < std::vector<std::string> > &file);
      void Load(const std::string &filename);
      void Save(const std::string &filename);
      void Mirror();
      std::string GetName() const;
      void SetName(const std::string &name) { this->name = name; }

      void AddExtension(const std::string &name, boost::shared_ptr<AnimationExtension> extension);
      boost::shared_ptr<AnimationExtension> GetExtension(const std::string &name);

      const std::string &GetVariable(const char *name) const;
      void SetVariable(const std::string &name, const std::string &value);
      const std::string &GetAnimType() const { return cache_AnimType; }
      boost::shared_ptr<XMLTree> GetCustomData();

      // quick edit hax
      void Hax();

      std::vector<NodeAnimation*> &GetNodeAnimations() { return nodeAnimations; }
      std::map < std::string, boost::shared_ptr<AnimationExtension> > &GetExtensions() { return extensions; }

    protected:
      std::vector<NodeAnimation*> nodeAnimations;
      int frameCount;
      std::string name;

      std::map < std::string, boost::shared_ptr<AnimationExtension> > extensions;

      boost::shared_ptr<XMLTree> customData;
      std::map<const char*, std::string, CompareCharacterStrings> variableCache;

      // this hack only applies to humanoids
      // it's which foot is moving first in this anim
      e_Foot currentFoot;

      mutable bool cache_translation_dirty;
      mutable Vector3 cache_translation;
      mutable bool cache_incomingMovement_dirty;
      mutable Vector3 cache_incomingMovement;
      mutable bool cache_incomingVelocity_dirty;
      mutable float cache_incomingVelocity;
      mutable bool cache_outgoingDirection_dirty;
      mutable Vector3 cache_outgoingDirection;
      mutable bool cache_outgoingMovement_dirty;
      mutable Vector3 cache_outgoingMovement;
      mutable bool cache_rangedOutgoingMovement_dirty;
      mutable Vector3 cache_rangedOutgoingMovement;
      mutable bool cache_outgoingVelocity_dirty;
      mutable float cache_outgoingVelocity;
      mutable bool cache_angle_dirty;
      mutable radian cache_angle;
      mutable bool cache_incomingBodyAngle_dirty;
      mutable radian cache_incomingBodyAngle;
      mutable bool cache_outgoingBodyAngle_dirty;
      mutable radian cache_outgoingBodyAngle;
      mutable bool cache_incomingBodyDirection_dirty;
      mutable Vector3 cache_incomingBodyDirection;
      mutable bool cache_outgoingBodyDirection_dirty;
      mutable Vector3 cache_outgoingBodyDirection;

      std::string cache_AnimType;

  };

}

#endif
