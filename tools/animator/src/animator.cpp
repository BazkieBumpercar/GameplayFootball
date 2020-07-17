// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "animator.hpp"

#include "managers/usereventmanager.hpp"
#include "managers/resourcemanagerpool.hpp"
#include "scene/objects/geometry.hpp"
#include "scene/objectfactory.hpp"
#include "utils/objectloader.hpp"
#include "gui/gui.hpp"

#include "blunted.hpp"

Animator::Animator(boost::shared_ptr<Scene2D> scene2D, boost::shared_ptr<Scene3D> scene3D, GuiInterface *guiInterface) : scene2D(scene2D), scene3D(scene3D), guiInterface(guiInterface) {
  camera = static_pointer_cast<Camera>(ObjectFactory::GetInstance().CreateObject("camera", e_ObjectType_Camera));
  scene3D->CreateSystemObjects(camera);

  ObjectLoader loader;
  objectNode = boost::intrusive_ptr<Node>(new Node("the world!"));
  objectNode->AddNode(loader.LoadObject(scene3D, "media/objects/studio/studio.object"));

  // debug pilon
  boost::intrusive_ptr < Resource<GeometryData> > geometry = ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch("media/objects/helpers/green.ase", true);
  greenPilon = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("greenPilon", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(greenPilon);
  greenPilon->SetGeometryData(geometry);
  greenPilon->SetLocalMode(e_LocalMode_Absolute);
  scene3D->AddObject(greenPilon);

  geometry = ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch("media/objects/helpers/blue.ase", true);
  bluePilon = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("bluePilon", e_ObjectType_Geometry));
  scene3D->CreateSystemObjects(bluePilon);
  bluePilon->SetGeometryData(geometry);
  bluePilon->SetLocalMode(e_LocalMode_Absolute);
  scene3D->AddObject(bluePilon);

  camera->Init();
  Quaternion rot;
  rot.SetAngleAxis(0.42 * pi, Vector3(1, 0, 0));
  camera->SetRotation(rot);
  camera->SetPosition(Vector3(0, -4, 1));
  camera->SetFOV(44);
  camera->SetCapping(0.2, 120.0);
  cameraNode = boost::intrusive_ptr<Node>(new Node("camera"));
  cameraNode->AddObject(camera);

  objectNode->AddNode(cameraNode);

  playerNode = loader.LoadObject(scene3D, "../../../football/media/objects/players/player.object");
  playerNode->SetName("player");
  playerNode->SetPosition(Vector3(0, 0, 0));
  objectNode->AddNode(playerNode);
  FillNodeMap(playerNode, nodeMap);

  ballNode = loader.LoadObject(scene3D, "../../../football/media/objects/balls/generic.object");
  ballNode->SetName("ball");
  objectNode->AddNode(ballNode);

  currentDir = "../../../football/media/animations";

  //rot.SetAngleAxis(-0.3 * pi, Vector3(0, 0, 1));
  //objectNode->SetRotation(rot);

  scene3D->AddNode(objectNode);


  // interface

  /*
  GuiButton *button = new GuiButton(scene2D, "testButton", 0, 0, 20, 5, "test!");
  guiInterface->AddView(button);

  button = new GuiButton(scene2D, "testButton2", 0, 5, 20, 10, "ook een test..");
  guiInterface->AddView(button);

  button = new GuiButton(scene2D, "testButton3", 0, 10, 20, 15, "jawohl dat ist");
  guiInterface->AddView(button);
  */

  GuiButton *button = new GuiButton(scene2D, "button_load", 0, 0, 20, 5, "Load animation");
  guiInterface->AddView(button);

  button = new GuiButton(scene2D, "button_save", 0, 5, 20, 10, "Save animation");
  guiInterface->AddView(button);

  timeline = new GuiTimeline(scene2D, "motionTimeline", 0, 75, 100, 100);
  guiInterface->AddView(timeline);

  //playerNode->PrintTree();

  animation = new Animation;

  timeline->AddPart("player", "player");
  managedNodes.insert(std::pair < std::string, boost::intrusive_ptr<Node> >("player", playerNode));
  Vector3 position = playerNode->GetPosition(); // initial position
  animation->SetKeyFrame("player", 0, Quaternion(QUATERNION_IDENTITY), position);

  AddNodeToTimeline(playerNode, timeline);

  boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
  animation->AddExtension("football", extension);
  timeline->AddPart("ball", "ball");
  //animation->GetExtension("football")->SetKeyFrame(0, Vector3(0), Vector3(0), 1.0);

  timelineIndex = new GuiCaption(scene2D, "caption_timelineIndex", 90, 0, 100, 10, "0/0");
  guiInterface->AddView(timelineIndex);
  debugValues1 = new GuiCaption(scene2D, "caption_debugValues1", 80, 10, 100, 15, "");
  guiInterface->AddView(debugValues1);
  debugValues2 = new GuiCaption(scene2D, "caption_debugValues2", 80, 15, 100, 20, "");
  guiInterface->AddView(debugValues2);
  debugValues3 = new GuiCaption(scene2D, "caption_debugValues3", 80, 20, 100, 25, "");
  guiInterface->AddView(debugValues3);

  studioRot = 0;

  play = false;
  currentPlayFrame = 0;

  PopulateTimeline(animation, timeline);

  currentFile = "untitled.anim";

  counter = 0;
}

