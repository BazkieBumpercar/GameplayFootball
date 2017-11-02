// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "player.hpp"

#include "../match.hpp"

#include "controller/elizacontroller.hpp"
#include "controller/strategies/strategy.hpp"

#include "../../main.hpp"
#include "../../utils.hpp"

#include "base/geometry/triangle.hpp"

int PlayerBase::playerCount = 0;

PlayerBase::PlayerBase(Match *match, PlayerData *playerData) : match(match), playerData(playerData), id(playerCount++), humanoid(0), controller(0), externalController(0), isActive(false) {
  debug = false;
  lastTouchTime_ms = 0;
  lastTouchType = e_TouchType_None;
  fatigueFactorInv = 1.0;
  confidenceFactor = 1.0;

  averageStat = GetStat("physical_balance") +
                GetStat("physical_reaction") +
                GetStat("physical_acceleration") +
                GetStat("physical_velocity") +
                GetStat("physical_stamina") +
                GetStat("physical_agility") +
                GetStat("physical_shotpower") +
                GetStat("technical_standingtackle") +
                GetStat("technical_slidingtackle") +
                GetStat("technical_ballcontrol") +
                GetStat("technical_dribble") +
                GetStat("technical_shortpass") +
                GetStat("technical_highpass") +
                GetStat("technical_header") +
                GetStat("technical_shot") +
                GetStat("technical_volley") +
                GetStat("mental_calmness") +
                GetStat("mental_workrate") +
                GetStat("mental_resilience") +
                GetStat("mental_defensivepositioning") +
                GetStat("mental_offensivepositioning") +
                GetStat("mental_vision");
  averageStat /= 22.0;

  //if (Verbose()) printf("player '%s' has an average stat of %f\n", playerData->GetLastName().c_str(), averageStat);
  Log(e_Notice, "PlayerBase", "PlayerBase", "player '" + playerData->GetLastName() + "' has an average stat of " + real_to_str(averageStat));

}

PlayerBase::~PlayerBase() {
  if (Verbose()) printf("exiting playerbase.. ");
  if (isActive) Deactivate();
  if (Verbose()) printf("deleting humanoid.. ");
  if (humanoid) delete humanoid;
  if (Verbose()) printf("done\n");
}

void PlayerBase::Deactivate() {
  ResetSituation(GetPosition());

  isActive = false;

  if (humanoid) humanoid->Hide();

  if (externalController) externalController = 0;
  delete controller;
}

IController *PlayerBase::GetController() {
  if (externalController) return externalController;
                     else return controller;
}

void PlayerBase::RequestCommand(PlayerCommandQueue &commandQueue) {
  if (externalController) externalController->RequestCommand(commandQueue);
                     else controller->RequestCommand(commandQueue);
}

void PlayerBase::SetExternalController(IController *externalController) {
  this->externalController = externalController;
  if (this->externalController) {
    this->externalController->Reset();
    this->externalController->SetPlayer(this);
    this->externalController->SetFallbackController(controller);
    //debug = true;
  } else {
    controller->Reset();
    //debug = false;
  }
}

void PlayerBase::SetDebug(bool state) {
  debug = state;
}

bool PlayerBase::GetDebug() const {
  if (IsReleaseVersion()) return false; else return debug;
}

void PlayerBase::Process() {
  if (isActive) {
    if (externalController) externalController->Process(); else controller->Process();
    humanoid->Process();
  } else {
    if (humanoid) humanoid->Hide();
  }
  //if (debug) printf("::%f velo\n", GetMovement().GetLength());
}

void PlayerBase::PreparePutBuffers(unsigned long snapshotTime_ms) {
  humanoid->PreparePutBuffers(snapshotTime_ms);
}

void PlayerBase::FetchPutBuffers(unsigned long putTime_ms) {
  humanoid->FetchPutBuffers(putTime_ms);
}

void PlayerBase::Put() {
  humanoid->Put();
}

float PlayerBase::GetStat(const char *name) const {
  return playerData->GetStat(name);
}

float PlayerBase::GetMaxVelocity() const {
  // see humanoidbase's physics function
  return sprintVelocity * GetVelocityMultiplier();
}

float PlayerBase::GetVelocityMultiplier() const {
  // see humanoid_utils' physics function
  return 0.9f + GetStat("physical_velocity") * 0.1f;
}

float PlayerBase::GetLastTouchBias(int decay_ms, unsigned long time_ms) {
  unsigned long adaptedTime_ms = time_ms;
  if (time_ms == 0) adaptedTime_ms = match->GetActualTime_ms();
  if (decay_ms > 0) return 1.0f - clamp((adaptedTime_ms - GetLastTouchTime_ms()) / (float)decay_ms, 0.0f, 1.0f);
  return 0.0f;
}

void PlayerBase::ResetSituation(const Vector3 &focusPos) {
  positionHistoryPerSecond.clear();
  lastTouchTime_ms = 0;
  lastTouchType = e_TouchType_None;
  if (IsActive()) humanoid->ResetSituation(focusPos);
  if (GetController()) GetController()->Reset();
}
