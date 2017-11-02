// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "default_off.hpp"

#include "../../../../../main.hpp"

DefaultOffenseStrategy::DefaultOffenseStrategy(ElizaController *controller) : Strategy(controller) {
  name = "default offense";
}

DefaultOffenseStrategy::~DefaultOffenseStrategy() {
}

void DefaultOffenseStrategy::RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity) {

  bool offensiveComponents = true;
  bool defensiveComponents = true;
  bool laziness = true;

  Vector3 desiredPosition_static = team->GetController()->GetAdaptedFormationPosition(CastPlayer(), false);
  Vector3 desiredPosition_dynamic = team->GetController()->GetAdaptedFormationPosition(CastPlayer(), true);
  float actionDistance = NormalizedClamp(player->GetPosition().GetDistance(match->GetDesignatedPossessionPlayer()->GetPosition()), 15.0f, 20.0f);
  float staticPositionBias = curve(0.8f * actionDistance, 1.0f); // lower values = swap position with other players' formation positions more easily
  Vector3 desiredPosition = desiredPosition_static * staticPositionBias + desiredPosition_dynamic * (1.0f - staticPositionBias);

  if (GetDebugMode() == e_DebugMode_AI) {
    int scrX, scrY;
    GetDebugOverlayCoord(match, desiredPosition, scrX, scrY);
    Vector3 color(55, 55, 155);
    if (team->GetID() == 1) color.Set(155, 55, 55);
    GetDebugOverlay()->DrawRectangle(scrX - 2, scrY - 2, 4, 4, color);
  }

  if (offensiveComponents) {
    // support position
    float attackBias = NormalizedClamp((controller->GetFadingTeamPossessionAmount() - 0.5f) * 1.0f, 0.1f, 0.6f);
    bool makeRun = false;
    if (attackBias > 0.7f) {
      if (team->GetController()->GetEndApplyAttackingRun_ms() > match->GetActualTime_ms() && team->GetController()->GetAttackingRunPlayer() == player) {
        makeRun = true;
      }
    }
    Vector3 supportPosition = controller->GetSupportPosition_ForceField(mentalImage, desiredPosition, makeRun);
    desiredPosition = desiredPosition * (1.0f - attackBias) + supportPosition * attackBias;
  }

  if (defensiveComponents) {
    float mindset = AI_GetMindSet(CastPlayer()->GetDynamicFormationEntry().role);
    controller->AddDefensiveComponent(desiredPosition, pow(clamp(1.3f - mindset - controller->GetFadingTeamPossessionAmount(), 0.0f, 1.0f), 0.7f));
  }

  if (GetDebugMode() == e_DebugMode_AI) {
    int scrX, scrY;
    GetDebugOverlayCoord(match, desiredPosition, scrX, scrY);
    Vector3 color(155, 155, 255);
    if (team->GetID() == 1) color.Set(255, 155, 155);
    GetDebugOverlay()->DrawRectangle(scrX - 5, scrY - 5, 10, 10, color);
  }

  direction = (desiredPosition - player->GetPosition()).GetNormalized(player->GetDirectionVec());
  float desiredVelocity = (desiredPosition - player->GetPosition()).GetLength() * distanceToVelocityMultiplier;

  // laziness
  if (laziness) desiredVelocity = controller->GetLazyVelocity(desiredVelocity);

  desiredVelocity = clamp(desiredVelocity, 0, sprintVelocity);

  velocity = desiredVelocity;
}