Animator::~Animator() {
  delete animation;
  scene3D->DeleteObject(greenPilon);
  scene3D->DeleteObject(bluePilon);
  scene3D->DeleteNode(objectNode);
  camera.reset();
  objectNode.reset();
  playerNode.reset();
  scene2D.reset();
  scene3D.reset();
}


Vector3 GetFrontOfFootOffset(float velocity) {
  Vector3 ffo = Vector3(0, -0.2, 0); // basic ffo
  ffo += Vector3(0, -velocity / 35.0, 0);
  return ffo;
}

void Animator::FillNodeMap(boost::intrusive_ptr<Node> targetNode, std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap) {
  nodeMap.insert(std::pair < std::string, boost::intrusive_ptr<Node> >(targetNode->GetName(), targetNode));

  std::vector < boost::intrusive_ptr<Node> > gatherNodes;
  targetNode->GetNodes(gatherNodes);
  for (int i = 0; i < (signed int)gatherNodes.size(); i++) {
    FillNodeMap(gatherNodes.at(i), nodeMap);
  }
}

void Animator::AddNodeToTimeline(boost::intrusive_ptr<Node> node, GuiTimeline *timeline) {
  std::vector < boost::intrusive_ptr<Node> > nodes;
  node->GetNodes(nodes);
  for (int i = 0; i < (signed int)nodes.size(); i++) {
    timeline->AddPart(nodes.at(i)->GetName(), nodes.at(i)->GetName());
    managedNodes.insert(std::pair < std::string, boost::intrusive_ptr<Node> >(nodes.at(i)->GetName(), nodes.at(i)));
    Vector3 position = nodes.at(i)->GetPosition(); // initial position
    Vector3 angles;
    // initial angles
    if (nodes.at(i)->GetName() == "left_shoulder") angles.Set(0.1, -0.25, -0.15);
    if (nodes.at(i)->GetName() == "right_shoulder") angles.Set(0.1, 0.25, 0.15);
    if (nodes.at(i)->GetName() == "left_elbow") angles.Set(-0.2, 0.0, 0.0);
    if (nodes.at(i)->GetName() == "right_elbow") angles.Set(-0.2, -0.0, 0.0);
    if (nodes.at(i)->GetName() == "left_thigh") angles.Set(-0.06, -0.04, 0);
    if (nodes.at(i)->GetName() == "right_thigh") angles.Set(-0.06, 0.04, 0);
    if (nodes.at(i)->GetName() == "left_knee") angles.Set(0.18, 0, 0);
    if (nodes.at(i)->GetName() == "right_knee") angles.Set(0.18, 0, 0);
    if (nodes.at(i)->GetName() == "left_ankle") angles.Set(-0.12, 0, 0);
    if (nodes.at(i)->GetName() == "right_ankle") angles.Set(-0.12, 0, 0);

    Quaternion rotX, rotY, rotZ, quat;
    rotX.SetAngleAxis(angles.coords[0], Vector3(1, 0, 0));
    rotY.SetAngleAxis(angles.coords[1], Vector3(0, 1, 0));
    rotZ.SetAngleAxis(angles.coords[2], Vector3(0, 0, 1));
    quat = rotX * rotY * rotZ;
    animation->SetKeyFrame(nodes.at(i)->GetName(), 0, quat, position);
    AddNodeToTimeline(nodes.at(i), timeline);
  }
}

