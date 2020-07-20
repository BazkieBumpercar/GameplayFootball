// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "AIfunctions.hpp"

#include <cmath>

#include "mentalimage.hpp"

#include "../match.hpp"
#include "../team.hpp"
#include "../player/player.hpp"
#include "../ball.hpp"

#include "../player/humanoid/humanoid_utils.hpp"

#include "../../main.hpp"

// todo: do we need "match" stuff into MentalImage? especially GetPlayer, at the moment of writing that's the only method used here, so we could
// get rid of the match pointers.


// convert formation position to formation position based on ball position
// deprecated
Vector3 AI_GetAdaptedInitialPos(Match *match, const Vector3 &initialPosition, Vector3 focusPoint, float ballMagnetDistance, float ballMagnetDistancePow) {

  if (focusPoint.coords[2] == -100.0) focusPoint = match->GetBall()->Predict(100).Get2D();
  Vector3 scaledFocusPoint = focusPoint;

  float width = 0.8;
  float depth = 0.4;

  if (scaledFocusPoint.coords[0] > pitchHalfW) scaledFocusPoint.coords[0] = pitchHalfW;
  if (scaledFocusPoint.coords[0] < -pitchHalfW) scaledFocusPoint.coords[0] = -pitchHalfW;
  if (scaledFocusPoint.coords[1] > pitchHalfH) scaledFocusPoint.coords[1] = pitchHalfH;
  if (scaledFocusPoint.coords[1] < -pitchHalfH) scaledFocusPoint.coords[1] = -pitchHalfH;
  scaledFocusPoint *= Vector3(1.0 / pitchHalfW, 1.0 / pitchHalfH, 0); // -1 .. 1

  Vector3 targetPos;
  Vector3 boundedInitialPosition = initialPosition;
  boundedInitialPosition.coords[0] = clamp(boundedInitialPosition.coords[0], -1.0, 1.0);
  boundedInitialPosition.coords[1] = clamp(boundedInitialPosition.coords[1], -1.0, 1.0);

  targetPos = boundedInitialPosition * Vector3(depth, width, 0);
  Vector3 max = Vector3(1.0 - depth, 1.0 - width, 0);
  targetPos += max * scaledFocusPoint;
  targetPos *= Vector3(pitchHalfW, pitchHalfH, 0);

  Vector3 ballMagnetPos = focusPoint;
  Vector3 ballMagnetOffset;
  float ballDistance = (ballMagnetPos - targetPos).GetLength();
  if (ballDistance < ballMagnetDistance) ballMagnetOffset = (ballMagnetPos - targetPos) * pow(((ballMagnetDistance - ballDistance) / ballMagnetDistance), ballMagnetDistancePow);
  targetPos += ballMagnetOffset;

  return targetPos;
}

Vector3 AI_GetAdaptedFormationPosition(Match *match, Player *player, float backXBound, float frontXBound, float lowYBound, float highYBound, float xFocus, float xFocusStrength, float yFocus, float yFocusStrength, const Vector3 &microFocus, float microFocusStrength, float midfieldFocus, float midfieldFocusStrength, bool useDynamicFormationPosition) {
  Team *team = player->GetTeam();
  int side = team->GetSide();

  Vector3 position;

  if (useDynamicFormationPosition) {
    position = player->GetDynamicFormationEntry().position;
  } else {
    position = player->GetFormationEntry().position;
  }

  // stretch midfield into defending or attack position
  if (midfieldFocusStrength > 0.0f) {
    float midfieldPositionFactor = midfieldFocus * 2.0f - 1.0f; // -1 .. 1

    // only for midfielders
    float stretchBias = clamp(1.0f - fabs(position.coords[0] * 1.2f), 0.0f, 1.0f); // overstretch a bit so defenders/attackers are left alone (they usually aren't fully x = 0 or 1)
    stretchBias = curve(stretchBias, 1.0f);

    stretchBias *= midfieldFocusStrength;
    //if (player->GetTeam()->GetID() == 0) printf("==> %f\n", position.coords[0]);
    position.coords[0] = position.coords[0] * (1.0f - stretchBias) + midfieldPositionFactor * stretchBias;
    //if (player->GetTeam()->GetID() == 0) printf("--> %f %f %f\n", position.coords[0], midfieldPositionFactor, stretchBias);
  }

  float xLength = frontXBound - backXBound;
  position.coords[0] = backXBound + (position.coords[0] * 0.5f + 0.5f) * xLength;
  float yLength = highYBound - lowYBound;
  position.coords[1] = lowYBound + (position.coords[1] * -side * 0.5f + 0.5f) * yLength;

  Vector3 purePosition = position;

  if (xFocusStrength > 0.0f) {
    float bias = 1.0f - clamp( fabs(xFocus - position.coords[0]) / fabs(backXBound - frontXBound) , 0.0f, 1.0f);
    bias = -std::cos(bias * pi) * 0.5f + 0.5f;
    bias = std::pow(bias, 0.8);
    bias *= xFocusStrength;
    position.coords[0] = position.coords[0] * (1.0f - bias) + xFocus * bias;
  }

  if (yFocusStrength > 0.0f) {
    float distance = clamp( fabs(yFocus - position.coords[1]) / fabs(highYBound - lowYBound) , 0.0f, 1.0f);

    float bias = 1.0f - distance;
    bias *= 0.2f + 0.8f * fabs(yFocus) / pitchHalfH;
    bias *= yFocusStrength;

    position.coords[1] = position.coords[1] * (1.0f - bias) + yFocus * bias;
  }

  // microfocus
  if (microFocusStrength > 0.0f) {

    float homogeneousYInfluenceBias = 0.2f; // 1.0f == act as if everybody is on the same Y position (so, as if we're in 1D, with only X)
    float homogeneousYPositionBias = 0.4f; // 1.0f == don't influence player's resulting Y position (so, as if we're in 1D, with only X)

    // this way, players are more strictly keeping to their positions
    float distStatic = ((microFocus - purePosition) * Vector3(1, 1.0f - homogeneousYInfluenceBias, 0)).GetLength() / 50.0f;//26.0f;//32.0f;
    // this way, microfocus gets more direct attention from players around the action - they kinda 'forget' their base positions a bit though
    //float distDynamic = (microFocus - player->GetPosition()).GetLength() / 34.0f;

    float dist = distStatic;// * 0.3f + distDynamic * 0.7f;
    //dist = std::min(dist, 0.7f); // always some microfocus (exp)

    if (dist < 1.0f) {
      /*
      // wolfram alpha: (sin((x + 0.5) * pi) * 0.5 + 0.5) * 0.7 + (sin((x - 0.25) * 2.0 * pi) * 0.5 + 0.5) * 0.3 | from x = 0 to 1
      float microFocusBias1 = sin((dist + 0.5f) * pi) * 0.5f + 0.5f; // -\_
      float microFocusBias2 = sin((dist - 0.25f) * 2.0f * pi) * 0.5f + 0.5f; // _/-\_
      // mates in team in possession want to keep some distance for easier pass-to-ability, while defenders just want to jump into the action more
      // todo ^ this is bullshit, right? as in, it's not about the target position, but about the target players to reposition (towards the focus)..
      float sineBias = 0.15f;// + clamp(player->GetTeam()->GetFadingTeamPossessionAmount() - 0.5f, 0.0f, 1.0f) * 0.2f;
      float microFocusBias = microFocusBias1 * (1.0f - sineBias) +
                             microFocusBias2 * sineBias;
      */

      float microFocusBias = 1.0f;

      // less serious when far away
      // we need this, because we use bias to vary between pos and microfocuspos. so for far away players, microfocuspos has way more influence since it's farther away.
      microFocusBias *= 1.0f - dist;

      // more of a binary choice to come over completely or not at all
      microFocusBias = curve(microFocusBias, 0.3f);

      // more bulgy curve
      //microFocusBias = pow(microFocusBias, 0.9f);

      // extra short distance peak
      float peakLocation = 0.15f;
      float peakWidth = 0.25f;
      float peakHeight = 0.1f;
      microFocusBias += (1.0f - NormalizedClamp(fabs(dist - peakLocation), 0.0f, peakWidth)) * peakHeight;
      microFocusBias = clamp(microFocusBias, 0.0f, 1.0f);

      // 'compressor' (wolfram alpha: x, (x^0.7) * 0.7 | from x = 0 to 1)
      // microFocusBias = pow(microFocusBias, 0.7f);
      // microFocusBias = microFocusBias * 0.7f;

      //printf("%f\n", microFocusStrength);
      microFocusBias *= microFocusStrength;
      //microFocusBias = clamp(microFocusBias, 0.0f, 0.1f);

      Vector3 microFocusPosition = microFocus * Vector3(1, 1.0f - homogeneousYPositionBias, 1) + position * Vector3(0, homogeneousYPositionBias, 0);
      position = position * (1.0f - microFocusBias) + microFocusPosition * (microFocusBias);
    }
  }

  return position;
}

