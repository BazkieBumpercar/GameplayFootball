// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "animcollection.hpp"

#include "utils/directoryparser.hpp"

#include "utils/animationextensions/footballanimationextension.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "utils/objectloader.hpp"
#include "scene/objectfactory.hpp"

#include "humanoid_utils.hpp"

#include "humanoid.hpp"

#include "main.hpp"

void FillNodeMap(boost::intrusive_ptr<Node> targetNode, std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap) {
  //printf("%s\n", targetNode->GetName().c_str());
  nodeMap.insert(std::pair < std::string, boost::intrusive_ptr<Node> >(targetNode->GetName(), targetNode));

  std::vector < boost::intrusive_ptr<Node> > gatherNodes;
  targetNode->GetNodes(gatherNodes);
  for (unsigned int i = 0; i < gatherNodes.size(); i++) {
    FillNodeMap(gatherNodes.at(i), nodeMap);
  }

}

AnimCollection::AnimCollection(boost::shared_ptr<Scene3D> scene3D) : scene3D(scene3D) {
  defString[0] = "";
  defString[1] = "outgoing_special_state";
  defString[2] = "incoming_special_state";
  defString[3] = "specialvar1";
  defString[4] = "specialvar2";
  defString[5] = "type";
  defString[6] = "trap";
  defString[7] = "deflect";
  defString[8] = "interfere";
  defString[9] = "trip";
  defString[10] = "shortpass";
  defString[11] = "longpass";
  defString[12] = "shot";
  defString[13] = "sliding";
  defString[14] = "movement";
  defString[15] = "special";
  defString[16] = "ballcontrol";
  defString[17] = "highpass";
  defString[18] = "catch";
  defString[19] = "outgoing_retain_state";
  defString[20] = "incoming_retain_state";

  maxIncomingBallDirectionDeviation = 0.25f * pi;
  maxOutgoingBallDirectionDeviation = 0.25f * pi;


  // quadrants denote the quantized outgoing movement - there are only certain possibilities:
  // idle - dribble - walk - run, in combination with angles: 0 - 20 - 45 - 90 - 135 - 180 (and their negatives, where applicable)
  // so every anim falls into one of these quadrants.

  // idle
  Quadrant quadrant;
  quadrant.id = 0;
  quadrant.velocity = e_Velocity_Idle;
  quadrant.angle = 0;
  quadrant.position = Vector3(0, 0, 0);
  quadrants.push_back(quadrant);

  int id = 1;
  for (int velocityID = 1; velocityID < 4; velocityID++) {

    e_Velocity velocity;
    if (velocityID == 1) velocity = e_Velocity_Dribble;
    else if (velocityID == 2) velocity = e_Velocity_Walk;
    else if (velocityID == 3) velocity = e_Velocity_Sprint;

    for (int angleID = 0; angleID < 11; angleID++) {

      radian angle;
      if      (angleID == 0)  angle = pi / 180.0f *    0.0f;
      else if (angleID == 1)  angle = pi / 180.0f *   20.0f;
      else if (angleID == 2)  angle = pi / 180.0f *   45.0f;
      else if (angleID == 3)  angle = pi / 180.0f *   90.0f;
      else if (angleID == 4)  angle = pi / 180.0f *  135.0f;
      else if (angleID == 5)  angle = pi / 180.0f *  179.0f;
      else if (angleID == 6)  angle = pi / 180.0f *  -20.0f;
      else if (angleID == 7)  angle = pi / 180.0f *  -45.0f;
      else if (angleID == 8)  angle = pi / 180.0f *  -90.0f;
      else if (angleID == 9)  angle = pi / 180.0f * -135.0f;
      //else if (angleID == 10) angle = pi / 180.0f * -179.0f;

      Quadrant quadrant;
      quadrant.id = id;
      quadrant.velocity = velocity;
      quadrant.angle = angle;
      quadrant.position = Vector3(0, -1, 0).GetRotated2D(angle) * EnumToFloatVelocity(velocity);
      quadrants.push_back(quadrant);

      id++;
    }
  }

}

AnimCollection::~AnimCollection() {
  if (Verbose()) printf("exiting animcollection.. ");
  Clear();
  if (Verbose()) printf("done\n");
}

void AnimCollection::Clear() {
  std::vector < Animation* >::iterator animIter = animations.begin();
  while (animIter != animations.end()) {
    delete *animIter;
    animIter++;
  }
  animations.clear();
}

radian GetAngle(int directionID) {
  radian angle = 0.0f;
  switch (directionID) {
    case 0:
      angle = 0.0f;
      break;
    case 1:
      angle = 0.25f * pi;
      break;
    case 2:
      angle = -0.25f * pi;
      break;
    case 3:
      angle = 0.50f * pi;
      break;
    case 4:
      angle = -0.50f * pi;
      break;
    case 5:
      angle = 0.75f * pi;
      break;
    case 6:
      angle = -0.75f * pi;
      break;
    case 7:
      angle = 0.99f * pi;
      break;
    case 8:
      angle = -0.99f * pi;
      break;
    default:
      break;

  }
  return angle;
}

