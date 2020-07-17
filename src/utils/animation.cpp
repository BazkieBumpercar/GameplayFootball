// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "animation.hpp"

#include "base/utils.hpp"

#include <stdio.h>

#include "animationextensions/footballanimationextension.hpp"

std::string emptyString = "";

namespace blunted {

  // todo: make default orientation changeable
  radian FixAngle(radian angle, bool modulateIntoRange = true) {
    // convert engine angle into football angle (different base orientation: 'down' on y instead of 'right' on x)
    radian newAngle = angle;
    newAngle += 0.5 * pi;
    if (modulateIntoRange) newAngle = ModulateIntoRange(-pi, pi, newAngle);
    return newAngle;
  }

  Animation::Animation() {
    frameCount = 0;
    // all humanoid movies are supposed to have a moving right foot at first (unless mirrored)
    currentFoot = e_Foot_Right;

    DirtyCache();
  }

  Animation::Animation(const Animation &src) {
    int animSize = src.nodeAnimations.size();
    for (int i = 0; i < animSize; i++) {
      nodeAnimations.push_back(new NodeAnimation(*src.nodeAnimations.at(i)));
    }

    frameCount = src.frameCount;
    name = src.name;

    // attention! shallow copy!
    extensions = src.extensions;

    boost::shared_ptr<XMLTree> tmpCustomData(new XMLTree(*src.customData));
    customData = tmpCustomData;

    std::map<const char*, std::string>::const_iterator varCacheIter = src.variableCache.begin();
    while (varCacheIter != src.variableCache.end()) {
      char *varName = new char[256];
      memcpy(varName, varCacheIter->first, 256 * sizeof(char));
      variableCache.insert(std::pair<const char*, std::string>(varName, varCacheIter->second));
      varCacheIter++;
    }

    currentFoot = src.currentFoot;

    cache_translation_dirty = src.cache_translation_dirty;
    cache_translation = src.cache_translation;
    cache_incomingMovement_dirty = src.cache_incomingMovement_dirty;
    cache_incomingMovement = src.cache_incomingMovement;
    cache_incomingVelocity_dirty = src.cache_incomingVelocity_dirty;
    cache_incomingVelocity = src.cache_incomingVelocity;
    cache_outgoingDirection_dirty = src.cache_outgoingDirection_dirty;
    cache_outgoingDirection = src.cache_outgoingDirection;
    cache_outgoingMovement_dirty = src.cache_outgoingMovement_dirty;
    cache_outgoingMovement = src.cache_outgoingMovement;
    cache_rangedOutgoingMovement_dirty = src.cache_rangedOutgoingMovement_dirty;
    cache_rangedOutgoingMovement = src.cache_rangedOutgoingMovement;
    cache_outgoingVelocity_dirty = src.cache_outgoingVelocity_dirty;
    cache_outgoingVelocity = src.cache_outgoingVelocity;
    cache_angle_dirty = src.cache_angle_dirty;
    cache_angle = src.cache_angle;
    cache_incomingBodyAngle_dirty = src.cache_incomingBodyAngle_dirty;
    cache_incomingBodyAngle = src.cache_incomingBodyAngle;
    cache_outgoingBodyAngle_dirty = src.cache_outgoingBodyAngle_dirty;
    cache_outgoingBodyAngle = src.cache_outgoingBodyAngle;
    cache_incomingBodyDirection_dirty = src.cache_incomingBodyDirection_dirty;
    cache_incomingBodyDirection = src.cache_incomingBodyDirection;
    cache_outgoingBodyDirection_dirty = src.cache_outgoingBodyDirection_dirty;
    cache_outgoingBodyDirection = src.cache_outgoingBodyDirection;

    cache_AnimType = src.cache_AnimType;
  }

  Animation::~Animation() {
    Reset();
  }


  void Animation::DirtyCache() {
    cache_translation_dirty = true;
    cache_incomingMovement_dirty = true;
    cache_incomingVelocity_dirty = true;
    cache_outgoingDirection_dirty = true;
    cache_outgoingMovement_dirty = true;
    cache_rangedOutgoingMovement_dirty = true;
    cache_outgoingVelocity_dirty = true;
    cache_angle_dirty = true;
    cache_incomingBodyAngle_dirty = true;
    cache_outgoingBodyAngle_dirty = true;
    cache_incomingBodyDirection_dirty = true;
    cache_outgoingBodyDirection_dirty = true;
  }

  int Animation::GetFrameCount() const {
    return frameCount;
  }

  bool Animation::GetKeyFrame(std::string nodeName, int frame, Quaternion &orientation, Vector3 &position, bool getOrientation, bool getPosition) const {
    int animSize = nodeAnimations.size();
    for (int i = 0; i < animSize; i++) {
      if (nodeAnimations.at(i)->nodeName == nodeName) {
        GetInterpolatedValues(nodeAnimations.at(i)->animation, frame, orientation, position, getOrientation, getPosition);
        return nodeAnimations.at(i)->animation.find(frame) != nodeAnimations.at(i)->animation.end();
      }
    }
    return false;
  }

  void Animation::SetKeyFrame(std::string nodeName, int frame, const Quaternion &orientation, const Vector3 &position) {
    if (frame >= frameCount) frameCount = frame + 1;

    NodeAnimation *nodeAnimation = 0;

    // find node
    int animSize = nodeAnimations.size();
    for (int i = 0; i < animSize; i++) {
      if (nodeAnimations.at(i)->nodeName == nodeName) {
        nodeAnimation = nodeAnimations.at(i);
        break;
      }
    }

    // node doesn't exist yet?
    if (nodeAnimation == 0) {
      nodeAnimation = new NodeAnimation();
      nodeAnimation->nodeName = nodeName;
      nodeAnimations.push_back(nodeAnimation);
    }

    // find frame
    const std::map<int, KeyFrame>::iterator animIter = nodeAnimation->animation.find(frame);
    if (animIter != nodeAnimation->animation.end()) {
      // change
      animIter->second.orientation = orientation;
      animIter->second.position = position;
    } else {
      // insert
      KeyFrame keyFrame;
      keyFrame.orientation = orientation;
      keyFrame.position = position;
      nodeAnimation->animation.insert(std::pair<int, KeyFrame>(frame, keyFrame));
    }

    DirtyCache();
  }

  void Animation::DeleteKeyFrame(std::string nodeName, int frame) {
    // iterate nodes
    int animSize = nodeAnimations.size();
    for (int i = 0; i < animSize; i++) {
      NodeAnimation *nodeAnimation = nodeAnimations.at(i);
      if (nodeAnimation->nodeName == nodeName) {
        // find frame
        const std::map<int, KeyFrame>::iterator animIter = nodeAnimation->animation.find(frame);
        if (animIter != nodeAnimation->animation.end()) {
          nodeAnimation->animation.erase(animIter);
          break;
        }
        if (animIter == nodeAnimation->animation.end()) printf("ERROR! keyframe does not exist\n");
      }
    }

    DirtyCache();
  }

