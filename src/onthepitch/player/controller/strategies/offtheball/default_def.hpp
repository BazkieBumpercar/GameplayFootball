// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_STRATEGY_DEFAULT_DEFENSE
#define _HPP_STRATEGY_DEFAULT_DEFENSE

#include "../strategy.hpp"

class DefaultDefenseStrategy : public Strategy {

  public:
    DefaultDefenseStrategy(ElizaController *controller);
    virtual ~DefaultDefenseStrategy();

    virtual void RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity);

  protected:

};

#endif