void GenerateAutoAnims(const std::vector<Animation*> &templates, std::vector<Animation*> &autoAnims) {

  const float leanAmount = 0.001f;
  const int frameCount = 25;
  const float margin = 0.01f;

  for (unsigned int t1 = 0; t1 < templates.size(); t1++) {
    for (unsigned int t2 = 0; t2 < templates.size(); t2++) {

      Animation *anim1 = templates.at(t1);
      Animation *anim2 = templates.at(t2);

      for (unsigned int direction = 0; direction < 9; direction++) {

        radian angle = GetAngle(direction);
        float incomingVelocityT1 = anim1->GetIncomingVelocity();
        float outgoingVelocityT2 = anim2->GetOutgoingVelocity();
        radian incomingBodyAngleT1 = anim1->GetIncomingBodyAngle();
        radian outgoingBodyAngleT2 = anim2->GetOutgoingBodyAngle();

        bool legalAnim = true;

        int incomingVeloID = GetVelocityID(FloatToEnumVelocity(incomingVelocityT1));
        int outgoingVeloID = GetVelocityID(FloatToEnumVelocity(outgoingVelocityT2));
        float averageVeloFactor = NormalizedClamp(incomingVeloID + outgoingVeloID, 0, 6);

        // max acceleration
        int veloIDDiff = outgoingVeloID - incomingVeloID;
        if (veloIDDiff > 1) legalAnim = false;

        // max deceleration
        // use dot product to make sure "running 180 degrees" isn't considered "keeping the same velocity" and therefore legal.
        float dot = Vector3(0, -1, 0).GetDotProduct(Vector3(0, -1, 0).GetRotated2D(angle)); // todo: this is optimizable (simple angle -> dot)
        int veloIDDiff_dotted = round(outgoingVeloID * dot - incomingVeloID);
        if (veloIDDiff_dotted < -3) legalAnim = false;

        if (incomingVeloID == 3 && outgoingVeloID == 1 && (fabs(outgoingBodyAngleT2) > 0.25f * pi + margin || fabs(angle) > 0.25f * pi + margin)) legalAnim = false; // no sprint to dribble with backwards body angle
        //if (incomingVeloID == 3 && outgoingVeloID == 0 && fabs(angle) > 0.25f * pi) legalAnim = false; // no sprint to idle with big angle

        // experimental block
        if (incomingVeloID > 0 && outgoingVeloID > 0 && fabs(angle) > 0.50f * pi + margin) legalAnim = false;
        //if (incomingVeloID > 0 && outgoingVeloID == 0 && fabs(outgoingBodyAngleT2) > 0.50f * pi + margin) legalAnim = false;
        // if (incomingVeloID == 3 && outgoingVeloID == 2 && (fabs(outgoingBodyAngleT2) > 0.3f * pi || fabs(angle) > 0.25f * pi)) legalAnim = false;
        //zzif (incomingVeloID == 3 && outgoingVeloID == 0) legalAnim = false;
        //if (incomingVeloID == 3 && outgoingVeloID == 2 && fabs(angle) > 0.25f * pi) legalAnim = false;
        //if (fabs(outgoingBodyAngleT2 - incomingBodyAngleT1 + angle) > 0.5f * pi) legalAnim = false;

        //if (incomingVeloID + outgoingVeloID > 5 && fabs(angle) > 0.25f * pi) legalAnim = false; // sprint -> sprint
        //if (incomingVeloID + outgoingVeloID > 4 && fabs(angle) > 0.5f * pi) legalAnim = false; // walk -> sprint && sprint -> walk
        if (incomingVeloID + outgoingVeloID > 5 && fabs(angle) > 0.25f * pi + margin) legalAnim = false; // sprint -> sprint
        if (incomingVeloID + outgoingVeloID > 4 && fabs(angle) > 0.25f * pi + margin) legalAnim = false; // walk -> sprint && sprint -> walk
        //if (incomingVeloID + outgoingVeloID > 3 && fabs(angle) > 0.50f * pi + margin) legalAnim = false; // walk -> walk && sprint -> dribble && dribble -> sprint
        //if (incomingVeloID + outgoingVeloID > 2 && fabs(angle) > 0.75f * pi + margin) legalAnim = false; // dribble -> walk && walk -> dribble
        //if (incomingVeloID > 0 && outgoingVeloID > 0 && fabs(angle) > 0.5f * pi + margin) legalAnim = false; // dribble -> walk && walk -> dribble

        radian bodyAngleDelta = ModulateIntoRange(-pi, pi, outgoingBodyAngleT2 - incomingBodyAngleT1);
        if (fabs(angle + bodyAngleDelta) > 1.0f * pi + margin) legalAnim = false; // don't rotate over 180 degrees (we've got an anim for that the shorter away around anyway)
        //if (fabs(angle + bodyAngleDelta) > 0.5f * pi + margin) legalAnim = false; // just don't turn too fast

        //if (fabs(bodyAngleDelta) > 0.9f * pi) legalAnim = false; // body rotation limit: if we allow 180 degrees, the slerp doesn't know whether it should go CW or CCW (seems to be fixed by the ModulateIntoRange above - why?)
        //if (fabs(veloIDDiff) > 2 && fabs(angle + bodyAngleDelta) > 0.25f * pi) legalAnim = false; // don't brake AND turn all too much
        //|__[R]__ autogen [v2 b-135] => [v2 b45 a-179]_mirror
        //if (incomingVeloID + outgoingVeloID > 3 && fabs(angle) > 0.50f * pi) legalAnim = false;

/* too uncontrollable
        Vector3 movementChangeVec = (Vector3(0, -1, 0) * incomingVelocityT1) - (Vector3(0, -1, 0).GetRotated2D(angle) * outgoingVelocityT2);
        Vector3 bodyDirChangeVec = (Vector3(0, -1, 0).GetRotated2D(incomingBodyAngleT1)) - (Vector3(0, -1, 0).GetRotated2D(angle + outgoingBodyAngleT2));
        Vector3 totalChangeVec = movementChangeVec + bodyDirChangeVec * 1.0f;
        if (totalChangeVec.GetLength() > sprintVelocity) legalAnim = false;
*/

        float animSpeedFactor = 1.0f;

        if (legalAnim == true) {

          Animation *gen = new Animation(*templates.at(t1));
          gen->SetName("autogen [v" + int_to_str(GetVelocityID(FloatToEnumVelocity(incomingVelocityT1))) + " b" + int_to_str(incomingBodyAngleT1 / pi * 180) + "] => [v" + int_to_str(GetVelocityID(FloatToEnumVelocity(outgoingVelocityT2))) + " b" + int_to_str(outgoingBodyAngleT2 / pi * 180) + " a" + int_to_str(angle / pi * 180) + "]");
          gen->SetVariable("priority", "1");

          std::vector<NodeAnimation*> &nodeAnimsT1 = anim1->GetNodeAnimations();
          std::vector<NodeAnimation*> &nodeAnimsT2 = anim2->GetNodeAnimations();

          for (unsigned int n = 0; n < nodeAnimsT1.size(); n++) {

            gen->GetNodeAnimations().at(n)->animation.clear();

            // NodeAnimation *nodeAnimT1 = nodeAnimsT1.at(n);
            // NodeAnimation *nodeAnimT2 = nodeAnimsT2.at(n);
            std::map<int, KeyFrame> &animationT1 = nodeAnimsT1.at(n)->animation;
            std::map<int, KeyFrame> &animationT2 = nodeAnimsT2.at(n)->animation;

            Vector3 cumulativePosition;
            int prevFrame = 0;
            Vector3 outgoingMovement = anim2->GetOutgoingMovement().GetRotated2D(angle);

            Vector3 movementChangeMPS = (outgoingMovement - anim1->GetIncomingMovement()) * (100.0f / frameCount);


            // COLLECT KEYFRAMES FOR THIS NODEANIM

            std::list<int> keyFrames; // frames at which one of the two anims has a keyframe
            // first, add all keyframes, even if duplicate
            std::map<int, KeyFrame>::const_iterator animKeyIter = animationT1.begin();
            while (animKeyIter != animationT1.end()) { keyFrames.push_back(animKeyIter->first); animKeyIter++; }
            animKeyIter = animationT2.begin();
            while (animKeyIter != animationT2.end()) { keyFrames.push_back(animKeyIter->first); animKeyIter++; }
            if (n == 0) {
              // make sure there's 2 position keyframes close to each other at the start and at the end, so we will have the right ingoing and outgoing velocities
              keyFrames.push_back(1);
              keyFrames.push_back(23);
              //for (int i = 2; i < frameCount - 2; i += 2) keyFrames.push_back(i); // smoother
              for (int i = 2; i < frameCount - 2; i += 4) keyFrames.push_back(i); // smoother
            }
            keyFrames.sort();
            keyFrames.unique(); // delete duplicates


            // ITERATE AND INTERPOLATE KEYFRAMES

            std::list<int>::iterator keyIter = keyFrames.begin();
            while (keyIter != keyFrames.end()) {
              int frame = *keyIter;
              float targetFrame = frame * (1.0f / animSpeedFactor);

              Quaternion orientationT1 = QUATERNION_IDENTITY;
              Quaternion orientationT2 = QUATERNION_IDENTITY;
              Vector3 positionT1, positionT2;
              bool getOrientation = (n == 0) ? false : true;
              bool getPosition = (n == 0) ? true : false;
              templates.at(t1)->GetInterpolatedValues(animationT1, frame, orientationT1, positionT1, getOrientation, getPosition);
              templates.at(t2)->GetInterpolatedValues(animationT2, frame, orientationT2, positionT2, getOrientation, getPosition);

              //float bias = frame / ((float)frameCount - 1);
              float origBias = clamp(frame - 1.0f, 0.0f, frameCount - 3.0f) / ((float)frameCount - 3.0f); // version that ignores first and last 2 frames, so incoming/outgoing velo/angle will be correct
              // bias = pow(bias, 0.7f + pow(averageVeloFactor, 2.0) * 0.2f); // move a bit earlier
              // bias = curve(bias, 1.0f - pow(averageVeloFactor, 2.0) * 0.8f); // concentrate change in the middle of the anim
              float bias = origBias;
              //bias = pow(bias, 0.7f); // move a bit earlier
              //bias = pow(bias, 0.2f + 0.6f * averageVeloFactor);
              //if (frame == frameCount - 2) printf("before: %f, ", bias);
              bias = pow(bias, 1.0f * (0.3f + 0.4f * averageVeloFactor + 0.3f * NormalizedClamp(movementChangeMPS.GetLength(), 0.0f, 20.0f)));
              //if (frame == frameCount - 2) printf("middle: %f, ", bias);
              bias = curve(bias, 0.7f); //0.3f// concentrate change in the middle of the anim
              //if (frame == frameCount - 2) printf("after: %f\n", bias);
              Quaternion orientation = orientationT1.GetSlerped(bias, orientationT2);

              if (n == 1) { // body
                // body orientation
                Quaternion angleQuat; angleQuat.SetAngleAxis(angle * pow((bias * 0.7f + origBias * 0.3f), 1.0f), Vector3(0, 0, 1));
                orientation = angleQuat * orientation;

                // leaning
                //movementChangeMPS *= (incomingVeloID + outgoingVeloID) / 6.0f;
                movementChangeMPS *= 0.5f + 0.5f * ((anim1->GetIncomingMovement() * (1.0f - bias) + outgoingMovement * bias).GetLength() / sprintVelocity); // high velo = more leaning into the wind and such :p
                Quaternion leanQuat; leanQuat.SetAngleAxis(movementChangeMPS.GetLength() * leanAmount * (0.5f + 0.5f * sin(origBias * pi)), Vector3(0, 1, 0).GetRotated2D(movementChangeMPS.GetNormalized(0).GetAngle2D()));
                orientation = leanQuat * orientation;
              }

              float height = 0.0f;
              if (n == 0) { // player
                //if (outgoingVeloID == 0 && incomingVeloID != 0) printf("frame: %i, bias: %f\n", frame, bias);
                cumulativePosition += anim1->GetIncomingMovement() * ((frame - prevFrame) * 0.01f) * (1.0f - bias) +
                                      outgoingMovement * ((frame - prevFrame) * 0.01f) * (bias);
                height = positionT1.coords[2] * (1.0f - bias) + positionT2.coords[2] * bias;
              }

              gen->SetKeyFrame(nodeAnimsT1.at(n)->nodeName, int(floor(targetFrame)), orientation, cumulativePosition * (1.0f / animSpeedFactor) + Vector3(0, 0, height));

              prevFrame = frame;
              keyIter++;
            }
          }

          gen->DirtyCache();

          assert(gen->GetIncomingVelocity() == anim1->GetIncomingVelocity());
          assert(gen->GetOutgoingVelocity() == anim2->GetOutgoingVelocity());

          // enable this to save all the autogenerated anims to files, so that we can use them as a base for manual anims. don't forget to disable again after usage ;)
          //gen->Save("media/animations/debug_luxury/autogen v" + int_to_str(GetVelocityID(FloatToEnumVelocity(incomingVelocityT1))) + " b" + int_to_str(round(incomingBodyAngleT1 / pi * 180)) + " _to_ v" + int_to_str(GetVelocityID(FloatToEnumVelocity(outgoingVelocityT2))) + " b" + int_to_str(round(outgoingBodyAngleT2 / pi * 180)) + " a" + int_to_str(angle / pi * 180) + ".anim");

          autoAnims.push_back(gen);

        } // == legalAnim

      }
    }
  }

  Log(e_Notice, "AnimCollection", "GenerateAutoAnims", int_to_str(autoAnims.size()) + " autogenerated anims! huzzah!");
}