  void Animation::GetInterpolatedValues(const std::map<int, KeyFrame> &animation, int frame, Quaternion &orientation, Vector3 &position, bool getOrientation, bool getPosition) const {
    std::vector<WeighedKey> weighedKeys;
    // todo: rename weighing to weighting, omg

    position = Vector3(0);
    orientation = QUATERNION_IDENTITY;

    if (frame > 0 && frame < GetFrameCount()) {
      std::map<int, KeyFrame>::const_iterator animIter = animation.begin();

      while (animIter != animation.end()) {

        // still before current frame and yet encountered keys? clear them, we don't need earlier keys
        if (animIter->first < frame && weighedKeys.size() > 0) weighedKeys.clear();

        // add key, hopefully this is the last one before our current frame, or the first after
        WeighedKey key;
        key.keyFrame = animIter->second;
        key.frame = animIter->first;
        weighedKeys.push_back(key);

        // if this keyframe came after our current frame, we've got everything we need, so bail out
        if (animIter->first >= frame) {
          animIter = animation.end();
        } else {
          animIter++;
        }
      }

      // we've now got either 1 keyframe (before/after/at our current frame) or 2, on both sides of (or 1 at) our keyframe.
      // calculate its/their influence
      float bias = 0;

      if (weighedKeys.size() == 1) {

        WeighedKey &key = weighedKeys.at(0);
        key.influence = 1;

      } else if (weighedKeys.size() == 2) {

        // distance between keyframes and current frame
        int distance = weighedKeys.at(1).frame - weighedKeys.at(0).frame;
        int distance1 = frame - weighedKeys.at(0).frame;
        int distance2 = weighedKeys.at(1).frame - frame;
        float ratio1 = 1 - (distance1 * 1.0) / (distance * 1.0);
        float ratio2 = 1 - (distance2 * 1.0) / (distance * 1.0);
        weighedKeys.at(0).influence = ratio1;
        weighedKeys.at(1).influence = ratio2;

        int relPosition = frame - weighedKeys.at(0).frame;
        bias = (relPosition * 1.0) / (distance * 1.0);

      }

      if (getOrientation) {
        if (weighedKeys.size() > 1) {
          orientation = weighedKeys.at(0).keyFrame.orientation.GetSlerped(bias, weighedKeys.at(1).keyFrame.orientation);
        } else {
          orientation = weighedKeys.at(0).keyFrame.orientation;
        }
      }

      if (getPosition) {
        for (unsigned int v = 0; v < weighedKeys.size(); v++) {
          position += weighedKeys.at(v).keyFrame.position * weighedKeys.at(v).influence;
        }
      }

    } else if (frame >= GetFrameCount()) {

      // extrapolate beyond animation
      // remember, weighed keys are in reverse order here!

      // get last 2 keyframes
      std::map<int, KeyFrame>::const_iterator animIter = animation.end();
      while (animIter != animation.begin()) {
        animIter--;

        // add key
        WeighedKey key;
        key.keyFrame = animIter->second;
        key.frame = animIter->first;
        weighedKeys.push_back(key);

        if (weighedKeys.size() == 2) {
          animIter = animation.begin();
        }
      }
      if (weighedKeys.size() == 0) return; // should not happen
      if (weighedKeys.size() == 1) {
        if (getOrientation) orientation = weighedKeys.at(0).keyFrame.orientation;
        if (getPosition) position = weighedKeys.at(0).keyFrame.position;
        return;
      }
      // extrapolate
      //position = weighedKeys.at(0).keyFrame.position; // ignore position
      float dist1 = weighedKeys.at(0).frame - weighedKeys.at(1).frame;
      float dist2 = frame - weighedKeys.at(0).frame;
      float bias = 1 + ((1 / dist1) * dist2);
      // extrapolation through hax: bias > 1 doesn't really give proper results. however, it works for small angles so FUFUUFUUUU XD
      // (use slerp instead of lerp - lerp can't extrapolate at all)
      //bias = clamp(bias, 0.0f, 1.0f);
      if (getOrientation) orientation = weighedKeys.at(1).keyFrame.orientation.GetSlerped(bias, weighedKeys.at(0).keyFrame.orientation);
      if (getPosition) position = weighedKeys.at(1).keyFrame.position * (1 - bias) + weighedKeys.at(0).keyFrame.position * bias;

    } else if (frame <= 0) {

      // extrapolate before animation
      // for now, just return first key

      std::map<int, KeyFrame>::const_iterator animIter = animation.begin();
      if (getOrientation) orientation = animIter->second.orientation;
      if (getPosition) position = animIter->second.position;

    }

  }

  void Animation::ConvertToStartFacingForwardIfIdle() {

    radian incomingBodyAngle = GetIncomingBodyAngle();

    // only applies to anims starting out idle
    if (GetIncomingVelocity() >= 1.8) return;

    // rotate player pos
    std::map<int, KeyFrame> &player = nodeAnimations.at(0)->animation;
    std::map<int, KeyFrame>::iterator animIter = player.begin();
    while (animIter != player.end()) {
      //animIter->second.position.Print();
      animIter->second.position.Rotate2D(-incomingBodyAngle);
      animIter++;
    }

    // rotate body dir
    std::map<int, KeyFrame> &body = nodeAnimations.at(1)->animation;
    animIter = body.begin();
    while (animIter != body.end()) {
      //animIter->second.position.Print();
      Quaternion rotation(QUATERNION_IDENTITY);
      Quaternion zRot;
      zRot.SetAngleAxis(-incomingBodyAngle, Vector3(0, 0, 1));
      rotation = zRot * rotation;
      animIter->second.orientation = rotation * animIter->second.orientation;
      //animIter->second.angles.coords[2] -= GetIncomingBodyAngle();
      animIter++;
    }

    std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter = extensions.begin();
    while (extensionIter != extensions.end()) {
      extensionIter->second->Rotate2D(-incomingBodyAngle);
      extensionIter++;
    }

    // variables
    Vector3 newBallDirVec = GetVariable("balldirection") != "" ? GetVectorFromString(GetVariable("balldirection")).GetRotated2D(-incomingBodyAngle) : 0;
    Vector3 newIncomingBallDirVec = GetVariable("incomingballdirection") != "" ? GetVectorFromString(GetVariable("incomingballdirection")).GetRotated2D(-incomingBodyAngle) : 0;
    Vector3 newBumpDirVec = GetVariable("bumpdirection") != "" ? GetVectorFromString(GetVariable("bumpdirection")).GetRotated2D(-incomingBodyAngle) : 0;
    SetVariable("balldirection", GetStringFromVector(newBallDirVec));
    SetVariable("incomingballdirection", GetStringFromVector(newIncomingBallDirVec));
    SetVariable("bumpdirection", GetStringFromVector(newBumpDirVec));

    DirtyCache();
  }

  void Animation::Invert() {

    NodeAnimation *nodeAnimation = 0;

    // simple keyframe-to-keyframe version
    int animSize = nodeAnimations.size();
    for (int i = 0; i < animSize; i++) {
      nodeAnimation = nodeAnimations.at(i);

      std::map<int, KeyFrame>::iterator keyIter = nodeAnimation->animation.begin();
      while (keyIter != nodeAnimation->animation.end()) {
        Quaternion &orient = (*keyIter).second.orientation;

        radian x, y, z;
        orient.GetAngles(x, y, z);
        Quaternion rotX, rotY, rotZ;
        rotX.SetAngleAxis(-x, Vector3(1, 0, 0));
        rotY.SetAngleAxis(-y, Vector3(0, 1, 0));
        rotZ.SetAngleAxis(-z, Vector3(0, 0, 1));
        orient = rotX * rotY * rotZ;

        keyIter++;
      }
    }
  }