float AI_CalculatePassingOdds(Match *match, const Vector3 &origin, const Vector3 &target, const std::vector<PlayerImage> &opponentPlayerImages) {
  float currentOdds = 1.0;

  // draw imaginary line between this and target player
  float targetDistance = (target - origin).GetLength();
  int checkCount = 1 + int(ceil(targetDistance * 1.0));

  Vector3 step = (target - origin) * (1.0 / (float)checkCount) * 0.96; // * 0.96: we don't mind players standing behind target that much anyway

  for (int i = 1; i < checkCount + 1; i++) {

    Vector3 ballPos = origin + step * i;

    float currentDistance = i * (targetDistance / (float)checkCount);
    float maxOpponentDistance = 0.5 + currentDistance * 0.25; // at this distance, opponents start being a threat

    for (int opp = 0; opp < (signed int)opponentPlayerImages.size(); opp++) {
      float opponentDistance = (opponentPlayerImages.at(opp).position - ballPos).GetLength();
      float odds = clamp(opponentDistance, 0, maxOpponentDistance) / maxOpponentDistance;
      if (odds < currentOdds) currentOdds = odds;
    }
  }

  currentOdds *= 1.0;

  return currentOdds;
}

float AI_CalculatePassingOdds(Match *match, PlayerImage thisPlayerImage, PlayerImage targetPlayerImage, const std::vector<PlayerImage> &opponentPlayerImages) {

  // player position predictions
  thisPlayerImage.position = thisPlayerImage.position + thisPlayerImage.directionVec * thisPlayerImage.velocity * 0.1;
  targetPlayerImage.position = targetPlayerImage.position + targetPlayerImage.directionVec * targetPlayerImage.velocity * 0.5; // situation in half a second

  float currentOdds = AI_CalculatePassingOdds(match, thisPlayerImage.position, targetPlayerImage.position, opponentPlayerImages);

  return currentOdds;
}

void AI_GetPassRatings(Match *match, int thisPlayerID, const MentalImage *mentalImage, float opportunism, PassRatings &passRatings) {

  int teamID = match->GetPlayer(thisPlayerID)->GetTeamID();

  PlayerImage thisPlayerImage = mentalImage->GetPlayerImage(thisPlayerID);

  std::vector<PlayerImage> playerImages;
  mentalImage->GetTeamPlayerImages(match->GetPlayer(thisPlayerID)->GetTeamID(), thisPlayerID, playerImages);

  std::vector<PlayerImage> opponentPlayerImages;
  mentalImage->GetTeamPlayerImages(abs(teamID - 1), -1, opponentPlayerImages);
  for (int i = 0; i < (signed int)opponentPlayerImages.size(); i++) {
    opponentPlayerImages.at(i).position = opponentPlayerImages.at(i).position + opponentPlayerImages.at(i).directionVec * opponentPlayerImages.at(i).velocity * 0.3; // situation in half a second
  }

  signed int side = match->GetPlayer(thisPlayerID)->GetTeam()->GetSide();

  for (int i = 0; i < (signed int)playerImages.size(); i++) {

    PlayerImage targetPlayerImage = mentalImage->GetPlayerImage(playerImages.at(i).playerID);

    float bodyDirPenalty = thisPlayerImage.directionVec.GetDotProduct((targetPlayerImage.position - thisPlayerImage.position).GetNormalized(Vector3(side, 0, 0)));
    bodyDirPenalty = clamp(bodyDirPenalty, -1.0, 0.0) + 1; // 0 .. 1
    bodyDirPenalty *= 0.7;
    bodyDirPenalty += 0.3; // 0.3 .. 1.0

    float odds = AI_CalculatePassingOdds(match, thisPlayerImage, targetPlayerImage, opponentPlayerImages);
    odds *= bodyDirPenalty;

    // distance to goal rating
    float goalDistance =     clamp( (Vector3(pitchHalfW * -side, 0, 0) -    thisPlayerImage.position).GetLength() , 8.0, 100.0) * 0.01; // ~= 0.18 .. 1.0
    float mateGoalDistance = clamp( (Vector3(pitchHalfW * -side, 0, 0) - playerImages.at(i).position).GetLength() , 8.0, 100.0) * 0.01; // ~= 0.18 .. 1.0
    float pos = clamp( 0.5 + (goalDistance - mateGoalDistance) * 0.5 , 0.3, 1.0);

    // situational rating
    float sit = AI_GetSituationRating(match, playerImages.at(i).playerID, mentalImage);

    PassRating passRating(playerImages.at(i).playerID, pow(odds, 0.8), pow(pos, 0.7), pow(sit, 0.6));
    passRating.CalculateRating(opportunism);

    passRatings.push_back(passRating);
  }

  std::sort(passRatings.begin(), passRatings.end());
}

float AI_GetSituationRating(Match *match, int thisPlayerID, const MentalImage *mentalImage) {

  float currentSituation = 1.0;
  float safeDistance = 8.0; // bigger == safer

  PlayerImage thisPlayerImage = mentalImage->GetPlayerImage(thisPlayerID);

  std::vector<PlayerImage> opponentPlayerImages;
  mentalImage->GetTeamPlayerImages(abs(match->GetPlayer(thisPlayerID)->GetTeamID() - 1), -1, opponentPlayerImages);

  // player position predictions
  thisPlayerImage.position = thisPlayerImage.position + thisPlayerImage.directionVec * thisPlayerImage.velocity * 0.5; // situation in half a second
  for (int i = 0; i < (signed int)opponentPlayerImages.size(); i++) {
    opponentPlayerImages.at(i).position = opponentPlayerImages.at(i).position + opponentPlayerImages.at(i).directionVec * opponentPlayerImages.at(i).velocity * 0.5; // situation in half a second
    float situation = clamp((opponentPlayerImages.at(i).position - thisPlayerImage.position).GetLength(), 0, safeDistance) / safeDistance;

    situation = pow(situation, 0.5);

    if (situation < currentSituation) currentSituation = situation;
  }

  // penalty for getting out of bounds
  float penalty = 0;
  Vector3 position = thisPlayerImage.position;
  if (fabs(position.coords[0] > 50)) penalty += (fabs(position.coords[0]) - 50);
  if (fabs(position.coords[1] > 32)) penalty += (fabs(position.coords[1]) - 32);
  currentSituation -= penalty;

  currentSituation = clamp(currentSituation, 0.0, 1.0);

  return currentSituation;
}

float AI_CalculateFreeSpace(Match *match, const MentalImage *mentalImage, int teamID, const Vector3 &focusPos, float safeDistance, float futureTime_sec, bool ignoreKeeper) {
  //void AI_GetClosestPlayers(Team *team, const Vector3 &position, bool onlyAIControlled, std::vector<Player*> &result, unsigned int playerCount)
  // todo: only closest?

  assert(mentalImage);

  float currentSituation = 0.0f;

  std::vector<PlayerImage> opponentPlayerImages;
  mentalImage->GetTeamPlayerImages(abs(teamID - 1), -1, opponentPlayerImages);

  // player position predictions
  for (int i = 0; i < (signed int)opponentPlayerImages.size(); i++) {
    if (ignoreKeeper == false || opponentPlayerImages.at(i).dynamicFormationEntry.role != e_PlayerRole_GK) {

      // resulting opp position
      opponentPlayerImages.at(i).position = opponentPlayerImages.at(i).position + opponentPlayerImages.at(i).movement * 0.2f; // slowness
      Vector3 toFocusMovement = (focusPos - opponentPlayerImages.at(i).position).GetNormalized(0) * sprintVelocity * clamp(futureTime_sec - 0.2f, 0.0f, 1000.0f);
      if (toFocusMovement.GetLength() > (focusPos - opponentPlayerImages.at(i).position).GetLength()) toFocusMovement = focusPos - opponentPlayerImages.at(i).position;
      opponentPlayerImages.at(i).position += toFocusMovement;

      float situation = 1.0f - clamp((opponentPlayerImages.at(i).position - focusPos).GetLength(), 0, safeDistance) / safeDistance;

      currentSituation += situation;

    }
  }

  return 1.0f - NormalizedClamp(currentSituation, 0.0f, 2.5f);
}