void AnimCollection::Load(boost::filesystem::path directory) {


  // load utility player to get things like foot position in the frames around the balltouch etc.

  Log(e_Notice, "AnimCollection", "Load", "Loading utility player");

  ObjectLoader loader;
  boost::intrusive_ptr<Node> playerNode;
  playerNode = loader.LoadObject(scene3D, "media/objects/players/player.object");
  playerNode->SetName("player");
  playerNode->SetLocalMode(e_LocalMode_Absolute);

  std::list < boost::intrusive_ptr<Object> > bodyParts;
  playerNode->GetObjects(e_ObjectType_Geometry, bodyParts, true);

  std::map < const std::string, boost::intrusive_ptr<Node> > nodeMap;
  FillNodeMap(playerNode, nodeMap);


  // base anim with default angles - all anims' joints will be inversely rotated by the joints in this anim. this way, the fullbody mesh doesn't need to have 0 degree angles

/*
  Animation *baseAnim = new Animation();
  baseAnim->Load("media/animations/base.anim.util");
*/


  // auto generated anims

  Log(e_Notice, "AnimCollection", "Load", "Parsing autogenerated animation template directory");

  DirectoryParser parser;
  std::vector<std::string> files;
  parser.Parse(directory / "/templates", "anim", files);

  Log(e_Notice, "AnimCollection", "Load", "Loading autogenerated animation templates");

  std::vector<Animation*> templates;
  for (unsigned int i = 0; i < files.size(); i++) {
    Animation *animTemplate = new Animation();
    animTemplate->Load(files.at(i));
    templates.push_back(animTemplate);
  }

  std::vector<Animation*> autoAnims;
  GenerateAutoAnims(templates, autoAnims);
  std::vector < Animation* >::iterator animIter = templates.begin();
  while (animIter != templates.end()) {
    delete *animIter;
    animIter++;
  }
  templates.clear();

  for (unsigned int i = 0; i < autoAnims.size(); i++) {
    Animation *animation = new Animation(*autoAnims.at(i));
    boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
    animation->AddExtension("football", extension);
    animation->Mirror();
    _PrepareAnim(animation, playerNode, bodyParts, nodeMap, false);

    animation = autoAnims.at(i);
    extension.reset(new FootballAnimationExtension(animation));
    animation->AddExtension("football", extension);
    _PrepareAnim(animation, playerNode, bodyParts, nodeMap, false);
  }


  // load all other animations

  Log(e_Notice, "AnimCollection", "Load", "Parsing animation directory");

  files.clear();
  parser.Parse(directory, "anim", files);

  Log(e_Notice, "AnimCollection", "Load", "Loading animations");

  bool omitLuxuryAnims = true;

  for (unsigned int i = 0; i < files.size(); i++) {

    //printf("%s\n", files.at(i).c_str());

    if ((omitLuxuryAnims && files.at(i).find("luxury") != std::string::npos) || files.at(i).find("templates") != std::string::npos) {
      //printf ("ignoring\n");

    } else {

      if (Verbose()) printf("%s\n", files.at(i).c_str());

      for (int mirror = 0; mirror < 2; mirror++) {
        Animation *animation = new Animation();
        boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
        animation->AddExtension("football", extension);
        animation->Load(files.at(i));
        if (mirror == 1) animation->Mirror();

        _PrepareAnim(animation, playerNode, bodyParts, nodeMap, false);

        /* disabled: too many side effects, should just make the most important of these manually

        // duplicate dribble anims with > 45 degree body directions (either in or out) and convert duplicate to walking speed
        // this is because of the decision to allow 135 degree body directions ('walking backward') on walking velocities.
        // more correct (to get proper leg movement for walking velocities) would be to create separate anims for these, but i'm feeling lazy
        if (animation->GetAnimType().compare("movement") == 0) {
          if (fabs(animation->GetIncomingBodyAngle()) > 0.5 * pi || fabs(animation->GetOutgoingBodyAngle()) > 0.5 * pi) {
            if (FloatToEnumVelocity(animation->GetIncomingVelocity()) == e_Velocity_Dribble || FloatToEnumVelocity(animation->GetOutgoingVelocity()) == e_Velocity_Dribble) {

              Animation *animation2 = new Animation();
              boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
              animation2->AddExtension("football", extension);
              animation2->Load(files.at(i), mirror == 0 ? false : true);

              _PrepareAnim(animation2, playerNode, bodyParts, nodeMap, true);

            }
          }
        }*/
      }

    }

  }

  //Log(e_Notice, "AnimCollection", "Load", "Deleting base anim");

  //delete baseAnim;

  Log(e_Notice, "AnimCollection", "Load", "Deleting player node template");

  playerNode->Exit();

  Log(e_Notice, "AnimCollection", "Load", "Ready");
}

const std::vector < Animation* > &AnimCollection::GetAnimations() const {
  return animations;
}