  void Animation::Apply(const std::map < const std::string, boost::intrusive_ptr<Node> > nodeMap, int frame, int timeOffset_ms, bool smooth, float smoothFactor, /*const boost::shared_ptr<Animation> previousAnimation, int smoothFrames, */const Vector3 &basePos, radian baseRot, std::map < std::string, BiasedOffset > &offsets, MovementHistory *movementHistory, int timeDiff_ms, bool noPos, bool updateSpatial) {

    // simple keyframe-to-keyframe version

    NodeAnimation *nodeAnimation = 0;

    //int futureFrameOffset = 1;

    unsigned int animSize = nodeAnimations.size();
    for (unsigned int i = 0; i < animSize; i++) {

      nodeAnimation = nodeAnimations.at(i);

      Quaternion orientation;
      Vector3 position;

      // old: GetInterpolatedValues(nodeAnimation->animation, frame, orientation, position);

      // new, interpolated
      Quaternion orientation_pre, orientation_post;
      Vector3 position_pre, position_post;
      float bias = 0.5f;
      if (timeOffset_ms != -1) bias = clamp(timeOffset_ms / 10.0f, 0.0f, 1.0f);
      int smoothFrames = 0;
      if (smooth && 1 == 2) {
        smoothFrames = 0;
        if (GetAnimType() == "movement") smoothFrames = 1;
        float factor = (smoothFrames * 2) + 1;
        bias /= factor;
        bias += 0.5f - ((1.0f / factor) * 0.5f); // center, then subtract half of new size
      }
      GetInterpolatedValues(nodeAnimation->animation, frame     - smoothFrames, orientation_pre , position_pre);
      GetInterpolatedValues(nodeAnimation->animation, frame + 1 + smoothFrames, orientation_post, position_post);
      orientation_pre.MakeSameNeighborhood(orientation_post);
      orientation = orientation_pre.GetLerped(bias, orientation_post).GetNormalized();
      position = position_pre * (1.0f - bias) + position_post * bias;

      assert(nodeMap.find(nodeAnimation->nodeName) != nodeMap.end());

      if (nodeAnimation->nodeName.compare("player") == 0) {
        if (noPos) {
          position.coords[0] = 0;
          position.coords[1] = 0;
        } else {
          position.Rotate2D(baseRot);
        }
      }
      if (nodeAnimation->nodeName.compare("body") == 0) {
        Quaternion rotZ;
        rotZ.SetAngleAxis(baseRot, Vector3(0, 0, 1));
        orientation = rotZ * orientation;
        orientation.Normalize();
      }

      // offset
      std::map < std::string, BiasedOffset >::iterator iter = offsets.find(nodeAnimation->nodeName);
      if (iter != offsets.end()) {
        iter->second.orientation.MakeSameNeighborhood(orientation);
        if (iter->second.isRelative) {
          orientation = orientation.GetLerped(iter->second.bias, iter->second.orientation * orientation).GetNormalized();
        } else {
          orientation = orientation.GetLerped(iter->second.bias, iter->second.orientation).GetNormalized();
        }
      }


      // SMOOTHING

      if (smooth) {

        // needed for smoothing - keep track of old limb positions/movements so we can extrapolate those and use that for rotation change limit calculations

        assert(movementHistory);
        MovementHistoryEntry *movementHistoryEntry = 0;

        for (unsigned int node = 0; node < movementHistory->size(); node++) {
          if (movementHistory->at(node).nodeName.compare(nodeAnimation->nodeName) == 0) {
            movementHistoryEntry = &movementHistory->at(node);
          }
        }
        if (movementHistoryEntry == 0) { // not in movementhistory yet; add
          MovementHistoryEntry newEntry;
          newEntry.nodeName = nodeAnimation->nodeName;
          newEntry.position = position;
          newEntry.orientation = orientation;
          newEntry.timeDiff_ms = 10;
          movementHistory->push_back(newEntry);
          movementHistoryEntry = &movementHistory->back();
        }

        float beginBias = pow(curve(1.0f - NormalizedClamp(frame, 0, 8), 1.0f), 0.5f);
        float currentBias = 0.0f + beginBias * smoothFactor * 0.5f;

        if (nodeAnimation->nodeName.compare("player") != 0) {

          const Quaternion &previousOrientation = movementHistoryEntry->orientation;
          Quaternion currentOrientation = nodeMap.find(nodeAnimation->nodeName)->second->GetRotation();
          currentOrientation.MakeSameNeighborhood(previousOrientation);

          if (timeDiff_ms > 0) {

            bool simpleMethod = true; // non-simple-method bug: initial orientation seems to be off

            if (simpleMethod == false) {

              Quaternion identity(QUATERNION_IDENTITY);

              //Quaternion currentRotation = (previousOrientation.GetRotationTo(currentOrientation)).GetNormalized() * (1.0f / (movementHistoryEntry->timeDiff_ms * 0.001f));
              Quaternion currentRotation_per_ms = previousOrientation.GetRotationTo(currentOrientation).GetNormalized();
              currentRotation_per_ms = currentRotation_per_ms.GetRotationMultipliedBy(1.0f / (float)movementHistoryEntry->timeDiff_ms);// already normalized in function
              //currentOrientation.Normalize();
              //currentRotation.Normalize();

              // add some extra overall smoothness
              // todo: why does this work at all? let alone so well?! haha
              orientation = orientation.GetSlerped(pow(1.0f - clamp(timeDiff_ms / 30.0f, 0.0f, 1.0f), 0.5f) * (0.5f + smoothFactor * 0.5f * beginBias), currentOrientation).GetNormalized();
              //orientation = orientation.GetSlerped(pow(1.0f - clamp(timeDiff_ms / 30.0f, 0.0f, 1.0f), 0.5f), currentOrientation).GetNormalized();
              //orientation = orientation.GetSlerped(0.9f + 0.1f * beginBias, currentOrientation).GetNormalized();

              //float currentToDesiredDot = orientation.GetDotProduct(currentOrientation);
              float currentToDesiredDot = orientation.MakeSameNeighborhood(currentOrientation);


              // enforce max change

              radian angleDiff_per_ms = 2.0f * acos(clamp(currentToDesiredDot, -1.0f, 1.0f)) / (float)timeDiff_ms;
              radian maxDiff_per_ms = 5.0f * pi * 0.001f;
              //if (nodeAnimation->nodeName.compare("body") == 0) maxDiff_per_ms = 3.0f * pi * 0.001f;
              if (angleDiff_per_ms > maxDiff_per_ms) {
                // now make orientation into limited rotation * currentOrientation
                float allowFraction = maxDiff_per_ms / angleDiff_per_ms;
                Quaternion desiredRotation = currentOrientation.GetRotationTo(orientation).GetNormalized();
                //if (desiredRotation.elements[3] < 0) desiredRotation.elements[3] = -desiredRotation.elements[3];
                orientation = desiredRotation.GetRotationMultipliedBy(allowFraction) * currentOrientation;
                orientation.Normalize();
              }

      /*
              // noms too much cpu?
              // damping (maybe useful: http://www.freebasic.net/forum/viewtopic.php?t=9769 )
              Quaternion desiredRotation_per_ms = currentOrientation.GetRotationTo(orientation).GetNormalized();
              desiredRotation_per_ms = desiredRotation_per_ms.GetRotationMultipliedBy(1.0f / (float)timeDiff_ms);
              //assert(currentRotation_per_ms.GetDotProduct(desiredRotation_per_ms) >= 0.0f);
              //float dot = currentRotation_per_ms.MakeSameNeighborhood(desiredRotation_per_ms);
              Quaternion dampingRotation_per_ms = currentRotation_per_ms.GetRotationTo(desiredRotation_per_ms).GetNormalized();
              dot = dampingRotation_per_ms.GetDotProduct(currentRotation_per_ms);
              radian angle_sec = 2.0f * acos(clamp(dot, -1.0f, 1.0f)) * 1000.0f; // basically: angle between desired and current rotation (per second)
              if (angle_sec > 0.0f) {
                radian dampAngle_sec = 0.15f * pi;
                float dampFactor = NormalizedClamp(dampAngle_sec, 0.0f, angle_sec);

                if (currentRotation_per_ms.GetDotProduct(identity) < desiredRotation_per_ms.GetDotProduct(identity)) dampFactor = 1.0f; // slowing down joints is easier than accelerating

                // now change (hax!) currentRotation so that it's closer to the desiredRotation
                //XXcurrentRotation_per_ms = dampingRotation_per_ms.GetRotationMultipliedBy(dampFactor) * currentRotation_per_ms;
              }
      */
      /*
              Quaternion extrapolatedOrientation = currentRotation_per_ms.GetRotationMultipliedBy(timeDiff_ms) * currentOrientation;
              //Quaternion extrapolatedOrientation = desiredRotation_per_ms.GetRotationMultipliedBy(timeDiff_ms) * currentOrientation;
              extrapolatedOrientation.Normalize();

              // now we want to go from current to new orientation, but we can only differ so much from the rotation from previous to current orientation
              float maxTimeDiff_ms = 30.0f; // higher == smoother. in this amount of time, we can change direction completely.
              //if ((nodeAnimation->nodeName.compare("body") == 0 || nodeAnimation->nodeName.compare("middle") == 0)) maxTimeDiff_ms = 60.0f;
              float movementInfluence = 1.0f - clamp(timeDiff_ms / maxTimeDiff_ms, 0.0f, 1.0f);
              movementInfluence = pow(movementInfluence, 0.5f); // influence of old movement wears off in exponential fashion (source: laws of nature)
              //movementInfluence *= 0.5f + beginBias * 0.5f;
              //movementInfluence *= beginBias;
              //movementInfluence = 1.0f;

              orientation = orientation.GetSlerped(movementInfluence, extrapolatedOrientation).GetNormalized();
      */

              Quaternion desiredRotation_per_ms = currentOrientation.GetRotationTo(orientation).GetNormalized();
              desiredRotation_per_ms = desiredRotation_per_ms.GetRotationMultipliedBy(1.0f / (float)timeDiff_ms);

              // now get the resulting rotation, which is the current rotation, converged as much towards the desired rotation as allowed, based upon maximum rotational angle change per time unit
              Quaternion resultingRotation_per_ms = desiredRotation_per_ms;
              float currentToDesiredRotationDot = desiredRotation_per_ms.MakeSameNeighborhood(currentRotation_per_ms);
              radian angleDiff_per_ms_per_s = 2.0f * acos(clamp(currentToDesiredRotationDot, -1.0f, 1.0f)) / ((float)timeDiff_ms * 0.001f);
              radian maxDiff_per_ms_per_s = 0.2f * pi;//0.15f
              // braking is easier than accelerating
              //if (identity.MakeSameNeighborhood(desiredRotation_per_ms) > identity.MakeSameNeighborhood(currentRotation_per_ms)) maxDiff_per_ms_per_s = 0.4f * pi;
              //if (nodeAnimation->nodeName.compare("body") == 0) maxDiff_per_ms_per_s = 0.1f * pi;
              if (angleDiff_per_ms_per_s > maxDiff_per_ms_per_s) {
                // now make desiredRotation_per_ms into limited rotation * currentRotation_per_ms
                float allowFraction = maxDiff_per_ms_per_s / angleDiff_per_ms_per_s;
                Quaternion desiredRotationRotation = currentRotation_per_ms.GetRotationTo(desiredRotation_per_ms).GetNormalized();
                //if (desiredRotation.elements[3] < 0) desiredRotation.elements[3] = -desiredRotation.elements[3];
                resultingRotation_per_ms = desiredRotationRotation.GetRotationMultipliedBy(allowFraction) * currentRotation_per_ms;
                resultingRotation_per_ms.Normalize();
              }

              //Quaternion resultingRotation_per_ms = desiredRotation_per_ms.GetLerped(movementInfluence, currentRotation_per_ms);
              Quaternion resultingOrientation = resultingRotation_per_ms.GetRotationMultipliedBy(timeDiff_ms) * currentOrientation;

              orientation = orientation.GetSlerped(beginBias, resultingOrientation).GetNormalized();
              //orientation = resultingOrientation;

              // add some extra overall smoothness
              //orientation = orientation.GetSlerped(pow(1.0f - clamp(timeDiff_ms / 30.0f, 0.0f, 1.0f), 0.5f) * (0.6f + 0.2f * beginBias), currentOrientation).GetNormalized();
              //orientation = (identity.GetRotationTo(orientation).GetNormalized().GetRotationMultipliedBy(0.1f) * orientation).GetNormalized();
              //orientation = orientation.GetSlerped(pow(1.0f - clamp(timeDiff_ms / 30.0f, 0.0f, 1.0f), 0.5f) * (0.2f + 0.1f * beginBias), currentOrientation).GetNormalized();
              //orientation = orientation.GetSlerped(0.3f + beginBias * 0.2f, currentOrientation).GetNormalized(); // incorrect! incorporate timeDiff_ms into this somehow, whilst still allowing for less influence than the above version

            }

            else { //if (simpleMethod == true) {

              orientation = orientation.GetSlerped(currentBias, currentOrientation).GetNormalized();

              // maximum rotational velocity
              float dot = orientation.MakeSameNeighborhood(currentOrientation);
              radian angle_per_second = 2.0f * acos(clamp(dot, -1.0f, 1.0f)) / ((float)timeDiff_ms * 0.001f);
              radian maxAngle_per_second = 7.5f * pi;//5.5f * pi;
              //if (nodeAnimation->nodeName.compare("body") == 0) maxAngle_per_second *= 0.8f;
              if (nodeAnimation->nodeName.compare("left_elbow") == 0) maxAngle_per_second *= 1.2f;
              else if (nodeAnimation->nodeName.compare("right_elbow") == 0) maxAngle_per_second *= 1.2f;
              else if (nodeAnimation->nodeName.compare("left_knee") == 0) maxAngle_per_second *= 1.2f;
              else if (nodeAnimation->nodeName.compare("right_knee") == 0) maxAngle_per_second *= 1.2f;
              else if (nodeAnimation->nodeName.compare("left_ankle") == 0) maxAngle_per_second *= 1.6f;
              else if (nodeAnimation->nodeName.compare("right_ankle") == 0) maxAngle_per_second *= 1.6f;
              maxAngle_per_second = (0.3f + 0.7f * (1.0f - beginBias)) * maxAngle_per_second;
              //if (nodeAnimation->nodeName.compare("body") == 0) maxAngle_per_second = 3.0f * pi;
              if (angle_per_second > maxAngle_per_second) {
                float allowFraction = maxAngle_per_second / angle_per_second;
                Quaternion desiredRotation = currentOrientation.GetRotationTo(orientation).GetNormalized();
                orientation = (desiredRotation.GetRotationMultipliedBy(allowFraction) * currentOrientation).GetNormalized();
              }

            }

          } // timeDiff_ms > 0

          /*
            // enforce max change

            float dot = clamp(orientation.GetDotProduct(oldOrientation), 0.0f, 1.0f);
            //radian diff = pi - acos(change) * 2.0f;
            radian diff = acos(dot);
            radian maxRadPerSec = 3.8f * pi;
            if ((nodeAnimation->nodeName.compare("body") == 0 || nodeAnimation->nodeName.compare("middle") == 0)) maxRadPerSec = 3.2f * pi;
            maxRadPerSec *= 1.0f - beginBias * smoothFactor;
            radian clampedDiff = clamp(diff, 0.0f, (timeDiff_ms * 0.001f) * maxRadPerSec);
            float slerpBias = 1.0f - NormalizedClamp(clampedDiff + 0.00001f, 0.0f, diff + 0.00001f); // convert from clampedDiff to slerp bias. 0 == diff, 1 == 0
            //slerpBias = 0.94f;
            //slerpBias = slerpBias * 0.3f + 0.7f;
            //slerpBias = 0.0f;
            orientation = orientation.GetSlerped(slerpBias, oldOrientation);
            //if (GetVariable("type").compare("ballcontrol") == 0 && nodeAnimation->nodeName.compare("body") == 0) printf("%i ms, %f %f %f %f\n", timeDiff_ms, dot, diff, clampedDiff, slerpBias);

            // tmp super low tek hax
            //if (dot < 0.95f) orientation = orientation.GetSlerped(0.3f, oldOrientation);


            // some global smoothing to top things off

            float smoothness = 0.1f * beginBias + 0.1f;

            //float result = clamp(slerpBias * (1.0f + smoothness), 0.0f, 0.99f);
            //orientation = orientation.GetSlerped(result, oldOrientation);

            orientation = orientation.GetSlerped(smoothness, oldOrientation);
          */

          movementHistoryEntry->orientation = currentOrientation;
          movementHistoryEntry->timeDiff_ms = timeDiff_ms;

        }

        else if (nodeAnimation->nodeName.compare("player") == 0) {

          const Vector3 &previousPosition = movementHistoryEntry->position;
          Vector3 currentPosition = nodeMap.find(nodeAnimation->nodeName)->second->GetPosition();

          if (timeDiff_ms > 0 && beginBias > 0.01f) {

            Vector3 currentMovement = (currentPosition - previousPosition) / (movementHistoryEntry->timeDiff_ms * 0.001f);
  /*
            // damping (maybe useful: http://www.freebasic.net/forum/viewtopic.php?t=9769 )
            Vector3 desiredMovement = ((basePos + position) - currentPosition) / (timeDiff_ms * 0.001f);
            Vector3 dampingMovement = (desiredMovement - currentMovement);
            currentMovement += dampingMovement.GetNormalizedMax(timeDiff_ms * 0.02f);

            Vector3 extrapolatedPosition = currentPosition + currentMovement * (timeDiff_ms * 0.001f);

            // now we want to go from current to new position, but we can only differ so much from the movement from previous to current position
            float movementInfluence = 1.0f - clamp(timeDiff_ms / 50.0f, 0.0f, 1.0f);
            movementInfluence = pow(movementInfluence, 0.5f); // influence of old movement wears off in exponential fashion (source: laws of nature)
            float beginBias = pow(1.0f - NormalizedClamp(frame, 0, 15), 0.5f);
            movementInfluence *= 0.4f + beginBias * 0.6f;

            //if (desiredMovement.GetLength() < currentMovement.GetLength()) movementInfluence = 0.0f;
            //if (currentMovement.GetLength() > desiredMovement.GetLength()) currentMovement = currentMovement.GetNormalized() * desiredMovement.GetLength();
            //if (currentMovement.GetLength() > desiredMovement.GetLength()) currentMovement = currentMovement.GetNormalized() * (currentMovement.GetLength() * movementInfluence + desiredMovement.GetLength() * (1.0f - movementInfluence));

            Vector3 newPosition = (extrapolatedPosition) * movementInfluence + // current-movement-implied position
                                  (basePos + position) * (1.0f - movementInfluence); // desired position
  */

            // smooth
            position.coords[2] = position.coords[2] * (1.0f - currentBias) + currentPosition.coords[2] * currentBias;


  // old version, use for now, until new version above is finished

            float maxMetersPerSec = 2.8f;//1.8f
            if (GetVariable("outgoing_special_state").compare("") != 0) maxMetersPerSec = 6.0f;
            float allowedDistance = maxMetersPerSec * ((float)timeDiff_ms * 0.001f);

            //float oldPosition = nodeMap.find(nodeAnimation->nodeName)->second->GetPosition().coords[2];
            float newZ = position.coords[2] + basePos.coords[2];
            float desiredDistance = fabs(newZ - currentPosition.coords[2]);

            float bias = 1.0f;
            if (desiredDistance > allowedDistance) bias = allowedDistance / desiredDistance;

            newZ = (newZ * bias + currentPosition.coords[2] * (1.0f - bias)) * beginBias +
                   newZ * (1.0f - beginBias); // dual biasses ^_^ works like this: currentPosition influence only effective when beginbias > 0

  // /old version


            //nodeMap.find(nodeAnimation->nodeName)->second->SetPosition(newPosition, false);
            // only height
            position = position.Get2D() + Vector3(0, 0, newZ);

            //nodeMap.find(nodeAnimation->nodeName)->second->SetPosition((position + basePos).Get2D() + Vector3(0, 0, newZ), false);

          }

          movementHistoryEntry->position = currentPosition;
          movementHistoryEntry->timeDiff_ms = timeDiff_ms;

        }

      } // smoothing


      if (nodeAnimation->nodeName.compare("player") != 0) {
        Quaternion currentOrientation = nodeMap.find(nodeAnimation->nodeName)->second->GetRotation();
        orientation.MakeSameNeighborhood(currentOrientation);
        nodeMap.find(nodeAnimation->nodeName)->second->SetRotation(orientation, false);
      } else if (nodeAnimation->nodeName.compare("player") == 0) {
        nodeMap.find(nodeAnimation->nodeName)->second->SetPosition(position + basePos, false);
      }

    }

    if (updateSpatial) (*nodeMap.find(nodeAnimations.at(0)->nodeName)).second->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }

  void Animation::Shift(int fromFrame, int offset) { // todo: offset does not yet work
    if (offset == 1) {
      bool somethingShifted = false;
      int animSize = nodeAnimations.size();
      for (int i = 0; i < animSize; i++) {
        NodeAnimation *nodeAnimation = nodeAnimations.at(i);
        std::map<int, KeyFrame>::iterator animIter = nodeAnimation->animation.begin();
        std::map<int, KeyFrame> newAnimation;
        while (animIter != nodeAnimation->animation.end()) {

          //printf("%i, ", animIter->first);
          KeyFrame keyFrame = animIter->second;
          int frameNum = animIter->first;
          if (animIter->first >= fromFrame) {
            frameNum++; // shift
            somethingShifted = true;
          }
          newAnimation.insert(std::pair<int, KeyFrame>(frameNum, keyFrame));
          animIter++;
        }
        nodeAnimation->animation = newAnimation;
      }

      if (somethingShifted) {
        frameCount++;
        DirtyCache();
      }
    }

    if (offset == -1) {
      bool somethingShifted = false;
      int animSize = nodeAnimations.size();
      for (int i = 0; i < animSize; i++) {
        NodeAnimation *nodeAnimation = nodeAnimations.at(i);
        std::map<int, KeyFrame>::iterator animIter = nodeAnimation->animation.begin();
        std::map<int, KeyFrame> newAnimation;
        while (animIter != nodeAnimation->animation.end()) {

          //printf("%i, ", animIter->first);
          KeyFrame keyFrame = animIter->second;
          int frameNum = animIter->first;
          if (animIter->first != fromFrame) {
            if (animIter->first > fromFrame) {
              frameNum--; // shift
              somethingShifted = true;
            }
            newAnimation.insert(std::pair<int, KeyFrame>(frameNum, keyFrame));
          }
          animIter++;
        }
        nodeAnimation->animation = newAnimation;
      }

      if (somethingShifted) {
        frameCount--;
        DirtyCache();
      }
    }

    std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter = extensions.begin();
    while (extensionIter != extensions.end()) {
      extensionIter->second->Shift(fromFrame, offset);
      extensionIter++;
    }
  }

