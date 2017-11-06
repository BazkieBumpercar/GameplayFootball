// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALL_ONTHEPITCH_ANIMCOLLECTION
#define _HPP_FOOTBALL_ONTHEPITCH_ANIMCOLLECTION

#include "utils/animation.hpp"
#include "../../ball.hpp"

#include "scene/objects/geometry.hpp"
#include "utils/objectloader.hpp"

#include "../../../gamedefines.hpp"

using namespace blunted;

inline radian FixAngle(radian angle);
inline float RangeVelocity(float velocity);
inline float ClampVelocity(float velocity);
inline float FloorVelocity(float velocity);
inline float EnumToFloatVelocity(e_Velocity velocity);
inline e_Velocity FloatToEnumVelocity(float velocity);

enum e_DefString {
  e_DefString_Empty = 0,
  e_DefString_OutgoingSpecialState = 1,
  e_DefString_IncomingSpecialState = 2,
  e_DefString_SpecialVar1 = 3,
  e_DefString_SpecialVar2 = 4,
  e_DefString_Type = 5,
  e_DefString_Trap = 6,
  e_DefString_Deflect = 7,
  e_DefString_Interfere = 8,
  e_DefString_Trip = 9,
  e_DefString_ShortPass = 10,
  e_DefString_LongPass = 11,
  e_DefString_Shot = 12,
  e_DefString_Sliding = 13,
  e_DefString_Movement = 14,
  e_DefString_Special = 15,
  e_DefString_BallControl = 16,
  e_DefString_HighPass = 17,
  e_DefString_Catch = 18,
  e_DefString_OutgoingRetainState = 19,
  e_DefString_IncomingRetainState = 20,
  e_DefString_Size = 21
};

radian FixAngle(radian angle) {
  // convert engine angle into football angle (different base orientation: 'down' on y instead of 'right' on x)
  radian newAngle = angle;
  newAngle += 0.5f * pi;
  return ModulateIntoRange(-pi, pi, newAngle);
}

float RangeVelocity(float velocity) {
  float retVelocity = idleVelocity;
  if (velocity >= idleDribbleSwitch && velocity < dribbleWalkSwitch) retVelocity = dribbleVelocity;
  else if (velocity >= dribbleWalkSwitch && velocity < walkSprintSwitch) retVelocity = walkVelocity;
  else if (velocity >= walkSprintSwitch) retVelocity = sprintVelocity;
  return retVelocity;
}

float ClampVelocity(float velocity) {
  if (velocity < 0) return 0;
  if (velocity > sprintVelocity) return sprintVelocity;
  return velocity;
}

float FloorVelocity(float velocity) {
  float retVelocity = idleVelocity;
  if (velocity > 0 && velocity < dribbleVelocity) retVelocity = dribbleVelocity;
  else if (velocity <= walkVelocity) retVelocity = walkVelocity;
  else retVelocity = sprintVelocity;
  return retVelocity;
}

float EnumToFloatVelocity(e_Velocity velocity) {
  switch (velocity) {
    case e_Velocity_Idle:
      return idleVelocity;
      break;
    case e_Velocity_Dribble:
      return dribbleVelocity;
      break;
    case e_Velocity_Walk:
      return walkVelocity;
      break;
    case e_Velocity_Sprint:
      return sprintVelocity;
      break;
  }
  return 0;
}

e_Velocity FloatToEnumVelocity(float velocity) {
  float rangedVelocity = RangeVelocity(velocity);
  if (rangedVelocity == idleVelocity) return e_Velocity_Idle;
  else if (rangedVelocity == dribbleVelocity) return e_Velocity_Dribble;
  else if (rangedVelocity == walkVelocity) return e_Velocity_Walk;
  else if (rangedVelocity == sprintVelocity) return e_Velocity_Sprint;
  else return e_Velocity_Idle;
}

struct CrudeSelectionQuery {
  CrudeSelectionQuery() {
    byFunctionType = false;
    byFoot = false; foot = e_Foot_Left;
    heedForcedFoot = false; strongFoot = e_Foot_Right;
    bySide = false;
    allowLastDitchAnims = false;
    byIncomingVelocity = false; incomingVelocity_Strict = false; incomingVelocity_NoDribbleToIdle = false; incomingVelocity_NoDribbleToSprint = false; incomingVelocity_ForceLinearity = false;
    byOutgoingVelocity = false;
    byPickupBall = false; pickupBall = true;
    byIncomingBodyDirection = false; incomingBodyDirection_Strict = false; incomingBodyDirection_ForceLinearity = false;
    byIncomingBallDirection = false;
    byOutgoingBallDirection = false;
    byTripType = false;
  }

  bool byFunctionType;
  e_FunctionType functionType;

  bool byFoot;
  e_Foot foot;

  bool heedForcedFoot;
  e_Foot strongFoot;

  bool bySide;
  Vector3 lookAtVecRel;

  bool allowLastDitchAnims;

  bool byIncomingVelocity;
  bool incomingVelocity_Strict; // if true, allow no difference in velocity
  bool incomingVelocity_NoDribbleToIdle;
  bool incomingVelocity_NoDribbleToSprint;
  bool incomingVelocity_ForceLinearity;
  e_Velocity incomingVelocity;

  bool byOutgoingVelocity;
  e_Velocity outgoingVelocity;

  bool byPickupBall;
  bool pickupBall;

  bool byIncomingBodyDirection;
  Vector3 incomingBodyDirection;
  bool incomingBodyDirection_Strict;
  bool incomingBodyDirection_ForceLinearity;

  bool byIncomingBallDirection;
  Vector3 incomingBallDirection;

  bool byOutgoingBallDirection;
  Vector3 outgoingBallDirection;

  bool byTripType;
  int tripType;

  Properties properties;
};

struct Quadrant {
  int id;
  Vector3 position;
  e_Velocity velocity;
  radian angle;
};

void FillNodeMap(boost::intrusive_ptr<Node> targetNode, std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap);

class AnimCollection {

  public:
    // scene3D for debugging pilon
    AnimCollection(boost::shared_ptr<Scene3D> scene3D);
    virtual ~AnimCollection();

    void Clear();
    void Load(boost::filesystem::path directory);

    const std::vector < Animation* > &GetAnimations() const;

    void CrudeSelection(DataSet &dataSet, const CrudeSelectionQuery &query);

    inline Animation* GetAnim(int index) {
      return animations.at(index);
    }

    inline const Quadrant &GetQuadrant(int id) {
      return quadrants.at(id);
    }

    int GetQuadrantID(Animation *animation, const Vector3 &movement, radian angle) const;

  protected:

    void _PrepareAnim(Animation *animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap, bool convertAngledDribbleToWalk = false);

    bool _CheckFunctionType(const std::string &functionType, e_FunctionType queryFunctionType) const;

    boost::shared_ptr<Scene3D> scene3D;

    std::vector<Animation*> animations;
    std::vector<Quadrant> quadrants;

    std::string defString[e_DefString_Size];

    radian maxIncomingBallDirectionDeviation;
    radian maxOutgoingBallDirectionDeviation;

};

#endif