void AnimCollection::CrudeSelection(DataSet &dataSet, const CrudeSelectionQuery &query) {

  // makes a crude selection to later refine


  int animSize = animations.size();

  for (int i = 0; i < animSize; i++) {

    const std::string &animType = animations.at(i)->GetAnimType();

    bool selectAnim = true;


    // select by TYPE

    if (selectAnim) {
      if (query.byFunctionType == true) {
        if (_CheckFunctionType(animType, query.functionType) == false) selectAnim = false;
      }
    }

/*
    // select by FOOT

    if (selectAnim) {
      if (query.byFoot == true) {
        if (animations.at(i)->GetCurrentFoot() != query.foot && FloatToEnumVelocity(animations.at(i)->GetIncomingVelocity()) != e_Velocity_Idle) selectAnim = false;
      }
    }
*/


    // select by INCOMING VELOCITY

    if (selectAnim) {
      if (query.byIncomingVelocity == true) {

        e_Velocity animIncomingVelocity = FloatToEnumVelocity(animations.at(i)->GetIncomingVelocity());

        if (query.incomingVelocity_Strict == false) {

          selectAnim = true;
          if (query.incomingVelocity_NoDribbleToIdle) {
            if (animIncomingVelocity == e_Velocity_Idle && query.incomingVelocity == e_Velocity_Dribble) selectAnim = false;
          }
          if (animIncomingVelocity == e_Velocity_Idle && query.incomingVelocity == e_Velocity_Walk) selectAnim = false;
          if (animIncomingVelocity == e_Velocity_Idle && query.incomingVelocity == e_Velocity_Sprint) selectAnim = false;
          if (animIncomingVelocity == e_Velocity_Dribble && query.incomingVelocity == e_Velocity_Idle) selectAnim = false;
          if (animIncomingVelocity == e_Velocity_Walk && query.incomingVelocity == e_Velocity_Idle) selectAnim = false;
          if (animIncomingVelocity == e_Velocity_Sprint && query.incomingVelocity == e_Velocity_Idle) selectAnim = false;

          if (query.incomingVelocity_NoDribbleToSprint) {
            if (animIncomingVelocity == e_Velocity_Sprint && query.incomingVelocity == e_Velocity_Dribble) selectAnim = false;
          }

          if (query.incomingVelocity_ForceLinearity) {
            // disallow going from current -> slower/faster -> current; the complete section needs to be linear
            float animIncomingVelocityFloat = RangeVelocity(animations.at(i)->GetIncomingVelocity());
            float animOutgoingVelocityFloat = RangeVelocity(animations.at(i)->GetOutgoingVelocity());
            float queryVelocityFloat = EnumToFloatVelocity(query.incomingVelocity);

            // treat dribble and walk the same
            if (FloatToEnumVelocity(animIncomingVelocityFloat) == e_Velocity_Dribble) animIncomingVelocityFloat = walkVelocity;
            if (FloatToEnumVelocity(animOutgoingVelocityFloat) == e_Velocity_Dribble) animOutgoingVelocityFloat = walkVelocity;
            if (FloatToEnumVelocity(queryVelocityFloat) == e_Velocity_Dribble) queryVelocityFloat = walkVelocity;

            if (animIncomingVelocityFloat > std::max(queryVelocityFloat, animOutgoingVelocityFloat)) selectAnim = false;
            if (animIncomingVelocityFloat < std::min(queryVelocityFloat, animOutgoingVelocityFloat)) selectAnim = false;
          }

        } else {
          // strict
          if (animIncomingVelocity != query.incomingVelocity) selectAnim = false;
        }

        // test: disallow idle -> moving and other way around
        //e_Velocity animOutgoingVelocity = FloatToEnumVelocity(animations.at(i)->GetOutgoingVelocity());
        //if (animIncomingVelocity == e_Velocity_Idle && animOutgoingVelocity != e_Velocity_Idle) selectAnim = false;
        //if (animOutgoingVelocity == e_Velocity_Idle && animIncomingVelocity != e_Velocity_Idle) selectAnim = false;
      }
    }

    // test: disable all sprint anims
    // e_Velocity animIncomingVelocity = FloatToEnumVelocity(animations.at(i)->GetIncomingVelocity());
    // e_Velocity animOutgoingVelocity = FloatToEnumVelocity(animations.at(i)->GetOutgoingVelocity());
    // if (animIncomingVelocity == e_Velocity_Sprint || animOutgoingVelocity == e_Velocity_Sprint) selectAnim = false;


    // select by OUTGOING VELOCITY

    if (selectAnim) {
      if (query.byOutgoingVelocity == true) {
        if (FloatToEnumVelocity(animations.at(i)->GetOutgoingVelocity()) != query.outgoingVelocity) selectAnim = false;
      }
    }


    // CULL WRONG ROTATIONAL SIDE

    if (selectAnim) {
      if (query.bySide == true) {

        Vector3 animIncomingDirection = animations.at(i)->GetIncomingBodyDirection();

        // find out in what direction the anim rotates
        Vector3 animOutgoingDirection = animations.at(i)->GetOutgoingDirection().GetRotated2D(animations.at(i)->GetOutgoingBodyAngle());
        radian animTurnAngle = animOutgoingDirection.GetAngle2D(animIncomingDirection);

        // anim should not pass through opposite (180 deg) of desired look angle
        Vector3 fencedDirection = query.lookAtVecRel.GetRotated2D(pi);

        if (fabs(animTurnAngle) > 0.06f * pi) { // threshold
          e_Side animSide = (animTurnAngle > 0) ? e_Side_Left : e_Side_Right;

          radian animIncomingToFenceAngle = fencedDirection.GetAngle2D(animIncomingDirection);
          radian queryIncomingToFenceAngle = fencedDirection.GetAngle2D(query.incomingBodyDirection);
          radian fenceToOutgoingAngle = animOutgoingDirection.GetAngle2D(fencedDirection);

          e_Side animIncomingToFenceSide = (animIncomingToFenceAngle > 0) ? e_Side_Left : e_Side_Right;
          e_Side queryIncomingToFenceSide = (queryIncomingToFenceAngle > 0) ? e_Side_Left : e_Side_Right;
          e_Side fenceToAnimOutgoingSide = (fenceToOutgoingAngle > 0) ? e_Side_Left : e_Side_Right;

          // passes through fence! n000!
          if (animIncomingToFenceSide  == animSide && fenceToAnimOutgoingSide == animSide && fabs(animIncomingToFenceAngle + fenceToOutgoingAngle) < pi) selectAnim = false;
          if (queryIncomingToFenceSide == animSide && fenceToAnimOutgoingSide == animSide && fabs(queryIncomingToFenceSide + fenceToOutgoingAngle) < pi) selectAnim = false;
        }

      }
    }


/*
    // select by OUTGOING ANGLE

    // can't be used, wanting to go 180 deg might need a 0 deg anim first (deccelerate)

    if (selectAnim) {
      if (query.byOutgoingAngle == true) {
        if (fabs(animations.at(i)->GetOutgoingAngle() - query.outgoingAngle) > pi * 0.5) selectAnim = false;
      }
    }
*/

    // select by RETAIN BALL

    if (selectAnim) {
      if (query.byPickupBall == true) {
        if ((animations.at(i)->GetVariable("outgoing_retain_state") == "" && query.pickupBall == true) ||
            (animations.at(i)->GetVariable("outgoing_retain_state") != "" && query.pickupBall == false)) {
          selectAnim = false;
        }
      }
    }


    // select LAST DITCH ANIMS

    if (selectAnim) {
      if (query.allowLastDitchAnims == false) {
        if (animations.at(i)->GetVariable("lastditch") == "true") {
          selectAnim = false;
        }
      }
    }


    // select by INCOMING BODY ANGLE

    if (selectAnim) {

      if (query.byIncomingBodyDirection == true && !(query.byIncomingVelocity == true && query.incomingVelocity == e_Velocity_Idle)) {

        radian marginRadians = 0.06f * pi; // anims can deviate a few degrees from the desired (quantized) directions

        if (FloatToEnumVelocity(animations.at(i)->GetIncomingVelocity()) != e_Velocity_Idle) {

          Vector3 incomingBodyDir = animations.at(i)->GetIncomingBodyDirection();

/* this is implicitly happening already because of the section after this one anyway, so disable *todo: is it?
          //if (selectAnim) {
            if (query.incomingBodyDirection_Strict == true) { // == non-movement, atm
              // strict
              //if (fabs(incomingBodyDir.GetAngle2D(Vector3(0, -1, 0))) > fabs(query.incomingBodyDirection.GetAngle2D(Vector3(0, -1, 0))) + marginRadians) selectAnim = false;
              // allow 45
              //if (fabs(incomingBodyDir.GetAngle2D(Vector3(0, -1, 0))) > fabs(query.incomingBodyDirection.GetAngle2D(Vector3(0, -1, 0))) + 0.25f * pi + marginRadians) selectAnim = false;

              //} else {
              // if (fabs(query.incomingBodyDirection.GetAngle2D(animations.at(i)->GetIncomingBodyDirection())) > marginRadians) selectAnim = false;
              //}
            }
          //}
*/
          if (selectAnim) {
            // disallow larger than x radians diff
            //if (fabs(query.incomingBodyDirection.GetAngle2D(animations.at(i)->GetIncomingBodyDirection())) > marginRadians) selectAnim = false;
            // disallow larger incoming than current
            if (fabs(FixAngle(animations.at(i)->GetIncomingBodyDirection().GetAngle2D())) > fabs(FixAngle(query.incomingBodyDirection.GetAngle2D())) + marginRadians) selectAnim = false;
          }

          if (selectAnim) {

            // absolute outgoing body dir is body dir + outgoing dir
            Vector3 outgoingBodyDir = Vector3(0, -1, 0).GetRotated2D(animations.at(i)->GetOutgoingBodyAngle() + animations.at(i)->GetOutgoingAngle());

            // disallow > ~135 degrees (not really needed, i guess)
            //if (fabs(animations.at(i)->GetIncomingBodyDirection().GetAngle2D(query.incomingBodyDirection)) > 0.75f * pi + marginRadians) selectAnim = false;
            //if (fabs(animations.at(i)->GetIncomingBodyDirection().GetAngle2D(query.incomingBodyDirection)) > 0.5f * pi + marginRadians) selectAnim = false;

            // disallow > ~90 degrees larger incoming than current
            //if (fabs(FixAngle(animations.at(i)->GetIncomingBodyDirection().GetAngle2D())) > fabs(FixAngle(query.incomingBodyDirection.GetAngle2D())) + 0.5f * pi + marginRadians) selectAnim = false;
            // disallow > ~90 degrees different incoming than current
            //if (fabs(fabs(FixAngle(animations.at(i)->GetIncomingBodyDirection().GetAngle2D())) - fabs(FixAngle(query.incomingBodyDirection.GetAngle2D()))) > 0.5f * pi + marginRadians) selectAnim = false;
            //if (fabs(animations.at(i)->GetIncomingBodyDirection().GetAngle2D(Vector3(0, -1, 0)) - query.incomingBodyDirection.GetAngle2D(Vector3(0, -1, 0))) > 0.5f * pi + marginRadians) selectAnim = false;
            // this version is not just moar beautiful, but also allows for -135 to 135 deg and vice versa
            if (query.incomingBodyDirection_Strict == true) {
              if (fabs(incomingBodyDir.GetAngle2D(query.incomingBodyDirection)) > marginRadians) selectAnim = false;
            } else {
              if (fabs(incomingBodyDir.GetAngle2D(query.incomingBodyDirection)) > 0.5f * pi + marginRadians) selectAnim = false;
            }

            // disallow > ~135 degrees between query incoming and abs outgoing body dir (kills 180 deg anims!)
            //if (fabs(outgoingBodyDir.GetAngle2D(query.incomingBodyDirection)) > 0.75f * pi + marginRadians) selectAnim = false;

            if (query.incomingBodyDirection_ForceLinearity) {
              // if anim incoming body dir == between (including) query incoming and anim outgoing dir, then this anim is legal (or rather: won't look idiotic)
              // how do we check this?
              // 1. if we look at the smallest angles between (anim incoming -> query incoming) and (anim incoming -> anim outgoing), one has to be positive, the other negative.
              // 2. the (absolute) angles added up have to be < pi radians. else, we could be on the 'other side' of the 'virtual half circle' and still have the former condition met

              radian shortestAngle1 = incomingBodyDir.GetAngle2D(outgoingBodyDir);
              radian shortestAngle2 = incomingBodyDir.GetAngle2D(query.incomingBodyDirection);
              if ((shortestAngle1 >  marginRadians && shortestAngle2 >  marginRadians) ||
                  (shortestAngle1 < -marginRadians && shortestAngle2 < -marginRadians)) {
                selectAnim = false;
              }
              if (fabs(shortestAngle1) + fabs(shortestAngle2) > pi + marginRadians) selectAnim = false;

            }

          }
        }

        else if (FloatToEnumVelocity(animations.at(i)->GetIncomingVelocity()) == e_Velocity_Idle) {

          // allow only same angle (which is moving anims with 0 outgoing body angle. since @ idle, that will become their only angle)
          //if (fabs(animations.at(i)->GetIncomingBodyDirection().GetAngle2D(query.incomingBodyDirection)) > marginRadians) selectAnim = false;
          if (query.incomingBodyDirection_Strict == true) {
            if (fabs(Vector3(0, -1, 0).GetAngle2D(query.incomingBodyDirection)) > marginRadians) selectAnim = false;
          } else {
            if (fabs(Vector3(0, -1, 0).GetAngle2D(query.incomingBodyDirection)) > 0.25f * pi + marginRadians) selectAnim = false;
          }

        }

      }

      // no backwards body angles (test) if (fabs(animations.at(i)->GetIncomingBodyAngle()) > 0.5 * pi || fabs(animations.at(i)->GetOutgoingBodyAngle()) > 0.5 * pi) selectAnim = false;
    }


    // select by INCOMING BALL DIRECTION

    if (selectAnim) {
      if (query.byIncomingBallDirection == true) {
        Vector3 animBallDirection = GetVectorFromString(animations.at(i)->GetVariable("incomingballdirection"));
        if (animBallDirection.GetLength() < 0.1f) {
          Log(e_FatalError, "AnimCollection", "Crudeselection", "Anim " + animations.at(i)->GetName() + " missing incoming ball direction");
        }
        if (animBallDirection.GetLength() != 0.0f && query.incomingBallDirection.GetLength() != 0.0f) {

          // decimate height diff
          animBallDirection.coords[2] *= 0.4f;
          animBallDirection.Normalize();
          Vector3 adaptedIncomingBallDirection = query.incomingBallDirection;
          adaptedIncomingBallDirection.coords[2] *= 0.4f;
          adaptedIncomingBallDirection.Normalize();

          //float ballDirectionSimilarity = adaptedIncomingBallDirection.GetDotProduct(animBallDirection);// * 0.5 + 0.5;
          radian ballDirectionAngle = fabs(adaptedIncomingBallDirection.GetAngle2D(animBallDirection));
          //printf("%s\n", animations.at(i)->GetName().c_str());
          //query.incomingBallDirection.Print();
          //animBallDirection.Print();
          //printf("%f\n", ballDirectionDiff);
          radian maxDeviation = fabs(atof(animations.at(i)->GetVariable("incomingballdirection_maxdeviation").c_str()) * pi);
          if (maxDeviation == 0.0f) {
            maxDeviation = maxIncomingBallDirectionDeviation;//0.45f;
            if (animType.compare(defString[e_DefString_Deflect]) == 0) maxDeviation = 0.4f * pi;
          }
          if (ballDirectionAngle > maxDeviation) selectAnim = false;
        }
      }
    }


    // select by OUTGOING BALL DIRECTION

    if (selectAnim) {
      if (query.byOutgoingBallDirection == true) {
        Vector3 animBallDirection = GetVectorFromString(animations.at(i)->GetVariable("balldirection"));
        animBallDirection.Normalize(Vector3(0));
        //printf("%s\n", animations.at(i)->GetName().c_str());
        //float ballDirectionSimilarity = query.outgoingBallDirection.Get2D().GetNormalized(animBallDirection).GetDotProduct(animBallDirection);
        radian ballDirectionAngle = fabs(query.outgoingBallDirection.Get2D().GetNormalized(animBallDirection).GetAngle2D(animBallDirection));
        //query.outgoingBallDirection.Print();
        //animBallDirection.Print();
        //printf("%f\n", ballDirectionDiff);
        radian maxDeviation = fabs(atof(animations.at(i)->GetVariable("outgoingballdirection_maxdeviation").c_str()) * pi);
        if (maxDeviation == 0.0) {
          maxDeviation = maxOutgoingBallDirectionDeviation;
        }
        if (ballDirectionAngle > maxDeviation) selectAnim = false;
      }
    }


    // select by PROPERTIES

    if (selectAnim) {
      if (query.properties.Get("incoming_special_state").compare(animations.at(i)->GetVariable("incoming_special_state")) != 0) selectAnim = false;
      // hax: allow switching of hands (except for deflect anims) (in future, maybe make special case for 'both hands at the same time')
      if ((query.functionType == e_FunctionType_Deflect || ((query.properties.Get("incoming_retain_state").compare("") != 0) != (animations.at(i)->GetVariable("incoming_retain_state").compare("") != 0))) &&
          query.properties.Get("incoming_retain_state").compare(animations.at(i)->GetVariable("incoming_retain_state")) != 0) selectAnim = false;
      if (atof(query.properties.Get("specialvar1").c_str()) != atof(animations.at(i)->GetVariable("specialvar1").c_str())) selectAnim = false;
      if (atof(query.properties.Get("specialvar2").c_str()) != atof(animations.at(i)->GetVariable("specialvar2").c_str())) selectAnim = false;
    }


    // select by TRIP TYPE

    if (selectAnim) {
      if (query.byTripType == true) {
        if (int(round(atof(animations.at(i)->GetVariable("triptype").c_str()))) != query.tripType) selectAnim = false;
      }
    }


    // select by FORCED FOOT
    // todonow: unit test! not sure if working correctly

    if (selectAnim) {
      if (query.heedForcedFoot == true) {

        std::string forcedFoot = animations.at(i)->GetVariable("forcedfoot");
        int which = 0;
        if (forcedFoot.compare("strong") == 0) which = 1;
        else if (forcedFoot.compare("weak") == 0) which = 2;
        if (which != 0) {

          std::string touchFoot = animations.at(i)->GetVariable("touchfoot");
          e_Foot animFoot = e_Foot_Right;
          if (touchFoot.compare("left") == 0) animFoot = e_Foot_Left;

          // for mirrored anims that, therefore, don't start with right foot
          if (animations.at(i)->GetCurrentFoot() == e_Foot_Left) {
            if (animFoot == e_Foot_Left) animFoot = e_Foot_Right; else animFoot = e_Foot_Left;
          }

          if (which == 1 && query.strongFoot != animFoot) selectAnim = false;
          if (which == 2 && query.strongFoot == animFoot) selectAnim = false;
          //if (!selectAnim) printf("deleting %s\n", animations.at(i)->GetName().c_str());
        }
      }
    }


    if (selectAnim) dataSet.push_back(i);

  }
}

