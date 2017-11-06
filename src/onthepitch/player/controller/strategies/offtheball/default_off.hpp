// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_STRATEGY_DEFAULT_OFFENSE
#define _HPP_STRATEGY_DEFAULT_OFFENSE

#include "../strategy.hpp"

class DefaultOffenseStrategy : public Strategy {

  public:
    DefaultOffenseStrategy(ElizaController *controller);
    virtual ~DefaultOffenseStrategy();

    virtual void RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity);

  protected:

};

#endif