float AI_GetOffsideLine(Match *match, const MentalImage *mentalImage, int teamID, unsigned int futureSim_ms) {

  signed int side = match->GetTeam(teamID)->GetSide();

  std::vector<PlayerImage> opponentPlayerImages;
  mentalImage->GetTeamPlayerImages(teamID, -1, opponentPlayerImages);

  int dudDeepestOpponent = 0;
  Vector3 deepestOpponentPosition;

  for (int i = 0; i < (signed int)opponentPlayerImages.size(); i++) {

    opponentPlayerImages.at(i).position.coords[0] += opponentPlayerImages.at(i).movement.coords[0] * futureSim_ms * 0.001f;

    // for offside
    if (opponentPlayerImages.at(i).position.coords[0] * side > opponentPlayerImages.at(dudDeepestOpponent).position.coords[0] * side) {
      dudDeepestOpponent = i;
    }
  }

  // offside: we are actually looking for the one-but-deepest opponent (association football rule! EAT THAT, PES6!! :P)
  for (int i = 0; i < (signed int)opponentPlayerImages.size(); i++) {
    if (opponentPlayerImages.at(i).position.coords[0] * side > deepestOpponentPosition.coords[0] * side && i != dudDeepestOpponent) {
      deepestOpponentPosition = opponentPlayerImages.at(i).position;
    }
  }

  float offsideLine = deepestOpponentPosition.coords[0];
  if (mentalImage->GetBallPrediction(0).coords[0] * side > offsideLine * side) {
    offsideLine = mentalImage->GetBallPrediction(0).coords[0];
  }
  if (offsideLine * side < 0) offsideLine = 0.01 * -side;
  offsideLine = clamp(offsideLine, -pitchHalfW, pitchHalfW);

//  if (teamID == 0) SetRedDebugPilon(Vector3(offsideLine, -20, 0));
//  if (teamID == 0) SetBlueDebugPilon(Vector3(offsideLine, 0, 0));
//  if (teamID == 0) SetGreenDebugPilon(Vector3(offsideLine, 20, 0));

  return offsideLine;
}

float veloExpFactor(float velo) {
  return pow(clamp(velo / sprintVelocity, 0.0f, 1.0f), 2.5f) * 3.0f;
}

void AI_GetBestDribbleMovement(Match *match, int thisPlayerID, const MentalImage *mentalImage, Vector3 &desiredDirection, float &desiredVelocity, const TeamTactics &teamTactics) {

  Player *player = match->GetPlayer(thisPlayerID);
  Vector3 myPos = player->GetPosition();
  Vector3 myMov = player->GetMovement();

  float offenseFactor = 0.7f + teamTactics.userProperties.GetReal("dribble_offensiveness", 0.5f) * 0.05f + AI_GetMindSet(player->GetDynamicFormationEntry().role) * 0.05f;
  float powerMultiplier = 1.0f; // should alter (average) resulting velocity

  float future_sec = 0.25f;

  PlayerImage thisPlayerImage = mentalImage->GetPlayerImage(thisPlayerID);

  Team *team = player->GetTeam();
  signed int side = team->GetSide();

  std::vector<PlayerImage> opponentPlayerImages;

  std::vector<Player*> opponents;
  AI_GetClosestPlayers(match->GetTeam(abs(team->GetID() - 1)), myPos, false, opponents, 5);
  for (unsigned int i = 0; i < opponents.size(); i++) {
    opponentPlayerImages.push_back(mentalImage->GetPlayerImage(opponents.at(i)->GetID()));
  }

  float nearBackline = NormalizedClamp(fabs(player->GetPosition().coords[0]) / pitchHalfW, 0.0f, 1.0f);
  float centerModifierInv =
      1.0f -
      std::pow(nearBackline,
               2.0f);  // near the end of the pitch, we want to get inside again
  centerModifierInv *= 0.5f; // stop going to the sides! wtf, todo, why does it prefer the sideline so much (probably because of no opponents :P)
  Vector3 oppGoalPos = Vector3(-side * pitchHalfW, myPos.coords[1] * (1.0f - teamTactics.userProperties.GetReal("dribble_centermagnet", 0.5f)) * centerModifierInv, 0);


  std::vector<ForceSpot> forceField;

  for (unsigned int i = 0; i < opponentPlayerImages.size(); i++) {

    const PlayerImage &oppImg = opponentPlayerImages.at(i);

    ForceSpot spot;
    Vector3 oppPos = oppImg.position + oppImg.movement * future_sec;
    spot.origin = oppPos;
    spot.magnetType = e_MagnetType_Repel;
    spot.decayType = e_DecayType_Variable;
    spot.power = 2.0f * powerMultiplier;//1.0f;
    spot.scale = 10.0f;//16.0f;
    spot.exp = 1.0f;//0.7f;
    forceField.push_back(spot);

    if (GetDebugMode() == e_DebugMode_AI && team->GetID() == 0) {
      int scrX, scrY;
      GetDebugOverlayCoord(match, spot.origin, scrX, scrY);
      //GetDebugOverlay()->DrawLine(line, color, 240);
      Vector3 color(255, 100, 100);
      //GetDebugOverlay()->PutPixel(scrX, scrY, color, 255);
      GetDebugOverlay()->DrawRectangle(scrX - 1, scrY - 1, 3, 3, color, 255);
    }
  }

  // sideline / backline
  {
    ForceSpot spot;
    spot.origin = Vector3(myPos.coords[0], (pitchHalfH + 5.0f) * signSide(myPos.coords[1]), 0);
    spot.magnetType = e_MagnetType_Repel;
    spot.decayType = e_DecayType_Variable;
    spot.power = 4.0f * powerMultiplier;
    spot.scale = 20.0f;
    spot.exp = 0.7f;
    forceField.push_back(spot);

    spot.origin = Vector3((pitchHalfW + 5.0f) * signSide(myPos.coords[0]), myPos.coords[1], 0);
    spot.magnetType = e_MagnetType_Repel;
    spot.decayType = e_DecayType_Variable;
    spot.power = 4.0f * powerMultiplier;
    spot.scale = 20.0f;
    spot.exp = 0.7f;
    forceField.push_back(spot);
  }

  // love for da goal
  {
    ForceSpot spot;
    spot.origin = oppGoalPos;
    spot.magnetType = e_MagnetType_Attract;
    spot.decayType = e_DecayType_Constant;
    spot.power = offenseFactor * powerMultiplier;
    forceField.push_back(spot);
  }

  //Vector3 forceFieldPosition = AI_GetForceFieldPosition(forceField, myPos);
  Vector3 forceFieldMovement = AI_GetForceFieldMovement(forceField, myPos + myMov * future_sec, 1.0f);

  desiredDirection = forceFieldMovement.GetNormalized(player->GetDirectionVec());
  desiredVelocity = clamp(forceFieldMovement.GetLength() * distanceToVelocityMultiplier, idleVelocity, sprintVelocity);
  desiredVelocity = RangeVelocity(desiredVelocity);
}

