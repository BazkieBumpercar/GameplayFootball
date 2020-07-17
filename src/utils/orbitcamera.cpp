// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "orbitcamera.hpp"

#include "base/math/bluntmath.hpp"
#include "managers/environmentmanager.hpp"
#include "managers/usereventmanager.hpp"
#include "managers/systemmanager.hpp"
#include "managers/resourcemanagerpool.hpp"

#include "scene/objectfactory.hpp"

#include "blunted.hpp"

namespace blunted {

  OrbitCamera::OrbitCamera(boost::shared_ptr<Scene3D> scene3D, const Vector3 &position) : time(0) {

    boost::intrusive_ptr<Camera> tmpOrbitCamera(new Camera("supervette cam"));
    orbitCamera = tmpOrbitCamera;
    scene3D->CreateSystemObjects(orbitCamera);


  orbitCamera->SetProperty("x_percent", "0");
  orbitCamera->SetProperty("width_percent", "100");
  orbitCamera->SetProperty("y_percent", "8");
  orbitCamera->SetProperty("height_percent", "84");
  orbitCamera->Init();

  orbitCamera->SetFOV(42);

    boost::intrusive_ptr<Node> tmpNode(new Node("OrbitCamera control node"));
    orbitCameraNode = tmpNode;
    orbitCameraNode->AddObject(orbitCamera);
    orbitCameraNode->SetPosition(position);
    scene3D->AddNode(orbitCameraNode);

    mouseVert = pi * 1.8; // down
    mouseHoriz = 0;

    prevTime = EnvironmentManager::GetInstance().GetTime_ms() / 1000.0;
  }

  OrbitCamera::~OrbitCamera() {
    light.reset();
    light2.reset();
    orbitCamera.reset();
    orbitCameraNode.reset();
  }

  boost::intrusive_ptr<Node> OrbitCamera::GetCameraNode() {
    return orbitCameraNode;
  }

  void OrbitCamera::GetPhase() {
    orbitCameraPosition = orbitCameraNode->GetPosition();
  }

  void OrbitCamera::ProcessPhase() {

    time = EnvironmentManager::GetInstance().GetTime_ms() / 1000.0;
    float timeDiff = time - prevTime;
    prevTime = time;

    // OrbitCamera

    bool left = UserEventManager::GetInstance().GetKeyboardState(SDLK_a);
    bool right = UserEventManager::GetInstance().GetKeyboardState(SDLK_d);
    bool up = UserEventManager::GetInstance().GetKeyboardState(SDLK_w);
    bool down = UserEventManager::GetInstance().GetKeyboardState(SDLK_s);

    Vector3 mouse = UserEventManager::GetInstance().GetMouseRelativePos();

    Quaternion pitch;
    mouseVert += mouse.coords[1] * 0.004;
    if (mouseVert <= pi) mouseVert = pi;
    if (mouseVert >= pi * 2) mouseVert = pi * 2;
    pitch.SetAngleAxis(mouseVert, Vector3(-1, 0, 0));

    Quaternion heading;
    mouseHoriz += mouse.coords[0] * 0.004;
    heading.SetAngleAxis(mouseHoriz, Vector3(0, 0, -1));

    newRotation = heading * pitch;

    Vector3 walkVec(0);
    if (left) walkVec += Vector3(-1, 0, 0);
    if (right) walkVec += Vector3(1, 0, 0);
    if (up) walkVec += Vector3(0, 0, -1);
    if (down) walkVec += Vector3(0, 0, 1);
    if (walkVec.GetLength() != 0) {
      walkVec.Normalize();
      walkVec *= timeDiff * 50.0;
    }

    Vector3 directionVec;
    directionVec = newRotation * walkVec;

    newPosition = orbitCameraPosition + directionVec;
  }

  void OrbitCamera::PutPhase() {
    orbitCameraNode->SetRotation(newRotation);
    orbitCameraNode->SetPosition(newPosition);
  }

}