void Animator::PopulateTimeline(Animation *animation, GuiTimeline *timeline) {

  std::vector<NodeAnimation*> &nodeAnimations = animation->GetNodeAnimations();

  // iterate nodes
  int animSize = nodeAnimations.size();
  for (int i = 0; i < animSize; i++) {
    NodeAnimation *nodeAnimation = nodeAnimations.at(i);
    std::map<int, KeyFrame>::iterator animIter = nodeAnimation->animation.begin();
    while (animIter != nodeAnimation->animation.end()) {
      timeline->EnableKeyFrame(nodeAnimations.at(i)->nodeName, animIter->first);
      animIter++;
    }
  }
  int i = animSize;

  std::map < std::string, boost::shared_ptr<AnimationExtension> >::iterator extensionIter = animation->GetExtensions().begin();
  while (extensionIter != animation->GetExtensions().end()) {

    //extensionIter->second->PopulateTimeline(i, timeline);
    //void FootballAnimationExtension::PopulateTimeline(int part, GuiTimeline *timeline) {
    std::map<int, FootballKeyFrame>::iterator animIter = boost::static_pointer_cast<FootballAnimationExtension>(extensionIter->second)->GetAnimation().begin();
    while (animIter != boost::static_pointer_cast<FootballAnimationExtension>(extensionIter->second)->GetAnimation().end()) {
      timeline->EnableKeyFrame(i, animIter->first);
      animIter++;
    }

    extensionIter++;
    i++;
  }
  timeline->Redraw();
}

//!
// adds touches around main touch
// debug: adds touches around main touch
void AddExtraTouches(Animation* animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap) {

  Vector3 animBallPos;
  int animTouchFrame;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);
  if (isTouch) {

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

    float heightCheat = 1.0;
    if (animBallPos.coords[2] > 0.8) heightCheat = 1.6;

    int range_pre = int(round(2.f * heightCheat));
    int range_post = int(round(4.f * heightCheat));
    if (animation->GetVariable("type") == "trap" || animation->GetVariable("type") == "interfere") {
      range_pre = int(round(4.f * heightCheat));
      range_post = int(round(4.f * heightCheat));
    } else if (animation->GetVariable("type") == "deflect") {
      range_pre = int(round(4.f * heightCheat));
      range_post = int(round(6.f * heightCheat));
    } else if (animation->GetVariable("type") == "sliding") {
      range_pre = int(round(6.f * heightCheat));
      range_post = int(round(6.f * heightCheat));
    } else if (animation->GetVariable("type") == "ballcontrol") {
      range_pre = int(round(2.f * heightCheat));
      range_post = int(round(6.f * heightCheat));
    }
    //range_pre *= 0.6;

    int frameOffset = 4; // correct for animation smoothing: player limbs always seem to be late at ballposition otherwise
    if (animTouchFrame + frameOffset - range_pre <= animation->GetFrameCount()) {
      boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->DeleteKeyFrame(animTouchFrame);
    }

    for (int i = animTouchFrame - range_pre; i < animTouchFrame + range_post + 1; i += 2) {
      if (i >= 0 && /*i != animTouchFrame &&*/ i < animation->GetFrameCount() - frameOffset - 1) {

        // set animation to this frame
        animation->Apply(nodeMap, i, 0, false);
        // find new ball position, based on the closest body part's position in this frame
        Vector3 position = closestBodyPart->GetDerivedPosition() + toBallVector;
        Vector3 origBodyPos = nodeMap.find("player")->second->GetDerivedPosition();
        animation->Apply(nodeMap, i + frameOffset, 0, false);
        Vector3 futureBodyPos = nodeMap.find("player")->second->GetDerivedPosition();
        //origBodyPos.Print();
        Vector3 diff2D = (futureBodyPos - origBodyPos).Get2D();
        Quaternion orientation;
        //Vector3 position = animBallPos + (animBallPos.Get2D() * (-animTouchFrame + i) * 0.05);
        boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->SetKeyFrame(i + frameOffset, orientation, position + diff2D, 0);
      }
    }
  }
}