  Vector3 Animation::GetTranslation() const {
    if (cache_translation_dirty) {
      cache_translation = ((--nodeAnimations.at(0)->animation.end())->second.position -
                           nodeAnimations.at(0)->animation.begin()->second.position);
      cache_translation.coords[2] = 0;
      cache_translation_dirty = false;
    }
    return cache_translation;
  }

  Vector3 Animation::GetIncomingMovement() const {
    // todo: check if correct with idle incoming velo + weird incoming body rot
    if (cache_incomingMovement_dirty) {
      if (nodeAnimations.at(0)->animation.size() > 1) {
        cache_incomingMovement = ((++nodeAnimations.at(0)->animation.begin())->second.position -
                                  nodeAnimations.at(0)->animation.begin()->second.position) /
                                 ((++nodeAnimations.at(0)->animation.begin())->first -
                                  nodeAnimations.at(0)->animation.begin()->first * 1.0) * 100;
        cache_incomingMovement.coords[2] = 0;
      } else {
        cache_incomingMovement = Vector3(0);
      }
      cache_incomingMovement_dirty = false;
    }
    return cache_incomingMovement;
  }

  float Animation::GetIncomingVelocity() const {
    if (cache_incomingVelocity_dirty) {
      if (nodeAnimations.at(0)->animation.size() > 1) {
        Vector3 result = ((++nodeAnimations.at(0)->animation.begin())->second.position -
                          nodeAnimations.at(0)->animation.begin()->second.position) /
                         ((++nodeAnimations.at(0)->animation.begin())->first -
                          nodeAnimations.at(0)->animation.begin()->first * 1.0) * 100;
        result.coords[2] = 0;
        cache_incomingVelocity = result.GetLength();
        if (cache_incomingVelocity < 1.8) cache_incomingVelocity = 0;
        else if (cache_incomingVelocity >= 1.8 && cache_incomingVelocity < 4.2) cache_incomingVelocity = 3.5;
        else if (cache_incomingVelocity >= 4.2 && cache_incomingVelocity < 6.0) cache_incomingVelocity = 5.0;
        else if (cache_incomingVelocity >= 6.0) cache_incomingVelocity = 7.0;
      } else {
        cache_incomingVelocity = 0;
      }
      cache_incomingVelocity_dirty = false;
    }
    return cache_incomingVelocity;
  }

  Vector3 Animation::GetOutgoingMovement() const {
    if (cache_outgoingMovement_dirty) {

      if (nodeAnimations.at(0)->animation.size() > 1) {
        cache_outgoingMovement = ((--nodeAnimations.at(0)->animation.end())->second.position -
                                  (--(--nodeAnimations.at(0)->animation.end()))->second.position) /
                                 ((--nodeAnimations.at(0)->animation.end())->first -
                                  (--(--nodeAnimations.at(0)->animation.end()))->first * 1.0) * 100;
        cache_outgoingMovement.coords[2] = 0;
      } else {
        cache_outgoingMovement = Vector3(0);
      }

      cache_outgoingMovement_dirty = false;
    }
    return cache_outgoingMovement;
  }

  Vector3 Animation::GetRangedOutgoingMovement() const {
    if (cache_rangedOutgoingMovement_dirty || cache_outgoingVelocity_dirty || cache_angle_dirty) {

      if (nodeAnimations.at(0)->animation.size() > 1) {
        cache_rangedOutgoingMovement = Vector3(0, -GetOutgoingVelocity(), 0).GetRotated2D(GetOutgoingAngle());
      } else {
        cache_rangedOutgoingMovement = Vector3(0);
      }

      cache_rangedOutgoingMovement_dirty = false;
    }
    return cache_rangedOutgoingMovement;
  }

