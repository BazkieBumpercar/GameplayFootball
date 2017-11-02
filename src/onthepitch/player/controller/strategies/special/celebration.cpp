// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "celebration.hpp"

CelebrationStrategy::CelebrationStrategy(ElizaController *controller) : Strategy(controller) {
  name = "celebration";
}

CelebrationStrategy::~CelebrationStrategy() {
}

void CelebrationStrategy::RequestInput(const MentalImage *mentalImage, Vector3 &direction, e_Velocity &velocity) {

  direction = player->GetDirectionVec();
  velocity = e_Velocity_Idle;
}