void Animator::GetPhase() {
}

void Animator::ProcessPhase() {

  std::string partName;
  int currentFrame;
  bool isKeyFrame;
  timeline->GetLocation(partName, currentFrame, isKeyFrame);

  //test->SetPosition(Vector3(1, -0.01, 0.47));

  bool newMessage = true;
  while (newMessage) {
    GuiSignal signal = guiInterface->signalQueue.GetMessage(newMessage);
    if (newMessage) {
      if (signal.sender->GetName() == "button_save" && signal.key == SDLK_RETURN) {
        saveDialog = new GuiFileDialog(scene2D, "dialog_save", 30, 10, 70, 90, currentDir, currentFile.substr(currentFile.find_last_of("/") + 1, std::string::npos));
        guiInterface->AddView(saveDialog);
        guiInterface->SetFocussedView(saveDialog);
      }
      else if (signal.sender->GetName() == "dialog_save_CancelButton" && signal.key == SDLK_RETURN) {
        guiInterface->DeleteView(saveDialog);
        saveDialog = 0;
      }
      else if (signal.sender->GetName() == "dialog_save_OkayButton" && signal.key == SDLK_RETURN) {
        currentFile = saveDialog->GetFilename();
        currentDir = saveDialog->GetDirectory();
        //printf("loading %s\n", currentFile.c_str());
        animation->Save(currentFile);

        guiInterface->DeleteView(saveDialog);
        saveDialog = 0;
      }
      else if (signal.sender->GetName() == "button_load" && signal.key == SDLK_RETURN) {
        loadDialog = new GuiFileDialog(scene2D, "dialog_load", 30, 10, 70, 90, currentDir, currentFile.substr(currentFile.find_last_of("/") + 1, std::string::npos));
        guiInterface->AddView(loadDialog);
        guiInterface->SetFocussedView(loadDialog);
      }
      else if (signal.sender->GetName() == "dialog_load_CancelButton" && signal.key == SDLK_RETURN) {
        guiInterface->DeleteView(loadDialog);
        loadDialog = 0;
      }
      else if (signal.sender->GetName() == "dialog_load_OkayButton" && signal.key == SDLK_RETURN) {
        currentFile = loadDialog->GetFilename();
        currentDir = loadDialog->GetDirectory();
        //printf("loading %s\n", currentFile.c_str());
        animation->Reset();
        delete animation;
        animation = new Animation;
        boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
        animation->AddExtension("football", extension);
        animation->Load(currentFile);
        //animation->ConvertToStartFacingForwardIfIdle();
        //animation->ConvertAngles();
        //animation->Invert();

/*
        // debug
        std::list < boost::intrusive_ptr<Object> > bodyParts;
        playerNode->GetObjects(e_ObjectType_Geometry, bodyParts, true);
        std::map < const std::string, boost::intrusive_ptr<Node> > nodeMap;
        FillNodeMap(playerNode, nodeMap);
        AddExtraTouches(animation, playerNode, bodyParts, nodeMap); //!
*/

        timeline->ClearKeys();
        PopulateTimeline(animation, timeline);
        //animation->Hax();

        printf("angle: %f\n", animation->GetOutgoingAngle());
        printf("body angle: %f\n", animation->GetOutgoingBodyAngle());

        float velocity = animation->GetIncomingMovement().GetLength();
        std::string mode;
        if (velocity < 1.8) mode = "idle";
        else if (velocity >= 1.8 && velocity < 4.2) mode = "dribble";
        else if (velocity >= 4.2 && velocity < 6.0) mode = "walk";
        else if (velocity >= 6.0) mode = "sprint";
        printf("%s - ", mode.c_str());

        velocity = animation->GetOutgoingMovement().GetLength();
        if (velocity < 1.8) mode = "idle";
        else if (velocity >= 1.8 && velocity < 4.2) mode = "dribble";
        else if (velocity >= 4.2 && velocity < 6.0) mode = "walk";
        else if (velocity >= 6.0) mode = "sprint";
        printf("%s\n", mode.c_str());

        guiInterface->DeleteView(loadDialog);
        loadDialog = 0;
      }
      else if (signal.sender->GetName() == "motionTimeline") {

        if (signal.key == SDLK_INSERT) {
          // insert new frame (move all keyframes from this frame on to the 'right')
          printf("shifting..\n");
          animation->Shift(currentFrame, +1);
          timeline->ClearKeys();
          PopulateTimeline(animation, timeline);
        }

        if (signal.key == SDLK_BACKSPACE) {
          // insert new frame (move all keyframes from this frame on to the 'left', erasing current keyframes)
          printf("shifting..\n");
          animation->Shift(currentFrame, -1);
          timeline->ClearKeys();
          PopulateTimeline(animation, timeline);
        }


        // is this a normal node?
        if (managedNodes.find(partName) != managedNodes.end()) {

          if (signal.key == SDLK_DELETE && isKeyFrame && currentFrame != 0) {
            static_cast<GuiTimeline*>(signal.sender)->ToggleKeyFrame();
            animation->DeleteKeyFrame(partName, currentFrame);
          }

          if (signal.key == SDLK_F4) {
            animation->Hax();
          }

          if (signal.key == SDLK_q || signal.key == SDLK_w || signal.key == SDLK_a || signal.key == SDLK_s || signal.key == SDLK_z || signal.key == SDLK_x ||
              signal.key == SDLK_e || signal.key == SDLK_d || signal.key == SDLK_c || signal.key == SDLK_i || signal.key == SDLK_k || signal.key == SDLK_j ||
              signal.key == SDLK_l || signal.key == SDLK_u || signal.key == SDLK_o || signal.key == SDLK_0) {

            Quaternion orientation;
            Vector3 rotationAngles, position;
            int adaptedCurrentFrame = currentFrame;
            if (adaptedCurrentFrame > animation->GetFrameCount() - 1) adaptedCurrentFrame = animation->GetFrameCount() - 1;
            animation->GetKeyFrame(partName, adaptedCurrentFrame, orientation, position);
            orientation.GetAngles(rotationAngles.coords[0], rotationAngles.coords[1], rotationAngles.coords[2]);
            if (!isKeyFrame) {
              static_cast<GuiTimeline*>(signal.sender)->ToggleKeyFrame();
              //position = managedNodes.find(partName)->second->GetPosition();
            }
            float rotStep = pi * 0.01;
            if (signal.key == SDLK_q) rotationAngles.coords[0] -= rotStep;
            if (signal.key == SDLK_e) rotationAngles.coords[0] += rotStep;
            if (signal.key == SDLK_w) rotationAngles.coords[0] = 0;
            if (signal.key == SDLK_a) rotationAngles.coords[1] -= rotStep;
            if (signal.key == SDLK_d) rotationAngles.coords[1] += rotStep;
            if (signal.key == SDLK_s) rotationAngles.coords[1] = 0;
            if (signal.key == SDLK_z) rotationAngles.coords[2] -= rotStep;
            if (signal.key == SDLK_c) rotationAngles.coords[2] += rotStep;
            if (signal.key == SDLK_x) rotationAngles.coords[2] = 0;
            if (signal.key == SDLK_j) position.coords[0] += 0.02;
            if (signal.key == SDLK_l) position.coords[0] -= 0.02;
            if (signal.key == SDLK_i) position.coords[1] -= 0.02;
            if (signal.key == SDLK_k) position.coords[1] += 0.02;
            if (signal.key == SDLK_u) position.coords[2] -= 0.01;
            if (signal.key == SDLK_o) position.coords[2] += 0.01;
            if (signal.key == SDLK_0) position.Set(0, 0, 0);
            Quaternion rotX, rotY, rotZ, quat;
            rotX.SetAngleAxis(rotationAngles.coords[0], Vector3(1, 0, 0));
            rotY.SetAngleAxis(rotationAngles.coords[1], Vector3(0, 1, 0));
            rotZ.SetAngleAxis(rotationAngles.coords[2], Vector3(0, 0, 1));
            quat = rotX * rotY * rotZ;
            animation->SetKeyFrame(partName, currentFrame, quat, position);

            float velocity = animation->GetIncomingMovement().GetLength();
            std::string mode;
            if (velocity < 1.8) mode = "idle";
            else if (velocity >= 1.8 && velocity < 4.2) mode = "dribble";
            else if (velocity >= 4.2 && velocity < 6.0) mode = "walk";
            else if (velocity >= 6.0) mode = "sprint";
            float diff = 0;
            if (mode == "idle") diff = velocity - idleVelocity;
            if (mode == "dribble") diff = velocity - dribbleVelocity;
            if (mode == "walk") diff = velocity - walkVelocity;
            if (mode == "sprint") diff = velocity - sprintVelocity;
            printf("%s (%f) - ", mode.c_str(), diff);

            velocity = animation->GetOutgoingMovement().GetLength();
            if (velocity < 1.8) mode = "idle";
            else if (velocity >= 1.8 && velocity < 4.2) mode = "dribble";
            else if (velocity >= 4.2 && velocity < 6.0) mode = "walk";
            else if (velocity >= 6.0) mode = "sprint";
            if (mode == "idle") diff = velocity - idleVelocity;
            if (mode == "dribble") diff = velocity - dribbleVelocity;
            if (mode == "walk") diff = velocity - walkVelocity;
            if (mode == "sprint") diff = velocity - sprintVelocity;
            printf("%s (%f)\n", mode.c_str(), diff);
            //animation->GetOutgoingDirection().Print();
          }

        // then what is it? a football extension maybe?
        } else {

          if (signal.key == SDLK_DELETE && isKeyFrame) {
            static_cast<GuiTimeline*>(signal.sender)->ToggleKeyFrame();
            animation->GetExtension("football")->DeleteKeyFrame(currentFrame);
          }

          if (signal.key == SDLK_i || signal.key == SDLK_k || signal.key == SDLK_j ||
              signal.key == SDLK_l || signal.key == SDLK_u || signal.key == SDLK_o || signal.key == SDLK_0) {

            Quaternion tmp;
            Vector3 position;
            float power;
            animation->GetExtension("football")->GetKeyFrame(currentFrame, tmp, position, power);
            if (!isKeyFrame) {
              static_cast<GuiTimeline*>(signal.sender)->ToggleKeyFrame();
              //position = managedNodes.find(partName)->second->GetPosition();
              position.coords[2] = 0.11;
            }
            float rotStep = pi * 0.01;
            if (signal.key == SDLK_j) position.coords[0] += 0.01;
            if (signal.key == SDLK_l) position.coords[0] -= 0.01;
            if (signal.key == SDLK_i) position.coords[1] -= 0.01;
            if (signal.key == SDLK_k) position.coords[1] += 0.01;
            if (signal.key == SDLK_u) position.coords[2] -= 0.01;
            if (signal.key == SDLK_o) position.coords[2] += 0.01;
            if (signal.key == SDLK_0) position.Set(0, 0, 0.11);
            animation->GetExtension("football")->SetKeyFrame(currentFrame, tmp, position, power);
          }

        }

        if (signal.key == SDLK_SPACE) { if (!play) play = true; else play = false; }
      }
    }

  }

  Quaternion orientation;
  Vector3 position;
  float power;

  if (managedNodes.find(partName) != managedNodes.end()) {
    animation->GetKeyFrame(partName, currentFrame, orientation, position);
    if (partName != "player") position = managedNodes.find(partName)->second->GetDerivedPosition();
  } else {
    animation->GetExtension("football")->GetKeyFrame(currentFrame, orientation, position, power);
  }

  Vector3 rotationAngles;
  orientation.GetAngles(rotationAngles.coords[0], rotationAngles.coords[1], rotationAngles.coords[2]);

  debugValues1->Set("rot: " +
                            int_to_str(360 * rotationAngles.coords[0] / (pi * 2)) + ", " +
                            int_to_str(360 * rotationAngles.coords[1] / (pi * 2)) + ", " +
                            int_to_str(360 * rotationAngles.coords[2] / (pi * 2)));
  debugValues2->Set("pos: " +
                            real_to_str(position.coords[0]).substr(0, 5) + ", " +
                            real_to_str(position.coords[1]).substr(0, 5) + ", " +
                            real_to_str(position.coords[2]).substr(0, 5));
  debugValues3->Set("outgoing rot: " +
                            int_to_str(int(round(animation->GetOutgoingAngle() / (2 * pi) * 360))));

  Vector3 ballPosition;
  Quaternion tmp;
  if (!play) {
    int adaptedCurrentFrame = currentFrame;
    if (adaptedCurrentFrame > animation->GetFrameCount() - 1) adaptedCurrentFrame = animation->GetFrameCount() - 1;
    animation->Apply(nodeMap, adaptedCurrentFrame, 0, false, 1.0f, Vector(0), 0);

    timelineIndex->Set(int_to_str(currentFrame) + "/" + int_to_str(timeline->GetFrameCount()));

    animation->GetExtension("football")->GetKeyFrame(currentFrame, tmp, ballPosition, power);
    //counter = 0;
  } else {
    animation->Apply(nodeMap, currentPlayFrame, 0, false);
    /*if (is_odd(counter)) */currentPlayFrame++;
    counter++;
    int frameCount = timeline->GetFrameCount();
    if (currentPlayFrame >= frameCount - 1) currentPlayFrame = 0;

    timelineIndex->Set(int_to_str(currentPlayFrame) + "/" + int_to_str(timeline->GetFrameCount()));

    animation->GetExtension("football")->GetKeyFrame(currentPlayFrame, tmp, ballPosition, power);
  }

  float velocity = animation->GetIncomingVelocity();
  greenPilon->SetPosition(GetFrontOfFootOffset(velocity));

  ballNode->GetObject("genericball")->SetPosition(ballPosition);
  if (ballPosition != Vector3(0)) {
    Vector3 ballDir;
    if (animation->GetVariable("incomingballdirection") != "") ballDir = GetVectorFromString(animation->GetVariable("incomingballdirection")) * -1.0;
    if (ballDir.GetLength() == 0) if (animation->GetVariable("balldirection") != "") ballDir = GetVectorFromString(animation->GetVariable("balldirection"));
    if (ballDir.GetLength() != 0) bluePilon->SetPosition(ballPosition + ballDir);
  }

  studioRot += UserEventManager::GetInstance().GetMouseRelativePos().coords[0] / 100.0;
  Quaternion rot;
  rot.SetAngleAxis(studioRot, Vector3(0, 0, 1));
  cameraNode->SetRotation(rot);
  cameraNode->SetPosition(playerNode->GetPosition().Get2D() + Vector3(0, 0, 0.65));
  //camera->SetPosition(Vector3(0, -3.4, 1.8 - 0.74) + playerNode->GetPosition());
}

void Animator::PutPhase() {
}