  Vector3 Animation::GetOutgoingDirection() const {
    if (cache_outgoingDirection_dirty || cache_angle_dirty) {
      cache_outgoingDirection = Vector3(0, -1, 0).GetRotated2D(GetOutgoingAngle());
      cache_outgoingDirection_dirty = false;
    }
    return cache_outgoingDirection;
  }

  Vector3 Animation::GetIncomingBodyDirection() const {
    if (cache_incomingBodyDirection_dirty || cache_incomingBodyAngle_dirty) {
      cache_incomingBodyDirection = Vector3(0, -1, 0).GetRotated2D(GetIncomingBodyAngle());
      cache_incomingBodyDirection_dirty = false;
    }
    return cache_incomingBodyDirection;
  }

  Vector3 Animation::GetOutgoingBodyDirection() const {
    if (cache_outgoingBodyDirection_dirty || cache_outgoingBodyAngle_dirty) {
      cache_outgoingBodyDirection = Vector3(0, -1, 0).GetRotated2D(GetOutgoingBodyAngle());
      cache_outgoingBodyDirection_dirty = false;
    }
    return cache_outgoingBodyDirection;
  }

  float Animation::GetOutgoingVelocity() const {
    if (cache_outgoingVelocity_dirty) {
      if (nodeAnimations.at(0)->animation.size() > 1) {
        Vector3 result = ((--nodeAnimations.at(0)->animation.end())->second.position -
                          (--(--nodeAnimations.at(0)->animation.end()))->second.position) /
                         ((--nodeAnimations.at(0)->animation.end())->first -
                          (--(--nodeAnimations.at(0)->animation.end()))->first * 1.0) * 100;
        result.coords[2] = 0;
        cache_outgoingVelocity = result.GetLength();
        if (cache_outgoingVelocity < 1.8) cache_outgoingVelocity = 0.0;
        else if (cache_outgoingVelocity >= 1.8 && cache_outgoingVelocity < 4.2) cache_outgoingVelocity = 3.5;
        else if (cache_outgoingVelocity >= 4.2 && cache_outgoingVelocity < 6.0) cache_outgoingVelocity = 5.0;
        else if (cache_outgoingVelocity >= 6.0) cache_outgoingVelocity = 7.0;
      } else {
        cache_outgoingVelocity = 0;
      }
      cache_outgoingVelocity_dirty = false;
    }
    return cache_outgoingVelocity;
  }

  radian Animation::GetOutgoingAngle() const {
    if (cache_angle_dirty || cache_outgoingVelocity_dirty) {
      if (GetOutgoingVelocity() >= 1.8) {
        //if (nodeAnimations.at(0)->animation.size() > 1) {

        // full player rotation - last move
        Vector3 lastMoveVector = (--nodeAnimations.at(0)->animation.end())->second.position -
                                 (--(--nodeAnimations.at(0)->animation.end()))->second.position;
        cache_angle = FixAngle(lastMoveVector.GetAngle2D(), true);

        // if angle is close to 180 degrees, we can't be sure if we want 180 or -180 deg. use body angle as hint.
        if (cache_angle < -0.95f * pi || cache_angle > 0.95f * pi) {
          radian x, y, z;
          (--(nodeAnimations.at(1)->animation.end()))->second.orientation.GetAngles(x, y, z);
          if (signSide(cache_angle) != signSide(z)) {
            cache_angle = pi * 0.99f * signSide(z);
          } else {
            cache_angle = clamp(cache_angle, -0.99f * pi, 0.99f * pi); // also do this if the side is already correct: we want to be a little away from pi, else vectors based on this don't have a clear sidedness
          }
        }

      } else {

        if (nodeAnimations.at(1)->animation.size() > 0) {
          // body rotation
          radian x, y, z;
          (--(nodeAnimations.at(1)->animation.end()))->second.orientation.GetAngles(x, y, z);
          cache_angle = z;

          // lying on the ground? (buggy)
          // float threshold = pi * 0.35;
          // if (fabs(x) > threshold || fabs(y) > threshold) {
          //   Vector3 quatDirection; quatDirection = (--(nodeAnimations.at(1)->animation.end()))->second.orientation;
          //   //quatDirection = -quatDirection;
          //   cache_angle = FixAngle(quatDirection.Get2D().GetNormalized().GetAngle2D());
          // }

          cache_angle = ModulateIntoRange(-pi, pi, cache_angle);

        } else {
          cache_angle = 0;
        }

      }

      cache_angle_dirty = false;
    }
    return cache_angle;
  }

  radian Animation::GetIncomingBodyAngle() const {
    if (cache_incomingBodyAngle_dirty) {
      if (nodeAnimations.at(1)->animation.size() > 0) {

        // body rotation
        radian x, y, z;
        (nodeAnimations.at(1)->animation.begin())->second.orientation.GetAngles(x, y, z);
        cache_incomingBodyAngle = z;

        // lying on the ground? (buggy)
        // float threshold = pi * 0.35;
        // if (fabs(x) > threshold || fabs(y) > threshold) {
        //   Vector3 quatDirection; quatDirection = (nodeAnimations.at(1)->animation.begin())->second.orientation;
        //   //quatDirection = -quatDirection;
        //   cache_incomingBodyAngle = FixAngle(quatDirection.Get2D().GetNormalized().GetAngle2D());
        // }

        cache_incomingBodyAngle = ModulateIntoRange(-pi, pi, cache_incomingBodyAngle);
        //printf("angle: %f\n", cache_incomingBodyAngle);

      } else {
        cache_incomingBodyAngle = 0;
      }

      cache_incomingBodyAngle_dirty = false;
    }
    return cache_incomingBodyAngle;
  }

  radian Animation::GetOutgoingBodyAngle() const {
    if (cache_outgoingBodyAngle_dirty || cache_angle_dirty || cache_outgoingVelocity_dirty) {
      if (GetOutgoingVelocity() >= 1.8) {
        if (nodeAnimations.at(1)->animation.size() > 0) {

          // body rotation
          radian x, y, z;
          (--(nodeAnimations.at(1)->animation.end()))->second.orientation.GetAngles(x, y, z);
          cache_outgoingBodyAngle = z;

/* impossible while moving, right?
          // lying on the ground? (buggy)
          float threshold = pi * 0.35;
          if (fabs(x) > threshold || fabs(y) > threshold) {
            Vector3 quatDirection; quatDirection = (--(nodeAnimations.at(1)->animation.end()))->second.orientation;
            //quatDirection = -quatDirection;
            cache_outgoingBodyAngle = FixAngle(quatDirection.Get2D().GetNormalized().GetAngle2D());
          }
*/
          cache_outgoingBodyAngle -= GetOutgoingAngle();

          cache_outgoingBodyAngle = ModulateIntoRange(-pi, pi, cache_outgoingBodyAngle);

        } else {
          cache_outgoingBodyAngle = 0;
        }

      } else { // new
        cache_outgoingBodyAngle = 0; // new
      } // new

      cache_outgoingBodyAngle_dirty = false;
    }
    return cache_outgoingBodyAngle;
  }

  e_Foot Animation::GetOutgoingFoot() const {
    std::string foot = GetVariable("steps");
    e_Foot curFoot = GetCurrentFoot();
    int steps = 1;
    if (foot != "") {
      steps = atoi(foot.c_str());
    }
    if (is_odd(steps)) {
      if (curFoot == e_Foot_Left) {
        return e_Foot_Right;
      } else {
        return e_Foot_Left;
      }
    } else {
      if (curFoot == e_Foot_Right) {
        return e_Foot_Right;
      } else {
        return e_Foot_Left;
      }
    }

    // default value to please the compiler
    return e_Foot_Right;
  }


  void Animation::Reset() {
    int animSize = nodeAnimations.size();
    for (int i = 0; i < animSize; i++) {
      delete nodeAnimations.at(i);
    }

    customData.reset();

    std::map<const char*, std::string>::const_iterator iter = variableCache.begin();
    while (iter != variableCache.end()) {
      delete [] (*iter).first;
      iter++;
    }
    variableCache.clear();
    nodeAnimations.clear();
    extensions.clear();
    frameCount = 0;

    DirtyCache();
  }