Vector3 AI_GetForceFieldMovement(const std::vector<ForceSpot> &forceField, const Vector3 &currentPos, float attractorDampingDistance) {

  // attractorDampingDistance: from this distance to attractor, dampen influence so we won't overshoot target

  Vector3 cumulVec;
  float cumulForce = 0.0f;

  for (unsigned int i = 0; i < forceField.size(); i++) {

    const ForceSpot &forceSpot = forceField.at(i);

    float distance, intensity;

    distance = (forceSpot.origin - currentPos).GetLength();
    if (forceSpot.decayType == e_DecayType_Constant) {
      intensity = 1.0f;
    } else {
      intensity = clamp(1.0f - distance / forceSpot.scale, 0.0f, 1.0f);
      if (forceSpot.exp != 1.0f) intensity = std::pow(intensity, forceSpot.exp);
    }
    if (intensity > 0.0f) {
      Vector3 relativeOrigin = forceSpot.origin - currentPos;
      relativeOrigin.Normalize(0);
      if (forceSpot.magnetType == e_MagnetType_Repel) {
        relativeOrigin = -relativeOrigin;//currentPos - forceSpot.origin;
      } else {
        // attractors need damping
        if (distance < attractorDampingDistance) relativeOrigin *= distance / attractorDampingDistance;
      }

      float force = forceSpot.power * intensity;

      cumulVec += relativeOrigin * force;
      cumulForce += force;
    }

  }

  if (cumulForce == 0.0f) return 0; else return (cumulVec / cumulForce) * sprintVelocity;
}

TimeNeeded AI_GetTimeNeededForDistance_ms(const Vector3 &playerPos, const Vector3 &playerMovement, const Vector3 &targetPos, float maxVelocity, bool precise, int maxTime_ms, bool debug) {

  TimeNeeded result;

  float optimizeDist = 16.0f;
  if (precise) optimizeDist = 48.0f;

  float initialDist = (playerPos - targetPos).GetLength();
  unsigned int defaultOptimizedTime_ms = int(
      std::round((targetPos - (playerPos + playerMovement * 0.2f)).GetLength() /
                 (maxVelocity * 0.75f) * 1000));
  if (initialDist > optimizeDist) {
    result.usual_ms = defaultOptimizedTime_ms;
    result.optimistic_ms = result.usual_ms - 200;
    return result;
  }

  // definitive best version of all versions
  // The One Version
  // or else..

  Vector3 currentPos = playerPos;
  Vector3 currentMovement = playerMovement;

  float ffo = 0.1f; // in front of foot offset (ideal ball position)
  if (currentMovement.GetLength() > idleDribbleSwitch) {
    currentPos += currentMovement.GetNormalized() * ffo;
    currentPos += currentMovement * 0.01f;
  } else {
    currentPos += (targetPos - playerPos).GetNormalized(0) * ffo;
  }

  unsigned int currentTime_ms = 0;
  const unsigned int timeStep_ms = 10;
  const unsigned int changeTime_ms = 700;
  float radius_usual = 0.28f; // starting distance from our base position where we can reach balls (effectively: leg extension length)
  float radius_optimistic = 0.9f;
  float resultingRadius_usual = radius_usual; // initial value > 0 because we don't want division by zero
  bool foundOptimisticTime = false;

  float adaptedMaxVelocity = maxVelocity * 0.94f; // don't use full maxvelocity, since the last part of that velo is very hard to attain (due to exponential air resistance)

  while (true) { // =]

    // too unstable! timeStep_ms = clamp(int(round(previousDistance * 30)) - 20, 10, 40); // variable timestep may not be 100% correct, so don't overdo it
    // round to 10s
    //timeStep_ms = int(floor(timeStep_ms / 10.0f)) * 10;

    float bias = clamp((float)currentTime_ms / (float)changeTime_ms, 0.0f, 1.0f);
    //bias = pow(bias, 1.7f); // higher exp == slower
    //bias = 0.1f + pow(bias, 0.8f) * 0.9f; // higher exp == slower
    //bias = 0.01f + pow(bias, 1.0f) * 0.99f; // higher exp == slower
    //bias = 0.1f + bias * 0.9f;

    if (bias >= 1.0f) {

      // we can now simply calculate the time needed
      float remainingDistance_usual = clamp((targetPos - currentPos).GetLength() - radius_usual, 0.0f, 100000.0f);
      result.usual_ms = currentTime_ms + (remainingDistance_usual / adaptedMaxVelocity) * 1000;
      resultingRadius_usual = radius_usual + remainingDistance_usual;

      if (!foundOptimisticTime) {
        float remainingDistance_optimistic = clamp((targetPos - currentPos).GetLength() - radius_optimistic, 0.0f, 100000.0f);
        result.optimistic_ms = currentTime_ms + (remainingDistance_optimistic / adaptedMaxVelocity) * 1000;
      }

      break;

    } else {

      // manual copy: this function is called a lot, so if we use the Vector3::operator*, it's creating a lot of temp vars.
      // currentMovement = playerMovement * (1.0f - bias);
      currentMovement.coords[0] = playerMovement.coords[0] * (1.0f - bias);
      currentMovement.coords[1] = playerMovement.coords[1] * (1.0f - bias);

      //currentPos += currentMovement * timeStep_ms * 0.001f;
      currentPos.coords[0] += currentMovement.coords[0] * timeStep_ms * 0.001f;
      currentPos.coords[1] += currentMovement.coords[1] * timeStep_ms * 0.001f;

      // within this radius, we can get to a ball
      radius_usual += adaptedMaxVelocity * bias * timeStep_ms * 0.001f;
      radius_optimistic += adaptedMaxVelocity * bias * timeStep_ms * 0.001f;

      float targetDistance = (targetPos - currentPos).GetLength();
      //if (currentTime_ms > 1000 && currentTime_ms % 100 == 0) printf("currentTime_ms: %i, targetDistance: %f, currentMovementLength: %f, bias: %f, radius: %f, changeTime_ms: %i\n", currentTime_ms, targetDistance, currentMovement.GetLength(), bias, radius, changeTime_ms);
      //if (debug) if (maxTime_ms != -1 && currentTime_ms == (unsigned int)maxTime_ms) printf("CANNOT GO FURTHER: %i\n", currentTime_ms);
      if ((targetDistance < radius_optimistic || (maxTime_ms != -1 && currentTime_ms > (unsigned int)maxTime_ms)) && !foundOptimisticTime) {
        result.optimistic_ms = currentTime_ms;
        foundOptimisticTime = true;
      }
      if (targetDistance < radius_usual || (maxTime_ms != -1 && currentTime_ms > (unsigned int)maxTime_ms)) {
        //currentTime_ms += int(round(((targetPos - currentPos).GetLength() / radius) * 40.0));
        //if (debug) printf("ACTUALLY FOUND AT: %i\n", currentTime_ms);
        resultingRadius_usual = radius_usual;
        result.usual_ms = currentTime_ms;
        break;
      }

    }

    currentTime_ms += timeStep_ms;
  }

  if (maxTime_ms != -1 && currentTime_ms > (unsigned int)maxTime_ms) {
    result.usual_ms = std::max(defaultOptimizedTime_ms, (currentTime_ms + 100) * 2);
    if (!foundOptimisticTime) result.optimistic_ms = result.usual_ms;
    return result;
  }

  // very, very close! just take distance as time, so we can still compare to other players properly
  if (result.usual_ms == 0) {
    result.usual_ms = int(std::round(
        clamp((targetPos - playerPos).GetLength() / resultingRadius_usual, 0.0f,
              1.0f) *
        10));
    result.optimistic_ms = result.usual_ms;
  }

  return result;

  /* too simple version
  return int(round((targetPos - (playerPos + playerMovement * 0.02)).GetLength() / (sprintVelocity * 0.9) * 1000));
  */
}