int AnimCollection::GetQuadrantID(Animation *animation, const Vector3 &movement, radian angle) const {
    // assign the animation it's rightful quadrant

  Vector3 adaptedMovement = movement.GetNormalized(0) * RangeVelocity(movement.GetLength());
  int quadrantID = 0;
  float shortestDistance = 100000.0f;
  for (unsigned int i = 0; i < quadrants.size(); i++) {
    float distance = adaptedMovement.GetDistance(quadrants.at(i).position);
    if (distance < shortestDistance) {
      shortestDistance = distance;
      quadrantID = quadrants.at(i).id;
    }
  }

  return quadrantID;
}

// adds touches around main touch
int AddExtraTouches(Animation* animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap) {
  Vector3 animBallPos;
  int animTouchFrame = -1;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);
  //printf("touchframe: %i\n", animTouchFrame);
  if (isTouch) {
    //printf("[touchframe: %i(%i); nodeMap size: %i] ", animTouchFrame, animation->GetFrameCount(), nodeMap.size());

    // find out what body part the balltouchpos is closest to
    animation->Apply(nodeMap, animTouchFrame, 0, false);

    boost::intrusive_ptr<Object> closestBodyPart = (*bodyParts.begin());
    float closestDistance = 100;
    Vector3 toBallVector = Vector3(0);
    std::list < boost::intrusive_ptr<Object> > ::const_iterator iter = bodyParts.begin();

    while (iter != bodyParts.end()) {
      float distance = (animBallPos - (*iter)->GetDerivedPosition()).GetLength();
      if (distance < closestDistance) {
        closestDistance = distance;
        closestBodyPart = *iter;
        toBallVector = animBallPos - (*iter)->GetDerivedPosition();
      }
      iter++;
    }
    //printf("closest: %s\n", closestBodyPart->GetName().c_str());
    animation->SetVariable("touch_bodypart", closestBodyPart->GetName());

    return animTouchFrame; // XDEBUG disable this

    int heightCheat = 0.0f;
    if (animBallPos.coords[2] > 0.8f) heightCheat = 2.0f;

    int range_pre = 2 + heightCheat;
    int range_post = 2 + heightCheat;

    int frameOffset = 0;

    boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->DeleteKeyFrame(animTouchFrame);

/*
    float bodypartBias = 0.9f;
    std::string animType = animation->GetVariable("type");
    if (animType.compare("ballcontrol") != 0) {
    //if (animType.compare("shortpass") == 0 || animType.compare("highpass") == 0 || animType.compare("shot") == 0) {
      bodypartBias = 0.95f;
    }
*/
    float bodypartBias = 1.0f;

    for (int i = animTouchFrame - range_pre; i < animTouchFrame + range_post + 1; i += 1) {
      if (i >= 0 && i < animation->GetFrameCount() - frameOffset - 1) {

        // set animation to this frame
        animation->Apply(nodeMap, i, 0, false);

        // find new ball position, based on the closest body part's position in this frame
        Vector3 position = closestBodyPart->GetDerivedPosition() + toBallVector;
        position.coords[2] = position.coords[2] * 0.4f + animBallPos.coords[2] * 0.6f; // take anim's height more seriously

        // correction for animation smoothing
        Vector3 origBodyPartPos = closestBodyPart->GetDerivedPosition() * bodypartBias + nodeMap.find("player")->second->GetDerivedPosition() * (1.0 - bodypartBias);

        Vector3 futureBodyPartPos = origBodyPartPos;
        // set anim to expected frame
        animation->Apply(nodeMap, i + frameOffset, 0, false);
        futureBodyPartPos = closestBodyPart->GetDerivedPosition() * bodypartBias + nodeMap.find("player")->second->GetDerivedPosition() * (1.0 - bodypartBias);

        //origBodyPos.Print();
        Vector3 diff2D = (futureBodyPartPos - origBodyPartPos).Get2D();

        Vector3 resultPosition = position + diff2D;

        // scale ballposition: this is to correct for the change in animation 'player dud' scale and actual scale (on average - since players may have different heights, of course)
        resultPosition.coords[0] = nodeMap.find("player")->second->GetDerivedPosition().coords[0] * 0.12f + resultPosition.coords[0] * 0.88f; // for some reason, anim ball pos is way too far from body (todo: investigate)
        resultPosition.coords[1] = nodeMap.find("player")->second->GetDerivedPosition().coords[1] * 0.12f + resultPosition.coords[1] * 0.88f;

        /*
        // experiment: ball more on the sides (y) to get more incoming anti-outgoingdir-movement before touch (aesthetics effect?)
        Vector3 outVec = Vector3(0, -1, 0).GetRotated2D(animation->GetOutgoingAngle() + animation->GetOutgoingBodyAngle());
        float dot = 1.0f - fabs(Vector3(0, -1, 0).GetDotProduct(outVec));
        resultPosition += outVec * dot * 0.30f;
        */

        Quaternion orientation;
        boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->SetKeyFrame(i + frameOffset, orientation, resultPosition, 0);
      }
    }
    return animTouchFrame + frameOffset;
  }

  return animTouchFrame; // default
}

