// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "officials.hpp"

#include "scene/scene3d/scene3d.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "utils/objectloader.hpp"
#include "scene/objectfactory.hpp"

#include "player/playerofficial.hpp"
#include "player/humanoid/humanoidbase.hpp"

#include "../data/playerdata.hpp"

#include "../main.hpp"

Officials::Officials(Match *match, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection) : match(match) {
  ObjectLoader loader;
  boost::intrusive_ptr<Node> playerNode = loader.LoadObject(GetScene3D(), "media/objects/players/player.object");
  playerNode->SetName("player");
  playerNode->SetLocalMode(e_LocalMode_Absolute);

  playerData = new PlayerData();
  referee = new PlayerOfficial(e_OfficialType_Referee, match, playerData);
  linesmen[0] = new PlayerOfficial(e_OfficialType_Linesman, match, playerData);
  linesmen[1] = new PlayerOfficial(e_OfficialType_Linesman, match, playerData);

  referee->Activate(playerNode, fullbodySourceNode, colorCoords, kit, match->GetAnimCollection());
  linesmen[0]->Activate(playerNode, fullbodySourceNode, colorCoords, kit, match->GetAnimCollection());
  linesmen[1]->Activate(playerNode, fullbodySourceNode, colorCoords, kit, match->GetAnimCollection());
  playerNode->Exit();
  playerNode.reset();

  referee->CastHumanoid()->ResetPosition(Vector3(10, -10, 0), Vector3(0));
  linesmen[0]->CastHumanoid()->ResetPosition(Vector3(25, -36.5, 0), Vector3(0));
  linesmen[1]->CastHumanoid()->ResetPosition(Vector3(-25, 36.5, 0), Vector3(0));

  boost::intrusive_ptr < Resource<GeometryData> > geometry = ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch("media/objects/officials/yellowcard.ase", true);
  yellowCard = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("yellowcard", e_ObjectType_Geometry));
  GetScene3D()->CreateSystemObjects(yellowCard);
  yellowCard->SetGeometryData(geometry);
  yellowCard->SetLocalMode(e_LocalMode_Absolute);
  yellowCard->SetPosition(Vector3(0, 0, -10));

  geometry = ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch("media/objects/officials/redcard.ase", true);
  redCard = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("redcard", e_ObjectType_Geometry));
  GetScene3D()->CreateSystemObjects(redCard);
  redCard->SetGeometryData(geometry);
  redCard->SetLocalMode(e_LocalMode_Absolute);
  redCard->SetPosition(Vector3(0, 0, -10));
}

Officials::~Officials() {
  if (Verbose()) printf("exiting officials.. ");
  delete referee;
  delete linesmen[0];
  delete linesmen[1];
  delete playerData;

  redCard.reset();
  yellowCard.reset();
  if (Verbose()) printf("done\n");
}

void Officials::GetPlayers(std::vector<PlayerBase*> &players) {
  players.push_back(referee);
  players.push_back(linesmen[0]);
  players.push_back(linesmen[1]);
}

void Officials::Process() {
  referee->Process();
  linesmen[0]->Process();
  linesmen[1]->Process();
}

void Officials::PreparePutBuffers(unsigned long snapshotTime_ms) {
  referee->PreparePutBuffers(snapshotTime_ms);
  linesmen[0]->PreparePutBuffers(snapshotTime_ms);
  linesmen[1]->PreparePutBuffers(snapshotTime_ms);
}

void Officials::FetchPutBuffers(unsigned long putTime_ms) {
  referee->FetchPutBuffers(putTime_ms);
  linesmen[0]->FetchPutBuffers(putTime_ms);
  linesmen[1]->FetchPutBuffers(putTime_ms);
}

void Officials::Put() {
  referee->Put();
  linesmen[0]->Put();
  linesmen[1]->Put();

  // todo: I don't think it's legal to use all those unbuffered (unmutex'ed) Getters here
  if (referee->GetCurrentFunctionType() == e_FunctionType_Special && (match->GetReferee()->GetCurrentFoulType() == 2 || match->GetReferee()->GetCurrentFoulType() == 3)) {
    std::string bodyPartName = "right_elbow";
    if (referee->GetCurrentAnim()->anim->GetName().find("mirror") != std::string::npos) bodyPartName = "left_elbow";

    const NodeMap &nodeMap = referee->GetNodeMap();
    std::map < const std::string, boost::intrusive_ptr<Node> >::const_iterator bodyPartIter = nodeMap.find(bodyPartName);
    if (bodyPartIter != nodeMap.end()) {
      boost::intrusive_ptr<Spatial> bodyPart = bodyPartIter->second;
      Vector3 position = bodyPart->GetDerivedPosition() + bodyPart->GetDerivedRotation() * Vector3(0.04, 0, -0.25); // -0.4
      if (match->GetReferee()->GetCurrentFoulType() == 2) {
        yellowCard->SetPosition(position);
        yellowCard->SetRotation(bodyPart->GetDerivedRotation());
      } else {
        redCard->SetPosition(position);
        redCard->SetRotation(bodyPart->GetDerivedRotation());
      }
    }
  } else if (referee->GetPreviousFunctionType() == e_FunctionType_Special) {
    yellowCard->SetPosition(Vector3(0, 0, -10));
    redCard->SetPosition(Vector3(0, 0, -10));
  }
}