unsigned int AI_GetToBallMovement(Match *match, const MentalImage *mentalImage, Player *player, const Vector3 &desiredDirection, float desiredVelocityFloat, Vector3 &bestDirection, float &bestVelocityFloat, Vector3 &bestLookAt, float haste) {

  Vector3 playerPos = player->GetPosition();

  float movementWeight = 1.0f;
  float timeWeight = 0.0f;
  float perpendicularWeight = 0.1f;
  float previousTargetWeight = 0.0f;

  Vector3 adaptedDesiredDirection = desiredDirection;


  // precalc shortest distance to ball 'line' for perpendicularity rating (so we have a minimum value there)

  Line ballLine(mentalImage->GetBallPrediction(0).Get2D(), mentalImage->GetBallPrediction(1000).Get2D());
  float u = 0;
  float playerBallLineShortestDistance = ballLine.GetDistanceToPoint(playerPos, u);
  Vector3 playerBallShortestTargetPos = ballLine.GetVertex(0) + (ballLine.GetVertex(1) - ballLine.GetVertex(0)) * u;


  // quantize input 8- or 16-way in relation to 'ball direction space'

  Vector3 ballSpaceDirection = (mentalImage->GetBallPrediction(0).Get2D() - mentalImage->GetBallPrediction(1000).Get2D()).GetNormalized(adaptedDesiredDirection);
  radian toBallSpaceAngle = ballSpaceDirection.GetAngle2D();
  Vector3 adaptedDesiredDirectionBallSpace = adaptedDesiredDirection.GetRotated2D(-toBallSpaceAngle);

  int directions = 16;

  radian angle = adaptedDesiredDirectionBallSpace.GetAngle2D();
  angle /= pi * 2.0f;
  angle = std::round(angle * directions);
  angle /= directions;
  angle *= pi * 2.0f;

  float bias = 1.0f;
  adaptedDesiredDirectionBallSpace = (adaptedDesiredDirectionBallSpace * (1.0 - bias) + (Vector3(1, 0, 0).GetRotated2D(angle) * bias)).GetNormalized(adaptedDesiredDirectionBallSpace);

  adaptedDesiredDirection = adaptedDesiredDirectionBallSpace.GetRotated2D(toBallSpaceAngle);

  //printf("ball angle: %f, result angle: %f\n", ballSpaceDirection.GetAngle2D(), adaptedDesiredDirection.GetAngle2D());


  Vector3 desiredMovement = adaptedDesiredDirection * desiredVelocityFloat;

/*
  // only use player desired movement if it's a lot different from perpendicular
  float desiredVsPerpendicularDot = adaptedDesiredDirection.GetDotProduct((playerBallShortestTargetPos - playerPos).GetNormalized(0));
  //if (desiredVsPerpendicularDot < 0.3f) movementWeight = 1.0f; else perpendicularWeight = 1.0f;
  float desiredVsPerpendicularBias = pow(NormalizedClamp(desiredVsPerpendicularDot, 0.0f, 1.0f), 0.5f);
  // if (player->GetDebug()) {
  //   SetGreenDebugPilon(playerPos + (playerBallShortestTargetPos - playerPos).GetNormalized(0) * 5.0f);
  //   SetYellowDebugPilon(playerPos + adaptedDesiredDirection.GetNormalized(0) * 5.0f);
  //   printf("desired vs perpendicular bias: %f\n", desiredVsPerpendicularBias);
  // }
  desiredVsPerpendicularBias = desiredVsPerpendicularBias * 0.5f + 0.5f;
  movementWeight = (1.0f - desiredVsPerpendicularBias);
  perpendicularWeight = desiredVsPerpendicularBias;
*/

  if (haste > 0.0f) {
    movementWeight = 0.0f;
    timeWeight = 1.0f;
    perpendicularWeight = 0.0f;
    /*
    movementWeight *= (1.0f - haste);
    timeWeight = timeWeight * (1.0f - haste) + haste;
    //effectiveTimeWeight += 1.0f; // recenlty added, testing
    perpendicularWeight *= (1.0f - haste);
    */
  }

  struct PossibleChoice {
    PossibleChoice() {
      rating = -10000.0f;
      time_ms = -10;
      timeNeeded_ms = -10;
    }
    float rating;
    int timeNeeded_ms;
    int time_ms;
  };

  //if (player->GetDebug()) SetBlueDebugPilon(playerPos + desiredMovement);

  PossibleChoice bestChoice;

  unsigned int startTime_ms = clamp(player->GetTimeNeededToGetToBall_ms(), 40, ballPredictionSize_ms - 10);
  // we can't start optimized at a later moment, because that would mean we'd aim outside the pitch to start with
  Vector3 ballPrediction = mentalImage->GetBallPrediction(startTime_ms);
  if (fabs(ballPrediction.coords[0]) > pitchHalfW - 0.2f ||
      fabs(ballPrediction.coords[1]) > pitchHalfH - 0.2f) {
    startTime_ms = 80;
  }

  int previousDesiredTargetTime_ms = player->GetDesiredTimeToBall_ms();
  int timeNeededToGetToBall_ms = player->GetTimeNeededToGetToBall_ms();

  float ffoLength = GetFrontOfFootOffsetRel(desiredVelocityFloat * 0.2f + player->GetFloatVelocity() * 0.8f, 0, 0).GetLength();

  Vector3 ballMovementRough = (mentalImage->GetBallPrediction(player->GetTimeNeededToGetToBall_ms() + 10) - mentalImage->GetBallPrediction(player->GetTimeNeededToGetToBall_ms())).Get2D() * 100.0f;
  float desiredBallDot = ballMovementRough.GetNormalized(0).GetDotProduct(adaptedDesiredDirection);
  Vector3 ballDirectionRough = ballMovementRough.GetNormalized(adaptedDesiredDirection);

  unsigned int timeStep = 1;

  for (unsigned int time_ms = startTime_ms; time_ms < ballPredictionSize_ms; time_ms += 10 * timeStep) {

    bool forced = false;

    Vector3 targetPos = mentalImage->GetBallPrediction(time_ms);
    if (fabs(targetPos.coords[0]) > pitchHalfW - 0.2f ||
        fabs(targetPos.coords[1]) > pitchHalfH - 0.2f) {
      forced = true;
    }
    if (!forced) if (targetPos.coords[2] >= 1.0f) continue; // unattainable
    targetPos = targetPos.Get2D();

    TimeNeeded timeNeeded = AI_GetTimeNeededForDistance_ms(playerPos, player->GetMovement(), targetPos, player->GetMaxVelocity(), true, time_ms);
    unsigned int timeNeeded_ms = timeNeeded.usual_ms;

    timeStep = std::min(
        std::max(
            int(std::round((signed int)(timeNeeded_ms - time_ms) * 0.05f)) - 5,
            1),
        10);  // if ball is going to be too far away anyway, optimize by
              // skipping 'frames'
    //printf("timestep: %u, %u, %u\n", timeStep, timeNeeded_ms, time_ms);

    if (timeNeeded_ms <= time_ms) {

      // ball will ideally be slightly in front of us, simulate this.
      //targetPos -= (targetPos - playerPos).GetNormalizedMax(1.0f) * 0.25f;
      // todo: for this to work, it would have to only remove distance perpendicular to ballmovement
      //targetPos -= (targetPos - playerPos).GetNormalizedMax(ffoLength);// 0.25f;

      float dot = (targetPos - playerPos).GetNormalized(0).GetDotProduct(ballDirectionRough);
      /* to do this, we need to change the look at stuff below too, else we will walk backwards with a strange body dir sometimes (i think)
      if (dot >= 0.6f) forced = true;
      //Vector3 ballMovement = mentalImage->GetBallPrediction(time_ms + 10) - mentalImage->GetBallPrediction(time_ms);
      if (ballMovementRough.GetLength() < sprintVelocity * 2.0f) {
        if (dot >= 0.4f) forced = true; // when ball goes slower, allow less walking in ball movement direction
      }
      */
      //if (dot >= 0.0f) forced = true; // force <= perpendicular
      float justInTimeFactor = clamp(player->GetTimeNeededToGetToBall_ms() / (float)time_ms, 0.0f, 1.0f); // lower == more time to spare
      //if (player->GetDebug()) printf("justInTimeFactor: %f\n", justInTimeFactor);
      if (justInTimeFactor < 0.35f) forced = true; // takes too long relative to how long it could take
      //LANCHANGE if (dot > 0.45f) forced = true; // too shallow angle, just go to ball already
      if (dot > 0.0f) forced = true; // too shallow angle, just go to ball already
      //if (fabs(angle) <= 0.21f * pi) forced = true;


      // --- heed desired direction

      float targetDistance = (targetPos - playerPos).GetLength();
      float targetVelocity = clamp(targetDistance * distanceToVelocityMultiplier, idleVelocity, sprintVelocity);
      Vector3 targetMovement = (targetPos - playerPos).GetNormalized(0) * targetVelocity;
      float movementRating = 1.0f - NormalizedClamp((desiredMovement - targetMovement).GetLength(), 0.0f, sprintVelocity); // > sprintvelocity will often get us farther from where we want to go to
      float directionRating = 1.0f - NormalizedClamp(fabs(adaptedDesiredDirection.GetAngle2D(targetMovement.GetNormalized(adaptedDesiredDirection))), 0.0f, 0.5f * pi); // > 0.5f * pi will often only get us farther from where we want to go to
      movementRating = movementRating * 0.4f + directionRating * 0.6f; // directionrating omits velocity and therefore has another quality


      // --- less time is better?

      float timeRating = 1.0f - NormalizedClamp(time_ms, 0, 5000);


      // --- rate perpendicularity targetmovement to ballmovement vector

      float perpendicularRating = 1.0f - NormalizedClamp((playerBallShortestTargetPos - targetPos).GetLength(), 0, 2.0f * sprintVelocity);


      // --- rate similarity to player's previous preferred target

      float previousTargetRating = 0.0f;
      if (previousDesiredTargetTime_ms >= timeNeededToGetToBall_ms) {
        previousTargetRating = 1.0f - abs(previousDesiredTargetTime_ms - timeNeededToGetToBall_ms) / 500.0f;
      }

      float rating = movementRating * movementWeight +
                     timeRating * timeWeight +
                     perpendicularRating * perpendicularWeight +
                     previousTargetRating * previousTargetWeight;

      if (rating > bestChoice.rating || (forced && bestChoice.time_ms == -10)) {
        bestChoice.rating = rating;
        bestChoice.time_ms = time_ms;
        bestChoice.timeNeeded_ms = timeNeeded_ms;
      }

    } // timeNeeded_ms <= time_ms

    if (forced) {
      if (bestChoice.time_ms == -10) { // this happens if timeNeeded_ms > time_ms
        bestChoice.time_ms = time_ms;
        bestChoice.timeNeeded_ms = time_ms; // just fake it - get as close as possible
      }
      break;
    }
  }

  if (bestChoice.time_ms == -10) {
    bestChoice.time_ms = ballPredictionSize_ms - 10;
    bestChoice.timeNeeded_ms = ballPredictionSize_ms - 10;
  }

  Vector3 targetPos = mentalImage->GetBallPrediction(bestChoice.time_ms).Get2D();
  float distance = (targetPos - playerPos).GetLength();
  bestDirection = (targetPos - playerPos).GetNormalized(player->GetDirectionVec());


  // velocity

  float bestVelocityRelaxed = clamp(distance * ((bestChoice.timeNeeded_ms + defaultTouchOffset_ms) / (float)bestChoice.time_ms) * distanceToVelocityMultiplier, idleVelocity, sprintVelocity);
  float bestVelocityTimeBased = clamp((bestChoice.timeNeeded_ms / (float)bestChoice.time_ms) * player->GetMaxVelocity(), idleVelocity, sprintVelocity);
  float bestVelocityASAP = clamp(distance * distanceToVelocityMultiplier, idleVelocity, sprintVelocity);


  // simple version
  bestVelocityFloat = bestVelocityTimeBased;// bestVelocityRelaxed;
  // maybe just for sprinting? if (bestChoice.timeNeeded_ms > 500 && desiredVelocityFloat > bestVelocityFloat) bestVelocityFloat = bestVelocityFloat * 0.5f + desiredVelocityFloat * 0.5f;//sprintVelocity;
  //if (distance > 0.25f * sprintVelocity) bestVelocityFloat = clamp(desiredVelocityFloat, bestVelocityFloat, sprintVelocity); // allow premature arrival at target
  //if (bestVelocityFloat < desiredVelocityFloat && bestVelocityASAP <= desiredVelocityFloat) bestVelocityFloat = bestVelocityASAP;
  // stick to current velo, to prevent switching back and forth
  if (bestChoice.time_ms > 250) bestVelocityFloat = bestVelocityFloat * 0.97f + player->GetFloatVelocity() * 0.03f;


  // lookat dir

  Vector3 lookAtBall = (mentalImage->GetBallPrediction(20).Get2D() - playerPos).GetNormalized(adaptedDesiredDirection);
  Vector3 lookAtDirection = bestDirection;
  Vector3 lookAtDesiredDir;
  // if we're close to target, allow looking in desired direction more. if we are farther away, we don't want to walk backwards yet, it makes no sense.
  if (bestChoice.time_ms < 100) {
    // these were desiredDirection first, instead of adapted
    lookAtDesiredDir = adaptedDesiredDirection;
  } else {
    if (FloatToEnumVelocity(bestVelocityFloat) != e_Velocity_Idle) {
      lookAtDesiredDir = adaptedDesiredDirection.GetClamped2D(lookAtBall, lookAtDirection);
    } else {
      lookAtDesiredDir = adaptedDesiredDirection;
    }
  }

  // combine!
  bestLookAt = playerPos + lookAtBall.GetRotated2D(clamp(lookAtDesiredDir.GetAngle2D(lookAtBall) * 1.0f, -0.25f * pi, 0.25f * pi)) * 10.0f;
  // if (player->GetDebug()) SetYellowDebugPilon(playerPos + lookAtBall * 5.0f);
  // if (player->GetDebug()) SetRedDebugPilon(playerPos + lookAtDesiredDir * 5.0f);

  return bestChoice.time_ms;
}

