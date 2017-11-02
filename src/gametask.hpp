// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALL_GAMETASK
#define _HPP_FOOTBALL_GAMETASK

#include "types/iusertask.hpp"
#include "types/thread.hpp"

#include "onthepitch/match.hpp"
#include "menu/menuscene.hpp"

#include "hid/keyboard.hpp"
#include "hid/gamepad.hpp"

#include "menu/menutask.hpp"

using namespace blunted;


class UpdateFullbodyModel : public Command {

  public:
    UpdateFullbodyModel(std::vector<PlayerBase*> playersToProcess) : Command("UpdateFullBodyModel"), playersToProcess(playersToProcess) {};
    virtual ~UpdateFullbodyModel() {};

  protected:
    void Update() {
      for (unsigned int i = 0; i < playersToProcess.size(); i++) {
        playersToProcess.at(i)->UpdateFullbodyModel();
      }
    }

    virtual bool Execute(void *caller = NULL) {
      Update();
      return true;
    }

    std::vector<PlayerBase*> playersToProcess;

};

class UploadFullbodyModel : public Command {

  public:
    UploadFullbodyModel(std::vector < boost::intrusive_ptr<Geometry> > geometryToUpload) : Command("UploadFullBodyModel"), geometryToUpload(geometryToUpload) {};
    virtual ~UploadFullbodyModel() {};

  protected:
    void Update();

    virtual bool Execute(void *caller = NULL) {
      Update();
      return true;
    }

    std::vector < boost::intrusive_ptr<Geometry> > geometryToUpload; // use shared pointers to geometry objects instead, to assure lifetime

};

enum e_GameTaskMessage {
  e_GameTaskMessage_StartMatch,
  e_GameTaskMessage_StopMatch,
  e_GameTaskMessage_StartMenuScene,
  e_GameTaskMessage_StopMenuScene,
  e_GameTaskMessage_None
};

class GameTask : public IUserTask {

  public:
    GameTask();
    virtual ~GameTask();

    void Exit();

    void Action(e_GameTaskMessage message);

    virtual void GetPhase();
    virtual void ProcessPhase();
    virtual void PutPhase();

    Match *GetMatch() { return match; }
    MenuScene *GetMenuScene() { return menuScene; }

    MessageQueue<e_GameTaskMessage> messageQueue;

    virtual std::string GetName() const { return "game"; }

    boost::mutex matchLifetimeMutex;

  protected:
    Match *match;
    MenuScene *menuScene;

    boost::timed_mutex matchPutBufferMutex;
    boost::mutex menuSceneLifetimeMutex;

    boost::shared_ptr<Scene3D> scene3D;

};

#endif