  void Animation::LoadData(std::vector < std::vector<std::string> > &file) {

    for (unsigned int line = 0; line < file.size(); line++) {

      unsigned int key = 1;
      while (key < file.at(line).size()) {
        int frame = int(round(atoi(file.at(line).at(key).c_str()) * 1.0));
        Quaternion orientation(QUATERNION_IDENTITY);

        if (line != 0) { // limbs, only rotations
          orientation.elements[0] = atof(file.at(line).at(key + 1).c_str());
          orientation.elements[1] = atof(file.at(line).at(key + 2).c_str());
          orientation.elements[2] = atof(file.at(line).at(key + 3).c_str());
          orientation.elements[3] = atof(file.at(line).at(key + 4).c_str());

          key += 5;
        }

        Vector3 position(0);

        if (line == 0) { // player, only position
          position.coords[0] = atof(file.at(line).at(key + 1).c_str());
          position.coords[1] = atof(file.at(line).at(key + 2).c_str());
          position.coords[2] = atof(file.at(line).at(key + 3).c_str());

          key += 4;
        }

        SetKeyFrame(file.at(line).at(0), frame, orientation, position);
      }
    }
  }

  void Animation::Load(const std::string &filename) {
    name = filename;

    std::vector<std::string> file;
    file_to_vector(filename, file);

    std::vector < std::vector<std::string> > tokenizedFile;
    int lastLine = 0;
    for (unsigned int i = 0; i < file.size(); i++) {
      std::vector<std::string> tokenizedLine;
      tokenize(file.at(i), tokenizedLine, ",");
      if (tokenizedLine.at(0) == "extension" || tokenizedLine.at(0).substr(0, 1) == "<") break;
      tokenizedFile.push_back(tokenizedLine);
      lastLine = i + 1;
    }

    LoadData(tokenizedFile);

    // cache extension data
    std::vector < std::vector <std::string> > tokenizedLines;

    for (unsigned int i = lastLine; i < file.size(); i++) {
      std::vector<std::string> tokenizedLine;
      tokenize(file.at(i), tokenizedLine, ",");
      std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter;
      if (tokenizedLine.at(0) == "extension") {
        tokenizedLines.push_back(tokenizedLine);
      } else {
        break;
      }
      lastLine = i + 1;
    }

    // additional xml data
    std::string xmlData;
    for (unsigned int i = lastLine; i < file.size(); i++) {
      xmlData.append(file.at(i));
    }
    XMLLoader xmlLoader;
    customData = boost::shared_ptr<XMLTree>(new XMLTree(xmlLoader.Load(xmlData)));

    // load extension data
    for (unsigned int i = 0; i < tokenizedLines.size(); i++) {
      std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter;
      extensionIter = extensions.find(tokenizedLines.at(i).at(1));
      if (extensionIter != extensions.end()) (*extensionIter).second->Load(tokenizedLines.at(i));
    }

    std::multimap<std::string, XMLTree>::iterator iter = customData->children.find("bumpdirection");
    if (iter != customData->children.end()) {
      const XMLTree &tree = iter->second;
      Vector3 bumpDirection = GetVectorFromString(tree.value);
      if (bumpDirection.GetLength() > 0) bumpDirection.Normalize();
      iter->second.value = real_to_str(bumpDirection.coords[0]) + "," + real_to_str(bumpDirection.coords[1]) + "," + real_to_str(bumpDirection.coords[2]);
    }

    iter = customData->children.find("balldirection");
    if (iter != customData->children.end()) {
      const XMLTree &tree = iter->second;
      Vector3 ballDirection = GetVectorFromString(tree.value);
      if (ballDirection.GetLength() > 0) ballDirection.Normalize();
      iter->second.value = real_to_str(ballDirection.coords[0]) + "," + real_to_str(ballDirection.coords[1]) + "," + real_to_str(ballDirection.coords[2]);
    }

    iter = customData->children.find("incomingballdirection");
    if (iter != customData->children.end()) {
      const XMLTree &tree = iter->second;
      Vector3 incomingBallDirection = GetVectorFromString(tree.value);
      if (incomingBallDirection.GetLength() > 0) incomingBallDirection.Normalize();
      iter->second.value = real_to_str(incomingBallDirection.coords[0]) + "," + real_to_str(incomingBallDirection.coords[1]) + "," + real_to_str(incomingBallDirection.coords[2]);
    }

    // create variable cache
    iter = customData->children.begin();
    while (iter != customData->children.end()) {
      char *varName = new char[256];
      strcpy(varName, (*iter).first.c_str());
      std::string varData = (*iter).second.value;
      //printf("varname: %s, vardata: %s\n", varName, varData.c_str());

      variableCache.insert(std::pair<const char*, std::string>(varName, varData));

      iter++;
    }

    cache_AnimType = variableCache.find("type")->second;

    ConvertToStartFacingForwardIfIdle();

  }

  void Animation::Save(const std::string &filename) {
    std::vector<std::string> fileData;
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line;
      line = nodeAnimations.at(i)->nodeName + ",";
      std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
      while (animIter != nodeAnimations.at(i)->animation.end()) {
        line.append(int_to_str(animIter->first) + ","); // frame number
        if (nodeAnimations.at(i)->nodeName != "player") {
          line.append(real_to_str(animIter->second.orientation.elements[0]) + ","); // X element
          line.append(real_to_str(animIter->second.orientation.elements[1]) + ","); // Y element
          line.append(real_to_str(animIter->second.orientation.elements[2]) + ","); // Z element
          line.append(real_to_str(animIter->second.orientation.elements[3]) + ","); // W element
        } else {
          line.append(real_to_str(animIter->second.position.coords[0]) + ","); // X pos
          line.append(real_to_str(animIter->second.position.coords[1]) + ","); // Y pos
          line.append(real_to_str(animIter->second.position.coords[2]) + ","); // Z pos
        }
        animIter++;
      }
      line = line.substr(0, line.length() - 1);
      fileData.push_back(line);
    }

    FILE *file;
    file = fopen(filename.c_str(), "w");
    if (!file) {
      Log(e_Error, "Animation", "Save", "Could not open file " + filename + " for writing");
      return;
    }
    for (int i = 0; i < (signed int)fileData.size(); i++) {
      fprintf(file, "%s\n", fileData.at(i).c_str());
    }

    std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter = extensions.begin();
    while (extensionIter != extensions.end()) {
      (*extensionIter).second->Save(file);
      extensionIter++;
    }

    XMLLoader loader;
    fprintf(file, "%s", loader.GetSource((*customData)).c_str());