unsigned int AI_GetBallControlMovement(const MentalImage *mentalImage, Player *player, const Vector3 &desiredDirection, float desiredVelocityFloat, Vector3 &bestDirection, float &bestVelocityFloat, Vector3 &bestLookAt) {

  unsigned int desiredTimeToBall_ms = 250 + defaultTouchOffset_ms;

  Vector3 toBallMovement = mentalImage->GetBallPrediction(desiredTimeToBall_ms).Get2D() - player->GetPosition();
  float toBallDistance = toBallMovement.GetLength();

  // this should get rid of short distance artifacts
  float manualDirectionStartDistanceThreshold = 0.2f;
  float manualDirectionEndDistanceThreshold = 0.4f;
  float autoDirectionBias = 1.0f;
  if (toBallDistance < manualDirectionEndDistanceThreshold) {
    autoDirectionBias = std::pow(
        NormalizedClamp(toBallDistance, manualDirectionStartDistanceThreshold,
                        manualDirectionEndDistanceThreshold),
        0.5f);
  }

  Vector3 autoDirection = toBallMovement.GetNormalized(player->GetDirectionVec());
  Vector3 manualDirection = player->GetDirectionVec();//desiredDirection;
  // test this: if (player->GetDirectionVec().GetDotProduct(desiredDirection) < 0) manualDirection = desiredDirection;

  bestDirection = autoDirection * autoDirectionBias + manualDirection * (1.0f - autoDirectionBias);
  bestDirection.Normalize(player->GetDirectionVec());
  //SetRedDebugPilon(player->GetPosition() + bestDirection * 2.0f);


  // look direction
  Vector3 bestLookDirection = bestDirection;

  float toBallVelocity = toBallDistance * distanceToVelocityMultiplier;
  bestVelocityFloat = toBallVelocity;

  //bestVelocityFloat = bestVelocityFloat * 0.5f + RangeVelocity(bestVelocityFloat) * 0.5f; // quantization is the root of all happiness
  if (bestVelocityFloat < dribbleVelocity) { // don't quantize low velos
    //bestVelocityFloat = idleVelocity;
  } else {
    float clampedDesiredVelocityFloat = clamp(desiredVelocityFloat, bestVelocityFloat, bestVelocityFloat + 8.0f);
    bestVelocityFloat = clampedDesiredVelocityFloat;
    if (RangeVelocity(bestVelocityFloat) < bestVelocityFloat) bestVelocityFloat = bestVelocityFloat * 0.9f + RangeVelocity(bestVelocityFloat) * 0.1f;
    if (RangeVelocity(bestVelocityFloat) > bestVelocityFloat) bestVelocityFloat = bestVelocityFloat * 0.1f + RangeVelocity(bestVelocityFloat) * 0.9f;
  }

  bestLookAt = player->GetPosition() + bestLookDirection * 10.0f;

  return player->GetTimeNeededToGetToBall_ms();
}

