// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_STRATEGY_CELEBRATION
#define _HPP_STRATEGY_CELEBRATION

#include "../strategy.hpp"

class CelebrationStrategy : public Strategy {

  public:
    CelebrationStrategy(ElizaController *controller);
    virtual ~CelebrationStrategy();

    virtual void RequestInput(const MentalImage *mentalImage, Vector3 &direction, e_Velocity &velocity);

  protected:
    unsigned long startTime_ms;

};

#endif
