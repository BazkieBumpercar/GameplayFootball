// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALL_ONTHEPITCH_REFEREECONTROLLER
#define _HPP_FOOTBALL_ONTHEPITCH_REFEREECONTROLLER

#include "icontroller.hpp"

class PlayerOfficial;

class RefereeController : public IController {

  public:
    RefereeController(Match *match);
    virtual ~RefereeController();

    PlayerOfficial *CastPlayer();

    void GetForceField(std::vector<ForceSpot> &forceField);

    virtual void RequestCommand(PlayerCommandQueue &commandQueue);
    virtual void Process();
    virtual Vector3 GetDirection();
    virtual float GetFloatVelocity();

    virtual int GetReactionTime_ms();

    virtual void Reset();

  protected:

};

#endif