bool AI_HasPossession(Ball *ball, Player *player) {
  Vector3 playerMovement = player->GetMovement();

  // premature optimization ;)
  if ((player->GetPosition() - ball->Predict(0)).GetLength() > 5.0) return false;

  Vector3 ballMovement = ball->GetMovement();
  if (fabs(ball->Predict(0).coords[2]) > 0.5) return false;


  bool distanceOK = true;
  float radius = 1.0f;
  Vector3 center = player->GetPosition() + player->GetMovement() * 0.05f + player->GetDirectionVec() * 0.1f;//was: 0.05f
  if ((ball->Predict(0).Get2D() - center).GetLength() > radius) distanceOK = false;

  bool movementOK = true;
  Vector3 ballMovement3D = (ball->Predict(10) - ball->Predict(0)) * 100.0f;
  if ((ballMovement3D - playerMovement).GetLength() > 6.0f) movementOK = false;

  if (distanceOK && movementOK) return true; else return false;
}

float AI_GetTeamPossessionFactor(Match *match, Team *team) {
  int t1 = team->GetTimeNeededToGetToBall_ms();
  int t2 = match->GetTeam(abs(team->GetID() - 1))->GetTimeNeededToGetToBall_ms();
  signed int amount = t2 - t1;

  float factor = (float)amount / 5000.0; // ~ -1 .. 1
  factor = clamp(factor * 0.5 + 0.5, 0.0, 1.0); // 0 .. 1

  return factor;
}

Player *AI_GetClosestPlayer(Team *team, const Vector3 &position, bool onlyAIControlled, Player *except) {
  const std::vector<Player*> &players = team->GetAllPlayers();

  float closestDistance = 10000;
  Player *closestPlayer = 0;

  for (unsigned int i = 0; i < players.size(); i++) {
    if (players.at(i)->IsActive() && players.at(i) != except) {
      float distance = (players.at(i)->GetPosition() - position).GetLength();
      if (distance < closestDistance) {
        if (!onlyAIControlled || !team->IsHumanControlled(players.at(i)->GetID())) {
          closestDistance = distance;
          closestPlayer = players.at(i);
        }
      }
    }
  }

  return closestPlayer;
}

void AI_GetClosestPlayers(Team *team, const Vector3 &position, bool onlyAIControlled, std::vector<Player*> &result, unsigned int playerCount) {
  const std::vector<Player*> &players = team->GetAllPlayers();
  std::multimap<float, Player*> tmpResult;

  float closestDistance = 10000;
  Player *closestPlayer = players.at(0);

  //printf("total players: %i\n", players.size());

  for (unsigned int i = 0; i < players.size(); i++) {
    if (players.at(i)->IsActive()) {
      float distance = (players.at(i)->GetPosition() - position).GetLength();
      if ((!onlyAIControlled) ||
          ( onlyAIControlled && !team->IsHumanControlled(players.at(i)->GetID()))) {
        tmpResult.insert(std::pair<float, Player*>(distance, players.at(i)));
      }
    }
  }

  //printf("tmp players: %i\n", tmpResult.size());

  std::map<float, Player*>::iterator iter = tmpResult.begin();
  for (unsigned int i = 0; i < playerCount && iter != tmpResult.end(); i++) {
    result.push_back(iter->second);
    iter++;
  }

  //printf("result: %i\n", result.size());

}

Player *AI_GetBestSwitchTargetPlayer(Match *match, Team *team, const Vector3 &desiredMovement) {

  // find most interesting position on pitch

  Vector3 actionPosition = match->GetDesignatedPossessionPlayer()->GetPosition() * 0.5f +
                           match->GetBall()->Predict(100).Get2D() * 0.5f;

  Vector3 defensePosition = (actionPosition * Vector3(1.0f, 0.8f, 0.0f)) + Vector3( team->GetSide() * 4.0f, 0.0f, 0.0f);
  Vector3 offensePosition = (actionPosition * Vector3(1.0f, 0.8f, 0.0f)) + Vector3(-team->GetSide() * 8.0f, 0.0f, 0.0f);

  // experiment: also take team possession player into account, try to pick one near
  defensePosition = defensePosition * 0.8f + team->GetDesignatedTeamPossessionPlayer()->GetPosition() * 0.2f;

  float offenseBias = team->GetFadingTeamPossessionAmount() - 0.5f;
  offenseBias = offenseBias * 0.2f + clamp(team->GetTeamPossessionAmount() - 0.5f, 0.0f, 1.0f) * 0.8f; // more direct, more urgent
  offenseBias = clamp((offenseBias - 0.5f) * 2.0 + 0.5f, 0.0f, 1.0f); // make more binary
  offenseBias =
      clamp(std::pow(offenseBias, 1.5f), 0.0f, 1.0f);  // tend towards defensive

  Vector3 resultingPosition = defensePosition * (1.0f - offenseBias) + offensePosition * offenseBias;

  if (team->GetID() == 0) {
    if (Verbose()) printf("SWITCH: offenseBias: %f\n", offenseBias);
  //   actionPosition.Print();
  //   defensePosition.Print();
  //   offensePosition.Print();
  //   resultingPosition.Print();
  }
  assert(offenseBias >= 0.0f && offenseBias <= 1.0f);

  // get sorted list of closest players

  std::vector<Player*> teamPlayers;
  AI_GetClosestPlayers(team, resultingPosition, true, teamPlayers, 8);
  std::vector<Player*>::iterator iter = teamPlayers.begin();
  while (iter != teamPlayers.end()) {
    if ((*iter)->GetFormationEntry().role == e_PlayerRole_GK) {
      iter = teamPlayers.erase(iter);
    } else {
      iter++;
    }
  }

  if (teamPlayers.size() == 0) return 0;


  // in case of defending, we ideally need someone who is closer to our goal than the opponent

  int bestPlayerIndex = 0; // closest player - sorted first in teamplayers array
  float tooLateDistance = 1.0f;

  Player *designated = match->GetDesignatedPossessionPlayer();
  if (designated->GetTeamID() != team->GetID()) {
    Player *opp = designated;
    Vector3 goalPos = Vector3(team->GetSide() * pitchHalfW, 0.0f, 0.0f);
    float oppGoalDist = (goalPos - (opp->GetPosition() + opp->GetMovement() * 0.5f)).GetLength();
    for (unsigned int i = 0; i < teamPlayers.size(); i++) {
      float mateGoalDist = (goalPos - (teamPlayers.at(i)->GetPosition() + teamPlayers.at(i)->GetMovement() * 0.5f)).GetLength();
      if (mateGoalDist < oppGoalDist + tooLateDistance + clamp(oppGoalDist * 0.1f, 0.0f, 3.0f)) { // doesn't matter much when opp is still far away from goal
        bestPlayerIndex = i;
        break;
      }
    }
  }

  return teamPlayers.at(bestPlayerIndex);
}

void AI_GetAutoPass(e_FunctionType passType, const Vector3 &vector, Vector3 &resultingDirection, float &resultingPower) {
  float heightOffset = 0.11f;
  float powerFactor = 1.8f;//1.6f
  float distanceExp = 1.4f;
  if (passType == e_FunctionType_HighPass) {
    heightOffset = 0.45f - NormalizedClamp(vector.GetLength(), 0.0f, 60.0f) * 0.15f;// 0.37f;
    powerFactor = 1.15f;//1.75
    distanceExp = 1.4f;//1.6
  }
  resultingDirection = (vector.GetNormalized(0) + Vector3(0, 0, heightOffset)).GetNormalized(0);
  resultingPower =
      std::pow(NormalizedClamp(vector.GetLength(), 0.0f, 60.0f), distanceExp) *
      powerFactor;
}