    fclose(file);
  }

  void Animation::Mirror() {
    name.append("_mirror");
    (currentFoot == e_Foot_Right) ? currentFoot = e_Foot_Left : currentFoot = e_Foot_Right;

    for (unsigned int i = 0; i < nodeAnimations.size(); i++) {

      if (nodeAnimations.at(i)->nodeName.substr(0, 4) == "left") {
        // find counterpart
        std::string needle = nodeAnimations.at(i)->nodeName;
        needle = needle.replace(0, 4, "right");

        for (unsigned int j = 0; j < nodeAnimations.size(); j++) {
          if (nodeAnimations.at(j)->nodeName.compare(needle) == 0) {
            // swap
            std::map<int, KeyFrame> tmp = nodeAnimations.at(i)->animation;
            nodeAnimations.at(i)->animation = nodeAnimations.at(j)->animation;
            nodeAnimations.at(j)->animation = tmp;
            break;
          }
        }

      }

    }

    for (unsigned int i = 0; i < nodeAnimations.size(); i++) {

      std::map<int, KeyFrame>::iterator keyIter = nodeAnimations.at(i)->animation.begin();
      while (keyIter != nodeAnimations.at(i)->animation.end()) {
        if (i == 0) {
          keyIter->second.position.coords[0] = -keyIter->second.position.coords[0];
        } else {
          keyIter->second.orientation.elements[1] = -keyIter->second.orientation.elements[1];
          keyIter->second.orientation.elements[2] = -keyIter->second.orientation.elements[2];
        }
        keyIter++;
      }

    }

    // extensions!
    std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter = extensions.begin();
    while (extensionIter != extensions.end()) {
      extensionIter->second->Mirror();
      extensionIter++;
    }

    // variables
    std::map<const char*, std::string>::iterator varIter = variableCache.begin();
    while (varIter != variableCache.end()) {
      std::string &varData = varIter->second;
      if (varData.substr(0, 4) == "left") {
        varData = varData.replace(0, 4, "right");
      } else if (varData.substr(0, 5) == "right") {
        varData = varData.replace(0, 5, "left");
      }
      varIter++;
    }

    Vector3 newBallDirVec = GetVariable("balldirection") != "" ? GetVectorFromString(GetVariable("balldirection")) * Vector3(-1, 1, 1) : 0;
    Vector3 newIncomingBallDirVec = GetVariable("incomingballdirection") != "" ? GetVectorFromString(GetVariable("incomingballdirection")) * Vector3(-1, 1, 1) : 0;
    Vector3 newBumpDirVec = GetVariable("bumpdirection") != "" ? GetVectorFromString(GetVariable("bumpdirection")) * Vector3(-1, 1, 1) : 0;
    SetVariable("balldirection", GetStringFromVector(newBallDirVec));
    SetVariable("incomingballdirection", GetStringFromVector(newIncomingBallDirVec));
    SetVariable("bumpdirection", GetStringFromVector(newBumpDirVec));

    DirtyCache();
  }

  std::string Animation::GetName() const {
    return name;
  }

  void Animation::AddExtension(const std::string &name, boost::shared_ptr<AnimationExtension> extension) {
    extensions.insert(std::pair < std::string, boost::shared_ptr<AnimationExtension> >(name, extension));
  }

  boost::shared_ptr<AnimationExtension> Animation::GetExtension(const std::string &name) {
    return extensions.find(name)->second;
  }

  const std::string &Animation::GetVariable(const char *name) const {

    std::map<const char*, std::string>::const_iterator iter = variableCache.find(name);
    //printf("looking for %s.. %s\n", name, variableCache.begin()->first);
    if (iter != variableCache.end()) {
      //printf("found %s\n", iter->second.c_str());
      return iter->second;
    } else {
      //printf("found nothing\n");
      return emptyString;
    }
  }

  void Animation::SetVariable(const std::string &name, const std::string &value) {
    std::multimap<std::string, XMLTree>::iterator iter = customData->children.find(name);
    if (iter != customData->children.end()) {
      XMLTree &tree = iter->second;
      tree.value = value;
    }

    // flat list for speed, i guess, i should start documenting stuff earlier
    std::map<const char*, std::string>::iterator iter2 = variableCache.find(name.c_str());
    if (iter2 != variableCache.end()) {
      iter2->second = value;
    } else {
      char *varName = new char[256];
      strcpy(varName, name.c_str());
      variableCache.insert(std::pair<const char*, std::string>(varName, value));
    }
  }

  boost::shared_ptr<XMLTree> Animation::GetCustomData() {
    return customData;
  }

  void Animation::Hax() {
/*
    // outgoing ball angle == outgoing player angle
    Vector3 position;
    int frame;
    bool touch = boost::static_pointer_cast<FootballAnimationExtension>(GetExtension("football"))->GetFirstTouch(position, frame);
    if (touch) {
      Vector3 angles, position;
      float power;
      GetExtension("football")->GetKeyFrame(frame, angles, position, power);
      //angles.coords[2] = ModulateIntoRange(-pi, pi, GetAngle());
      power = 1;//GetOutgoingVelocity() * 0.2;

      if (GetVariable("type") == "shortpass") {
        power = 2;//GetOutgoingVelocity() * 0.4;
      }


      //position = Vector3(0.0, -0.1 - 0.012 * GetOutgoingMovement().GetLength() * frame, 0);


      //if (position.GetAngle2D() + 0.5 * pi < 3.0)
      //position.Rotate2D(-(position.GetAngle2D() + 0.5 * pi));
      //position.Print();
      //position = Vector3(0, -0.5, 0);
      GetExtension("football")->SetKeyFrame(frame, angles, position, power);
      //printf("%f\n", power);
    }
*/

/*
      XMLTree tree;
      tree.value = "movement";
      customData->children.insert(std::pair<std::string, XMLTree>("type", tree));
*/


/*
    for (int i = 0; i < nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->node->GetName();
      printf("%s\n", line.c_str());
      if (line == "player") nodeAnimations.at(i)->node->SetName("body");
      if (line == "player node") nodeAnimations.at(i)->node->SetName("player");
    }
    printf("\n\n");
*/

/*
  nodeAnimations.at(0)->node->SetName("player");
  nodeAnimations.at(1)->node->SetName("body");
*/

// put player rotation into body
/*
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(0)->animation.begin();
        while (animIter != nodeAnimations.at(0)->animation.end()) {
          Vector3 angles = animIter->second.angles;
          animIter->second.angles = Vector3(0);
          SetKeyFrame(nodeAnimations.at(1)->node, animIter->first, angles, Vector3(0));
          animIter++;
        }
*/

// exaggerate middle forward bending
/*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line == "middle") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          animIter->second.angles.coords[0] += (pi * 2) / 360.0 * 2;
          animIter++;
        }
      }
    }
*/

    // exaggerate body lending
    /*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line == "body") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa *= 1.4;
          rotYa *= 1.4;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
    }
    */

// bend knees/hips more - lower player stance
/*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line == "left_thigh" || line == "right_thigh") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa -= 0.04;
          rotXa *= 1.1;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
      if (line == "left_knee" || line == "right_knee") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa += 0.08;
          rotXa *= 1.2;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
      if (line == "player") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {

          animIter->second.position.coords[2] -= 0.01;

          animIter++;
        }
      }
    }
*/

// exaggerate everything

    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line != "body") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa *= 1.1;
          rotYa *= 1.1;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
    }


// bend body/middle more
/*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
*/
/*
      if (line == "body") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa *= 1.4;
          rotYa *= 1.4;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
      */
/*
      if (line == "middle") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa -= 0.2;
          //rotYa -= 0.5;
          rotXa *= 1.2;
          //rotYa *= 1.4;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
    }
*/


// bend arms more
/*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line == "left_shoulder" || line == "right_shoulder") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa += 0.25f;
          //rotXa *= 1.4;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
*/
      /*
      if (line == "left_elbow" || line == "right_elbow") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          radian rotXa, rotYa, rotZa;
          animIter->second.orientation.GetAngles(rotXa, rotYa, rotZa);

          rotXa += 0.28;
          rotXa *= 1.4;

          Quaternion rotX, rotY, rotZ, quat;
          rotX.SetAngleAxis(rotXa, Vector3(1, 0, 0));
          rotY.SetAngleAxis(rotYa, Vector3(0, 1, 0));
          rotZ.SetAngleAxis(rotZa, Vector3(0, 0, 1));
          quat = rotX * rotY * rotZ;

          animIter->second.orientation = quat;

          animIter++;
        }
      }
    */
//    }

// faster movement
/*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line == "player") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {

          animIter->second.position.coords[0] *= 1.428;
          animIter->second.position.coords[1] *= 1.428;

          animIter++;
        }
      }
    }
*/

// heighten body node 0.74
/*
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(1)->animation.begin();
        while (animIter != nodeAnimations.at(1)->animation.end()) {
          Vector3 angles = animIter->second.angles;
          animIter->second.position += Vector3(0, 0, 0.74);
          animIter++;
        }
*/

// exaggerate body angle in bends
/*
    radian angle = GetAngle();
    if (angle > 0.24 * pi) angle = 0.24 * pi;
    float vel = (GetIncomingMovement().GetLength() + GetOutgoingMovement().GetLength() * 2) / 3.0;
    angle *= vel;

        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(1)->animation.begin();
        while (animIter != nodeAnimations.at(1)->animation.end()) {
          animIter->second.angles.coords[1] += angle * 3;
          animIter++;
        }
*/

// arms higher
    /*
    for (int i = 0; i < (signed int)nodeAnimations.size(); i++) {
      std::string line = nodeAnimations.at(i)->nodeName;
      if (line == "left_shoulder" || line == "right_shoulder") {
        std::map<int, KeyFrame>::iterator animIter = nodeAnimations.at(i)->animation.begin();
        while (animIter != nodeAnimations.at(i)->animation.end()) {
          float offset;
          if (line == "left_shoulder") offset = -0.05 * pi; else offset = 0.05 * pi;
          //animIter->second.angles.coords[1] += offset;
          animIter->second.angles.coords[1] += offset;
          animIter++;
        }
      }
    }
    */

  }

}
