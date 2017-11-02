// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_STRATEGY_GOALIE_DEFAULT
#define _HPP_STRATEGY_GOALIE_DEFAULT

#include "../strategy.hpp"

class GoalieDefaultStrategy : public Strategy {

  public:
    GoalieDefaultStrategy(ElizaController *controller);
    virtual ~GoalieDefaultStrategy();

    virtual void RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity);
    void CalculateIfBallIsBoundForGoal(const MentalImage *mentalImage);
    bool IsBallBoundForGoal() const { return ballBoundForGoal; }

  protected:
    bool ballBoundForGoal;
    float ballBoundForGoal_ycoord;

};

#endif