void AI_GetPass(Player *player, e_FunctionType passType, const Vector3 &inputDirection, float inputPower, float autoDirectionBias, float autoPowerBias, Vector3 &resultingDirection, float &resultingPower, Player *&targetPlayer, Player *forcedTargetPlayer) {

  // cheat for digital input

  bool fullAutoDirection = false;
  bool fullAutoPower = false;
  if (player->GetExternalController()) {
    if (static_cast<HumanController*>(player->GetExternalController())->GetHIDevice()->GetDeviceType() == e_HIDeviceType_Keyboard) {
      fullAutoDirection = true;
    }
  }

  float adaptedAutoDirectionBias = autoDirectionBias;
  float adaptedAutoPowerBias = autoPowerBias;

  assert(forcedTargetPlayer != player);


  // find out what player we intend to play to

  Vector playerPos = player->GetPosition() + player->GetMovement().GetNormalized(0) * 0.2f; // + ffo. don't need movement for future stuff, since getpass is recalled at moment of passing, for refinement
  if (player->TouchAnim()) {
    playerPos = player->GetTouchPos().Get2D();
  }
  Vector3 manualTarget = playerPos + inputDirection * clamp(inputPower * 60.0f, 1.0f, 100.0f);
  //if (player->GetDebug()) SetGreenDebugPilon(manualTarget);

  std::vector<Player*> players;
  player->GetTeam()->GetActivePlayers(players);

  if (players.size() < 2) {
    resultingDirection = player->GetDirectionVec();
    resultingPower = 1.0f;
    targetPlayer = player;
    fullAutoDirection = false;
    fullAutoPower = false;
    adaptedAutoDirectionBias = 0.0f;
    adaptedAutoPowerBias = 0.0f;
  }

  Player *bestTargetPlayer = player;
  Vector3 autoTarget = manualTarget;

  if (forcedTargetPlayer) {

    bestTargetPlayer = forcedTargetPlayer;

    float passDuration = 0.3f + (forcedTargetPlayer->GetPosition() - playerPos).GetLength() * 0.05f; // educated guess
    passDuration = std::pow(clamp(passDuration, 0.0f, 1.0f), 0.7f) *
                   0.7f;  // after this time, the player is supposed to have
                          // been able to stop

    autoTarget = forcedTargetPlayer->GetPosition() +
                 forcedTargetPlayer->GetMovement() * passDuration; // correct for pass duration

  } else {

    float bestRating = 10000;
    autoTarget = playerPos;

    for (int i = 0; i < (signed int)players.size(); i++) {
      if (players.at(i) != player/* && players.at(i)->IsActive()*/) {

        float passDuration = 0.3f + (players.at(i)->GetPosition() - playerPos).GetLength() * 0.05f; // educated guess
        passDuration = std::pow(clamp(passDuration, 0.0f, 1.0f), 0.7f) *
                       0.7f;  // after this time, the player is supposed to have
                              // been able to stop

        Vector3 targetPos = players.at(i)->GetPosition() +
                            players.at(i)->GetMovement() * passDuration; // correct for pass duration
        // rate
        float distanceRating =
            std::pow(NormalizedClamp((targetPos - manualTarget).GetLength(),
                                     0.0f, 70.0f),
                     0.8f) *
            0.8f;  // pow() this so small differences matter more - from some
                   // point on, it just doesn't really matter that much anymore
        float angleRating = fabs((targetPos - playerPos).GetNormalized(0).GetAngle2D(inputDirection) / (1.0f * pi)) * 1.0f;
        if (distanceRating + angleRating < bestRating) {
          bestRating = distanceRating + angleRating;
          bestTargetPlayer = players.at(i);
          autoTarget = targetPos;
        }
      }
    }

  }

  targetPlayer = bestTargetPlayer;
  assert(targetPlayer);
  //printf("target: %i\n", (int)targetPlayer);
  //if (player->GetDebug())
  //SetGreenDebugPilon(targetPlayer->GetPosition());
  //if (player->GetDebug())
  //SetYellowDebugPilon(manualTarget);

  Vector3 autoTargetRel = autoTarget - playerPos;
  Vector3 manualTargetRel = manualTarget - playerPos;

  if (forcedTargetPlayer || (fullAutoDirection && fullAutoPower)) {
    adaptedAutoDirectionBias = 1.0;
    adaptedAutoPowerBias = 1.0;
  } else {
    // only help when at least somewhat close to target
    float maxAllowedDistance = 50.0f;
    float distanceFactor =
        1.0 - std::pow(clamp((autoTargetRel - manualTargetRel).GetLength() /
                                 maxAllowedDistance,
                             0.0f, 1.0f),
                       1.5f);
    // extra help with power on close targets (because people can only physicaly press the pass button so short)
    float proximityBonus =
        std::pow(1.0f - NormalizedClamp((playerPos - autoTarget).GetLength(),
                                        0.0f, 12.0f),
                 0.5f);

    if (fullAutoDirection) {
      adaptedAutoDirectionBias = 1.0f;
    } else {
      adaptedAutoDirectionBias *= distanceFactor;
      adaptedAutoDirectionBias =
          std::pow(adaptedAutoDirectionBias, 1.0f - proximityBonus * 0.9f);
    }
    if (fullAutoPower) {
      adaptedAutoPowerBias = 1.0f;
    } else {
      adaptedAutoPowerBias *= distanceFactor;
      adaptedAutoPowerBias = clamp(adaptedAutoPowerBias * (1.0 + proximityBonus), 0.0f, 1.0f);
    }
    //printf("adaptedAutoPowerBias: %f\n", adaptedAutoPowerBias);
  }

  Vector3 offset;
  if (passType == e_FunctionType_LongPass) {
    float targetDistance = autoTargetRel.GetLength()   * adaptedAutoDirectionBias +
                           manualTargetRel.GetLength() * (1.0f - adaptedAutoDirectionBias);
    offset = Vector3(-player->GetTeam()->GetSide() * targetDistance * 0.2f, 0, 0);
    autoTargetRel += offset;
    manualTargetRel += offset;
  }
  Vector3 resultingTargetRel = (autoTargetRel.GetNormalized(0) * adaptedAutoDirectionBias + manualTargetRel.GetNormalized(0) * (1.0f - adaptedAutoDirectionBias)).GetNormalized(manualTarget);
  resultingTargetRel *= float(autoTargetRel.GetLength() * adaptedAutoPowerBias + manualTargetRel.GetLength() * (1.0f - adaptedAutoPowerBias));
  //if (player->GetDebug()) SetRedDebugPilon(playerPos + resultingTargetRel);
  //SetGreenDebugPilon(playerPos);

  AI_GetAutoPass(passType, resultingTargetRel, resultingDirection, resultingPower);
}

Vector3 AI_GetShotDirection(Player *player, const Vector3 &inputDirection, float autoDirectionBias) {

  Vector3 manualDirection = inputDirection;

  Vector3 goalPos = Vector3(player->GetTeam()->GetSide() * -pitchHalfW, 0, 0);
  Vector3 toGoal = (goalPos - (player->GetPosition() + player->GetMovement() * 0.12f)).GetNormalized(0);
  // if inputDirection ~== toGoal, it is considered as aiming 'through the middle'. so, get the deviation from inputDirection to toGoal, and make 90 degrees the maximum
  radian relAngle = toGoal.GetAngle2D(inputDirection);
  float sideFactor = clamp((relAngle / pi) / 0.5f, -1.0f, 1.0f);
  // more attenuation towards the sides
  sideFactor = std::pow(fabs(sideFactor), 0.7f) * signSide(sideFactor);

  goalPos.coords[1] = sideFactor * goalHalfWidth * 0.9f * player->GetTeam()->GetSide();
  Vector3 autoDirection = (goalPos - (player->GetPosition() + player->GetMovement() * 0.12f)).GetNormalized(0);

  //SetYellowDebugPilon(goalPos);

  return (manualDirection * (1.0f - autoDirectionBias) + autoDirection * autoDirectionBias).GetNormalized(inputDirection);
}

// get the offensiveness of a role
float AI_GetMindSet(e_PlayerRole role) {
  float mindSet = 0.5;

  if (role == e_PlayerRole_GK) mindSet = 0.0;

  if (role == e_PlayerRole_CB) mindSet = 0.0;

  if (role == e_PlayerRole_LB ||
      role == e_PlayerRole_RB) mindSet = 0.25;

  if (role == e_PlayerRole_DM) mindSet = 0.25;

  if (role == e_PlayerRole_LM ||
      role == e_PlayerRole_CM ||
      role == e_PlayerRole_RM) mindSet = 0.5;

  if (role == e_PlayerRole_AM) mindSet = 0.75;

  if (role == e_PlayerRole_CF) mindSet = 1.0;

  return mindSet;
}
