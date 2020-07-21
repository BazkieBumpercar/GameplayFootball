// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENUSCENE
#define _HPP_MENUSCENE

#include "scene/scene3d/node.hpp"
#include "scene/objects/camera.hpp"
#include "scene/objects/light.hpp"
#include "scene/objects/geometry.hpp"

#include "managers/environmentmanager.hpp"

using namespace blunted;

struct MenuSceneLocation {
  MenuSceneLocation() {
    position = Vector3(0.0f, 0.0f, 1.0f);
    orientation = Quaternion(QUATERNION_IDENTITY);
    timeStamp_ms = EnvironmentManager::GetInstance().GetTime_ms();
  }
  Vector3 position;
  Quaternion orientation;
  unsigned long timeStamp_ms;
};

class MenuScene {

  public:
    MenuScene();
    virtual ~MenuScene();

    void Get();
    void Process();
    void Put();

    void RandomizeTargetLocation();
    void SetTargetLocation(const Vector3 &position, radian angle);
    void SetTargetLocation(const Vector3 &position, const Quaternion &orientation);

  protected:
    boost::intrusive_ptr<Node> containerNode;
    boost::intrusive_ptr<Camera> camera;
    boost::intrusive_ptr<Light> mainLight;
    boost::intrusive_ptr<Geometry> geom;

    boost::intrusive_ptr<Light> hoverLights[3];
    Vector3 hoverLightPosition;

    boost::shared_ptr<Scene3D> scene3D;

    MenuSceneLocation sourceLocation;
    MenuSceneLocation targetLocation;

    Vector3 currentPosition;
    Quaternion currentOrientation;

    bool seamless;

};

#endif
