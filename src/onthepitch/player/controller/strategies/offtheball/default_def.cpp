// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "default_def.hpp"

DefaultDefenseStrategy::DefaultDefenseStrategy(ElizaController *controller) : Strategy(controller) {
  name = "default defense";
}

DefaultDefenseStrategy::~DefaultDefenseStrategy() {
}

void DefaultDefenseStrategy::RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity) {

  bool offensiveComponents = true;
  bool defensiveComponents = true;
  bool laziness = true;

  Vector3 desiredPosition_static = team->GetController()->GetAdaptedFormationPosition(CastPlayer(), false);
  Vector3 desiredPosition_dynamic = team->GetController()->GetAdaptedFormationPosition(CastPlayer(), true);
  float actionDistance = NormalizedClamp(player->GetPosition().GetDistance(match->GetDesignatedPossessionPlayer()->GetPosition()), 15.0f, 20.0f);
  float staticPositionBias = curve(1.0f * actionDistance, 1.0f); // lower values = swap position with other players' formation positions more easily
  Vector3 desiredPosition = desiredPosition_static * staticPositionBias + desiredPosition_dynamic * (1.0f - staticPositionBias);

  if (offensiveComponents) {
    // support position
    float attackBias = NormalizedClamp((controller->GetFadingTeamPossessionAmount() - 0.5f) * 1.0f, 0.2f, 0.9f);
    Vector3 supportPosition = controller->GetSupportPosition_ForceField(mentalImage, desiredPosition);
    desiredPosition = desiredPosition * (1.0f - attackBias) + supportPosition * attackBias;
  }

  if (defensiveComponents) {

    float mindset = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);
    controller->AddDefensiveComponent(desiredPosition, pow(clamp(1.9f - mindset - controller->GetFadingTeamPossessionAmount(), 0.0f, 1.0f), 0.7f));

    // offside trap (used to be applied before AddDefensiveComponent)
    team->GetController()->ApplyOffsideTrap(desiredPosition);
  }

  direction = (desiredPosition - player->GetPosition()).GetNormalized(player->GetDirectionVec());
  float desiredVelocity = (desiredPosition - player->GetPosition()).GetLength() * distanceToVelocityMultiplier;

  // laziness
  if (laziness) desiredVelocity = controller->GetLazyVelocity(desiredVelocity);

  desiredVelocity = clamp(desiredVelocity, 0, sprintVelocity);

  velocity = desiredVelocity;
}
