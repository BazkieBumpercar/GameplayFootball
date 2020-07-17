// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_ANIMATOR
#define _HPP_ANIMATOR

#include "types/iusertask.hpp"

#include "scene/scene2d/scene2d.hpp"
#include "scene/scene3d/scene3d.hpp"

#include "scene/objects/camera.hpp"

#include "framework/scheduler.hpp"

#include "gui/guiinterface.hpp"
#include "utils/animation.hpp"
#include "utils/animationextensions/footballanimationextension.hpp"

using namespace blunted;

const float idleVelocity = 0.0;
const float dribbleVelocity = 3.5;
const float walkVelocity = 5.0;
const float sprintVelocity = 7.0;

class Animator : public IUserTask {

  public:
    Animator(boost::shared_ptr<Scene2D> scene2D, boost::shared_ptr<Scene3D> scene3D, GuiInterface *guiInterface);
    virtual ~Animator();

    virtual std::string GetName() const { return "animator"; }

    void FillNodeMap(boost::intrusive_ptr<Node> targetNode, std::map < const std::string, boost::intrusive_ptr<Node> > &nodeMap);
    void AddNodeToTimeline(boost::intrusive_ptr<Node> node, GuiTimeline *timeline);
    void PopulateTimeline(Animation *animation, GuiTimeline *timeline);

    virtual void GetPhase();
    virtual void ProcessPhase();
    virtual void PutPhase();

  protected:
    boost::shared_ptr<Scene2D> scene2D;
    boost::shared_ptr<Scene3D> scene3D;
    GuiInterface *guiInterface;

    boost::shared_ptr<TaskSequence> guiSequence;

    boost::intrusive_ptr<Node> objectNode;

    boost::intrusive_ptr<Camera> camera;
    boost::intrusive_ptr<Node> cameraNode;
    boost::intrusive_ptr<Node> playerNode;
    boost::intrusive_ptr<Node> ballNode;

    boost::intrusive_ptr<Geometry> greenPilon;
    boost::intrusive_ptr<Geometry> bluePilon;

    GuiTimeline *timeline;
    GuiCaption *timelineIndex;
    GuiCaption *debugValues1;
    GuiCaption *debugValues2;
    GuiCaption *debugValues3;

    std::map < std::string, boost::intrusive_ptr<Node> > managedNodes;

    Animation *animation;
    std::map < const std::string, boost::intrusive_ptr<Node> > nodeMap;

    GuiFileDialog *saveDialog;
    GuiFileDialog *loadDialog;

    std::string currentFile;
    std::string currentDir;

    radian studioRot;
    bool play;
    int currentPlayFrame;

    int counter;

};

#endif