float CalculateAnimDifficulty(Animation *animation, float &absoluteDifficulty) {
  Vector3 animBallPos;
  int animTouchFrame;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);

  float bodyDirDifficulty = clamp(fabs(animation->GetIncomingBodyDirection().GetAngle2D(animation->GetOutgoingBodyDirection()) / pi), 0.0, 1.0);

  float directionDifficulty = clamp(fabs(Vector3(0, -1, 0).GetAngle2D(animation->GetOutgoingDirection()) / pi), 0.0, 1.0);

  float veloChangeDifficulty = clamp(fabs(animation->GetIncomingVelocity() - animation->GetOutgoingVelocity()) / sprintVelocity, 0.0, 1.0);
  float accelDifficulty = clamp((animation->GetOutgoingVelocity() - animation->GetIncomingVelocity()) / sprintVelocity, 0.0, 1.0);
  float veloDifficulty = veloChangeDifficulty * 0.5 + accelDifficulty * 0.5; // decelerating is easier than accelerating

  float averageVelocity = clamp((animation->GetIncomingVelocity() + animation->GetOutgoingVelocity()) / (sprintVelocity * 2.0), 0.0, 1.0);
  float movementDifficulty = clamp((animation->GetIncomingMovement() - animation->GetOutgoingMovement()).GetLength() / sprintVelocity, 0.0, 1.0) *
                             pow(averageVelocity, 2.0f);

  float bodyDirDifficultyWeight = 0.5f;
  float directionDifficultyWeight = 1.0f;
  float veloDifficultyWeight = 1.0f;
  float movementDifficultyWeight = 4.0f;

  float result = bodyDirDifficulty * bodyDirDifficultyWeight +
                 directionDifficulty * directionDifficultyWeight +
                 veloDifficulty * veloDifficultyWeight +
                 movementDifficulty * movementDifficultyWeight;

  result /= bodyDirDifficultyWeight + directionDifficultyWeight + veloDifficultyWeight + movementDifficultyWeight;

  float expectedFrameCount = 20 + result * 80;

  //float relativeDifficultyFactor = expectedFrameCount / (float)animation->GetFrameCount(); // < 1 == speed up, > 1 == slow down
  float absoluteDifficultyFactor = result; // towards 0.0 == easy .. towards 1.0 == hard
  //float frameCountBias = 0.9f; // towards 0.0 == absolute difficulty, disregard how many frames the anim takes to do something difficult, towards 1.0 == relative difficulty, if anim takes enough frames to do hard stuff then it's a wrap already
  //float factor = relativeDifficultyFactor * frameCountBias + absoluteDifficultyFactor * (1.0 - frameCountBias);

  // result must be in format 0 == current, 1 == much slower. ignore anims that should be speeded up (it's animators task to keep them at least fast enough == impose slowest possible)
  //float relativeDifficulty = clamp(relativeDifficultyFactor, 0.0, 2.0);
  absoluteDifficulty = clamp(absoluteDifficultyFactor, 0.0, 1.0);

  if (isTouch) {
    expectedFrameCount *= 1.1f;
    expectedFrameCount += 4;
  }

  absoluteDifficulty *= 0.88f;
  if (isTouch) absoluteDifficulty += 0.12f;

  //if ((int)expectedFrameCount > animation->GetFrameCount())
  //  printf("%s: framenum offset: %i, abs: %f\n", animation->GetName().c_str(), (int)expectedFrameCount - animation->GetFrameCount(), absoluteDifficulty);

  //printf("%f\n", difficulty);
  //return relativeDifficulty;

  expectedFrameCount = clamp(expectedFrameCount * 1.0f, 1, animation->GetEffectiveFrameCount() + 16);
  return expectedFrameCount;
}

