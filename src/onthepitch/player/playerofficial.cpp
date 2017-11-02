// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "playerofficial.hpp"

#include "../match.hpp"

#include "controller/refereecontroller.hpp"

#include "../../main.hpp"
#include "../../utils.hpp"

PlayerOfficial::PlayerOfficial(e_OfficialType officialType, Match *match, PlayerData *playerData) : PlayerBase(match, playerData), officialType(officialType) {
}

PlayerOfficial::~PlayerOfficial() {
  if (Verbose()) printf("exiting playerofficial.. ");
  if (Verbose()) printf("done\n");
}

HumanoidBase *PlayerOfficial::CastHumanoid() { return static_cast<HumanoidBase*>(humanoid); }

RefereeController *PlayerOfficial::CastController() {
  return static_cast<RefereeController*>(controller);
}

void PlayerOfficial::Activate(boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection) {
  isActive = true;
  humanoid = new HumanoidBase(this, match, humanoidSourceNode, fullbodySourceNode, colorCoords, animCollection, match->GetDynamicNode(), kit, 0);

  CastHumanoid()->ResetPosition(Vector3(0), Vector3(0));

  controller = new RefereeController(match);
  CastController()->SetPlayer(this);
}

void PlayerOfficial::Deactivate() {
  PlayerBase::Deactivate();
}

void PlayerOfficial::Process() {
  CastController()->Process();
  CastHumanoid()->Process();
}

void PlayerOfficial::PreparePutBuffers(unsigned long snapshotTime_ms) {
  PlayerBase::PreparePutBuffers(snapshotTime_ms);
}

void PlayerOfficial::FetchPutBuffers(unsigned long putTime_ms) {
  PlayerBase::FetchPutBuffers(putTime_ms);
}