void Slowdown(Animation *animation, float veloFactor, float expectedFrameCount, bool debug = false) {
  // stretches animations without changing their velocities

  assert(veloFactor >= 0.1f);
  assert(veloFactor <= 1.0f);

  int insertedFrames = 0.0;
  Quaternion orientation; // dud
  float power; // dud
  Vector3 position;

  int targetFrameCount = clamp(round(expectedFrameCount), animation->GetEffectiveFrameCount(), 1000000); // will not come out perfectly correct, because we can only add between frames (not before first/after last)

  if (targetFrameCount > animation->GetFrameCount()) {
    float insertsPerFrame = ((float)targetFrameCount / (float)animation->GetFrameCount()) - 1.0;
    int originalFrameCount = animation->GetFrameCount();

    float overflowCounter = insertsPerFrame; // when do we need to insert a frame?

    e_Velocity originalIncomingVelocity = FloatToEnumVelocity(animation->GetIncomingVelocity());
    e_Velocity originalOutgoingVelocity = FloatToEnumVelocity(animation->GetOutgoingVelocity());

    for (int frame = 1; frame < animation->GetFrameCount(); frame++) {

      while (overflowCounter >= 1.0f) {
        animation->Shift(frame, 1);
        overflowCounter -= 1.0f;

        frame++;
        insertedFrames++;
      }

      float currentShiftFactor = (float)(frame + insertedFrames) / (float)frame;

      // stretch position to retain proper velocity
      bool isFrame = animation->GetKeyFrame("player", frame, orientation, position);
      if (isFrame) {
        position.coords[0] *= currentShiftFactor;
        position.coords[1] *= currentShiftFactor;
        animation->SetKeyFrame("player", frame, orientation, position);
      }
      isFrame = animation->GetExtension("football")->GetKeyFrame(frame, orientation, position, power);
      if (isFrame) {
        position.coords[0] *= currentShiftFactor;
        position.coords[1] *= currentShiftFactor;
        animation->GetExtension("football")->SetKeyFrame(frame, orientation, position, power);
      }

      //printf("%f\n", currentShiftFactor);
      overflowCounter += insertsPerFrame;
    }

    if (debug) {
      if (FloatToEnumVelocity(animation->GetIncomingVelocity()) != originalIncomingVelocity) printf("incoming: %s: %f to %f\n", animation->GetName().c_str(), EnumToFloatVelocity(originalIncomingVelocity), RangeVelocity(animation->GetIncomingVelocity()));
      if (FloatToEnumVelocity(animation->GetOutgoingVelocity()) != originalOutgoingVelocity) printf("outgoing: %s: %f to %f\n", animation->GetName().c_str(), EnumToFloatVelocity(originalOutgoingVelocity), RangeVelocity(animation->GetOutgoingVelocity()));
    }

  }

}

void SmoothPositions(Animation *animation, bool convertAngledDribbleToWalk) {

  float bias = 1.0, exp = 1.0;

  if (animation->GetAnimType().compare("movement") == 0) {
    bias = 0.5;
    exp = 0.8 + pow(clamp(animation->GetOutgoingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.5) * 0.4 +
              + pow(clamp(animation->GetOutgoingVelocity() - animation->GetIncomingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.3) * 0.4;
  } else if (animation->GetAnimType().compare("ballcontrol") == 0) {
    bias = 0.3;
    exp = 0.9 + pow(clamp(animation->GetOutgoingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.5) * 0.5 +
              + pow(clamp(animation->GetOutgoingVelocity() - animation->GetIncomingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.3) * 0.5;
  } else if (animation->GetAnimType().compare("trap") == 0) {
    bias = 0.3;
    exp = 0.9 + pow(clamp(animation->GetOutgoingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.5) * 0.5 +
              + pow(clamp(animation->GetOutgoingVelocity() - animation->GetIncomingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.3) * 0.5;
  } else if (animation->GetAnimType().compare("interfere") == 0) {
    bias = 0.3;
    exp = 0.7 + pow(clamp(animation->GetOutgoingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.5) * 0.5 +
              + pow(clamp(animation->GetOutgoingVelocity() - animation->GetIncomingVelocity(), 0, sprintVelocity) / sprintVelocity, 1.3) * 0.5;
  } else return;

  e_Velocity originalIncomingVelocity = FloatToEnumVelocity(animation->GetIncomingVelocity());
  e_Velocity originalOutgoingVelocity = FloatToEnumVelocity(animation->GetOutgoingVelocity());

  Quaternion orientation; // dud
  Vector3 origPosition;
  Vector3 prevPosition;
  Quaternion touchOrientation; // dud
  Vector3 origTouchPosition;
  float power; // dud

  // backup previous positions
  Vector3 origPositions[animation->GetFrameCount()];
  for (int frame = 1; frame < animation->GetFrameCount(); frame++) {
    animation->GetKeyFrame("player", frame, orientation, origPositions[frame]);
  }

  Vector3 incoming = animation->GetIncomingMovement();
  Vector3 outgoing = animation->GetOutgoingMovement();

  //bias = 0.0f; // !!!!!!! disables smoothing !!!!!!! (except for the convert thing below)
  if (convertAngledDribbleToWalk) {
    if (FloatToEnumVelocity(animation->GetIncomingVelocity()) == e_Velocity_Dribble) { incoming.Normalize(0); incoming *= walkVelocity; bias = 1.0; }
    if (FloatToEnumVelocity(animation->GetOutgoingVelocity()) == e_Velocity_Dribble && fabs(animation->GetOutgoingAngle()) < 0.5 * pi) { outgoing.Normalize(0); outgoing *= walkVelocity; bias = 1.0; }
  }

  animation->GetKeyFrame("player", 0, orientation, origPosition);
  prevPosition = origPosition;

  for (int frame = 1; frame < animation->GetFrameCount(); frame++) {
    float frameBias = (float)(frame - 1) / (float)(animation->GetFrameCount() - 3); // first 2 and last 2 frames need to be bias 0 and 1, so our anim keeps having the original in- and outgoing movement
    frameBias = pow(clamp(frameBias, 0.0, 1.0), exp);
    //printf("%f, ", bias);
    Vector3 movement = incoming * (1 - frameBias) + outgoing * frameBias;
    //movement.Print();

    bool touchFrame = animation->GetExtension("football")->GetKeyFrame(frame, touchOrientation, origTouchPosition, power);

    animation->GetKeyFrame("player", frame, orientation, origPosition);
    Vector3 smoothPosition = prevPosition + movement * 0.01;
    smoothPosition.coords[2] = origPosition.coords[2];
    Vector3 resultingPosition = smoothPosition * bias + origPositions[frame] * (1.0 - bias);
    animation->SetKeyFrame("player", frame, orientation, resultingPosition);

    Vector3 resultingTouchPosition = origTouchPosition + (resultingPosition - origPosition);
    if (touchFrame) animation->GetExtension("football")->SetKeyFrame(frame, touchOrientation, resultingTouchPosition, power);

    prevPosition = smoothPosition;
  }

//  if (FloatToEnumVelocity(animation->GetIncomingVelocity()) != originalIncomingVelocity) printf("incoming: %s: %f to %f\n", animation->GetName().c_str(), EnumToFloatVelocity(originalIncomingVelocity), RangeVelocity(animation->GetIncomingVelocity()));
//  if (FloatToEnumVelocity(animation->GetOutgoingVelocity()) != originalOutgoingVelocity) printf("outgoing: %s: %f to %f\n", animation->GetName().c_str(), EnumToFloatVelocity(originalOutgoingVelocity), RangeVelocity(animation->GetOutgoingVelocity()));
}

void AnimCollection::_PrepareAnim(Animation *animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap, bool convertAngledDribbleToWalk) {

  //animation->Hax();

  Vector3 animBallPos;
  int animTouchFrame;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);
  if (isTouch && (animation->GetAnimType() == "movement" || animation->GetAnimType() == "trip" || animation->GetAnimType() == "special")) printf("invalid ball touch for animtype: %s\n", animation->GetName().c_str());
  if (!isTouch && (animation->GetAnimType() != "movement" && animation->GetAnimType() != "trip" && animation->GetAnimType() != "special")) printf("invalid ball touch for animtype: %s\n", animation->GetName().c_str());

  float absDiff;
  float expectedFrameCount = CalculateAnimDifficulty(animation, absDiff);
  animation->SetVariable("animdifficultyfactor", real_to_str(absDiff));
  //printf("diff: %f\n", absDiff);

  //Slowdown(animation, 1.0f, expectedFrameCount, true);
  //SmoothPositions(animation, convertAngledDribbleToWalk);

  int touchFrame = AddExtraTouches(animation, playerNode, bodyParts, nodeMap);
  animation->SetVariable("touchframe", int_to_str(touchFrame));


  // assign the animation its rightful quadrant

  Vector3 movement = animation->GetOutgoingMovement();
  radian angle = animation->GetOutgoingAngle();
  int quadrantID = GetQuadrantID(animation, movement, angle);

  animation->SetVariable("quadrant_id", int_to_str(quadrantID));
  //printf("quadrant: %i\n", quadrantID);


  animations.push_back(animation);

  //animation->Save(files.at(i));

  //XMLLoader loader;
  //loader.PrintTree((*animation->GetCustomData()));
}

inline bool AnimCollection::_CheckFunctionType(const std::string &functionType, e_FunctionType queryFunctionType) const {

  bool rightType = false;

  switch (queryFunctionType) {

    case e_FunctionType_Movement:
      if (functionType == defString[e_DefString_Movement]) rightType = true;
      break;

    case e_FunctionType_BallControl:
      if (functionType == defString[e_DefString_BallControl]) rightType = true;
      break;

    case e_FunctionType_Trap:
      if (functionType == defString[e_DefString_Trap]) rightType = true;
      break;

    case e_FunctionType_ShortPass:
      if (functionType == defString[e_DefString_ShortPass]) rightType = true;
      break;

    case e_FunctionType_LongPass:
      if (functionType == defString[e_DefString_LongPass]) rightType = true;
      break;

    case e_FunctionType_HighPass:
      if (functionType == defString[e_DefString_HighPass]) rightType = true;
      break;

    case e_FunctionType_Shot:
      if (functionType == defString[e_DefString_Shot]) rightType = true;
      break;

    case e_FunctionType_Deflect:
      if (functionType == defString[e_DefString_Deflect]) rightType = true;
      break;

    case e_FunctionType_Catch:
      if (functionType == defString[e_DefString_Catch]) rightType = true;
      break;

    case e_FunctionType_Interfere:
      if (functionType == defString[e_DefString_Interfere]) rightType = true;
      break;

    case e_FunctionType_Trip:
      if (functionType == defString[e_DefString_Trip]) rightType = true;
      break;

    case e_FunctionType_Sliding:
      if (functionType == defString[e_DefString_Sliding]) rightType = true;
      break;

    case e_FunctionType_Special:
      if (functionType == defString[e_DefString_Special]) rightType = true;
      break;

    default:
      rightType = false;
      break;
  }

  return rightType;
}
