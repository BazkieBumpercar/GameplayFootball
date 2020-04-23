// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MATCH
#define _HPP_MATCH

#include "framework/scheduler.hpp"

#include "team.hpp"
#include "ball.hpp"
#include "referee.hpp"
#include "officials.hpp"

#include "../data/matchdata.hpp"
#include "player/humanoid/animcollection.hpp"
#include "AIsupport/mentalimage.hpp"

#include "../menu/menutask.hpp"

#include "utils/gui2/widgets/caption.hpp"
#include "../menu/ingame/scoreboard.hpp"
#include "../menu/ingame/radar.hpp"
#include "../menu/ingame/tacticsdebug.hpp"

#include "scene/objects/camera.hpp"
#include "scene/objects/sound.hpp"
#include "scene/objects/light.hpp"

#include "types/messagequeue.hpp"
#include "types/command.hpp"
#include "types/lockable.hpp"

#include <boost/circular_buffer.hpp>

#include <fstream>
#include <iostream>

struct ReplaySpatialFrame {
  unsigned long frameTime_ms;
  Vector3 position;
  Quaternion orientation;
};

struct ReplayBallTouchesNetFrame {
  unsigned long frameTime_ms;
  bool ballTouchesNet;
};

struct ReplaySpatial {
  ReplaySpatial(int frameCount) {
    frames = boost::circular_buffer<ReplaySpatialFrame>(frameCount);
  }
  boost::intrusive_ptr<Spatial> spatial;
  boost::circular_buffer<ReplaySpatialFrame> frames;
};

struct PlayerBounce {
  Player *opp;
  float force;
};

struct ReplayState {
  ReplayState() {
    dirty = false;
  }
  bool dirty;
  unsigned long viewTime_ms;
  int cam;
  float modifierValue;
};

/*
struct MissingAnim {

  MissingAnim() {
    timesMissed = 1;
    angleDifference = 0.0f;
  }

  bool operator == (const MissingAnim &comp) const {
    if (this->outgoingVelocity == comp.outgoingVelocity &&
        this->outgoingDirection.GetDistance(comp.outgoingDirection) < 0.001f &&
        this->outgoingBodyDirection.GetDistance(comp.outgoingBodyDirection) <  0.001f) {
      return true;
    } else {
      return false;
    }
  }

  bool operator < (const MissingAnim &comp) const {
    if (this->timesMissed < comp.timesMissed) return true; else return false;
  }

  Vector3 outgoingDirection;
  e_Velocity outgoingVelocity;
  Vector3 outgoingBodyDirection;
  mutable int timesMissed;
  mutable radian angleDifference;
};
*/

class Match {

  public:
    Match(MatchData *matchData, const std::vector<IHIDevice*> &controllers);
    virtual ~Match();

    void Exit();

    void SetRandomSunParams();
    void RandomizeAdboards(boost::intrusive_ptr<Node> stadiumNode);
    void UpdateControllerSetup();
    void SpamMessage(const std::string &msg, int time_ms = 3000);
    int GetScore(int teamID) { return matchData->GetGoalCount(teamID); }
    Ball *GetBall() { return ball; }
    Team *GetTeam(int teamID) { return teams[teamID]; }
    Player *GetPlayer(int playerID);
    void GetAllTeamPlayers(int teamID, std::vector<Player*> &players);
    void GetActiveTeamPlayers(int teamID, std::vector<Player*> &players);
    void GetOfficialPlayers(std::vector<PlayerBase*> &players);

    boost::shared_ptr<AnimCollection> GetAnimCollection() { return anims; }

    const MentalImage *GetMentalImage(int history_ms);
    void UpdateLatestMentalImageBallPredictions();

    void ResetSituation(const Vector3 &focusPos);

    void Pause(bool doPause) { pause = doPause; }
    bool GetPause() { return pause; }
    void SetMatchPhase(e_MatchPhase newMatchPhase);
    e_MatchPhase GetMatchPhase() const { return matchPhase; }

    void StartPlay() { inPlay = true; }
    void StopPlay() { inPlay = false; }
    bool IsInPlay() const { return inPlay; }

    void StartSetPiece() { inSetPiece = true; }
    void StopSetPiece() { inSetPiece = false; }
    bool IsInSetPiece() const { return inSetPiece; }
    Referee *GetReferee() { return referee; }
    Officials *GetOfficials() { return officials; }
    const RefereeBuffer &GetRefereeBuffer() { return referee->GetBuffer(); };

    void SetGoalScored(bool onOff) { if (onOff == false) ballIsInGoal = false; goalScored = onOff; }
    bool IsGoalScored() const { return goalScored; }
    int GetLastGoalTeamID() const { return lastGoalTeamID; }
    void SetLastTouchTeamID(int id, e_TouchType touchType = e_TouchType_Intentional_Kicked) { lastTouchTeamIDs[touchType] = id; lastTouchTeamID = id; referee->BallTouched(); }
    int GetLastTouchTeamID(e_TouchType touchType) const { return lastTouchTeamIDs[touchType]; }
    int GetLastTouchTeamID() const { return lastTouchTeamID; }
    Team *GetLastTouchTeam(e_TouchType touchType) { if (lastTouchTeamIDs[touchType] != -1) return teams[lastTouchTeamIDs[touchType]]; else return 0; }
    Team *GetLastTouchTeam() { if (lastTouchTeamID != -1) return teams[lastTouchTeamID]; else return teams[0]; }
    Player *GetLastTouchPlayer(e_TouchType touchType) { if (GetLastTouchTeam(touchType)) return GetLastTouchTeam(touchType)->GetLastTouchPlayer(touchType); else return 0; }
    Player *GetLastTouchPlayer() { if (GetLastTouchTeam()) return GetLastTouchTeam()->GetLastTouchPlayer(); else return 0; }
    float GetLastTouchBias(int decay_ms, unsigned long time_ms = 0) { if (GetLastTouchTeam()) return GetLastTouchTeam()->GetLastTouchBias(decay_ms, time_ms); else return 0; }
    bool IsBallInGoal() const { return ballIsInGoal; }

    signed int GetBestPossessionTeamID();
    Player *GetDesignatedPossessionPlayer() { return designatedPossessionPlayer; }
    Player *GetBallRetainer() { return ballRetainer; }
    void SetBallRetainer(Player *retainer) { ballRetainer = retainer; }

    float GetAveragePossessionSide(int time_ms) const { return possessionSideHistory->GetAverage(time_ms); }

    unsigned long GetIterations() const { return iterations.GetData(); }
    unsigned long GetMatchTime_ms() const { return matchTime_ms; }
    unsigned long GetActualTime_ms() const { return actualTime_ms; }

    void GameOver();

    void GetCameraParams(float &zoom, float &height, float &fov, float &angleFactor);
    void SetCameraParams(float zoom, float height, float fov, float angleFactor);

    void UpdateIngameCamera();

    boost::intrusive_ptr<Camera> GetCamera() { return camera; }
    boost::shared_ptr<AnimCollection> GetAnims() { return anims; }

    void Get();
    void Process();
    void PreparePutBuffers();
    void FetchPutBuffers();
    void Put();

    boost::intrusive_ptr<Node> GetDynamicNode();

    void ApplyReplayFrame(unsigned long replayTime_ms);

    void FollowCamera(Quaternion &orientation, Quaternion &nodeOrientation, Vector3 &position, float &FOV, const Vector3 &targetPosition, float zoom);
    void SetReplayCamera(int camType, const Vector3 &target, float modifierValue);

    void SetAutoUpdateIngameCamera(bool autoUpdate = true) { if (autoUpdate != autoUpdateIngameCamera) { camPos.clear(); autoUpdateIngameCamera = autoUpdate; } }

    int GetReplaySize_ms();
    int GetReplayCamCount();

    void ProcessReplayMessages();
    Lockable<ReplayState> replayState;

    MatchData* GetMatchData() { return matchData; }

    float GetMatchDurationFactor() const { return matchDurationFactor; }
    float GetMatchDifficulty() const { return matchDifficulty; }

    std::vector<Vector3> &GetAnimPositionCache(Animation *anim) { return animPositionCache.find(anim)->second; }

    void UploadGoalNetting();

    unsigned long GetPreviousProcessTime_ms() { return previousProcessTime_ms; } // always around 10ms, not a very useful function, probably
    unsigned long GetPreviousPreparePutTime_ms() { return previousPreparePutTime_ms; }
    unsigned long GetPreviousPutTime_ms() { return previousPutTime_ms; }
    int GetTimeSincePreviousProcess_ms() { return timeSincePreviousProcess_ms; }
    int GetTimeSincePreviousPreparePut_ms() { return timeSincePreviousPreparePut_ms; }
    int GetTimeSincePreviousPut_ms() { return timeSincePreviousPut_ms; }

    //void AddMissingAnim(const MissingAnim &someAnim);

    // not sure about how signals work in this game at the moment. whole menu/game thing needs a rethink, i guess
    boost::signals2::signal<void(Match*)> sig_OnMatchPhaseChange;
    boost::signals2::signal<void(Match*)> sig_OnShortReplayMoment;
    boost::signals2::signal<void(Match*)> sig_OnExtendedReplayMoment;
    boost::signals2::signal<void(Match*)> sig_OnGameOver;
    boost::signals2::signal<void(Match*)> sig_OnCreatedMatch;
    boost::signals2::signal<void(Match*)> sig_OnExitedMatch;

  protected:
    void GetReplaySpatials(std::list < boost::intrusive_ptr<Spatial> > &spatials);
    void CaptureReplayFrame(unsigned long replayTime_ms);
    bool CheckForGoal(signed int side);

    void CalculateBestPossessionTeamID();
    void CheckHumanoidCollisions();
    void CheckHumanoidCollision(Player *p1, Player *p2, std::vector<PlayerBounce> &p1Bounce, std::vector<PlayerBounce> &p2Bounce);
    void CheckBallCollisions();

    void PrepareGoalNetting();
    void UpdateGoalNetting(bool ballTouchesNet = false);

    // for stuff like animation smoothing, we could need the time elapsed since last Put() and such
    unsigned long previousProcessTime_ms;
    unsigned long previousPreparePutTime_ms;
    unsigned long previousPutTime_ms;
    int timeSincePreviousProcess_ms;
    int timeSincePreviousPreparePut_ms;
    int timeSincePreviousPut_ms;

    MatchData *matchData;
    Team *teams[2];

    Officials *officials;

    boost::intrusive_ptr<Node> dynamicNode;

    boost::intrusive_ptr<Node> cameraNode;
    boost::intrusive_ptr<Camera> camera;
    boost::intrusive_ptr<Node> sunNode;

    boost::intrusive_ptr<Node> stadiumNode;
    boost::intrusive_ptr<Node> goalsNode;

    // camera user settings
    float cameraUserZoom;
    float cameraUserHeight;
    float cameraUserFOV;
    float cameraUserAngleFactor;

    boost::shared_ptr<AnimCollection> anims;

    const std::vector<IHIDevice*> &controllers;

    Ball *ball;

    std::vector<MentalImage*> mentalImages; // [index] == index * 10 ms ago ([0] == now)

    Gui2ScoreBoard *scoreboard;
    Gui2Radar *radar;
    Gui2TacticsDebug *tacticsDebug;
    Gui2Caption *messageCaption;
    unsigned long messageCaptionRemoveTime_ms;

    mutable Lockable<unsigned long> iterations;
    TaskSequenceInfo gameSequenceInfo;
    unsigned long matchTime_ms;
    unsigned long actualTime_ms;
    unsigned long buf_matchTime_ms;
    unsigned long buf_actualTime_ms;
    unsigned long fetchedbuf_matchTime_ms;
    unsigned long fetchedbuf_actualTime_ms;
    unsigned long goalScoredTimer;

    bool pause;
    e_MatchPhase matchPhase; // 0 - first half; 1 - second half; 2 - 1st extra time; 3 - 2nd extra time; 4 - penalties
    bool inPlay;
    bool inSetPiece;
    bool goalScored; // true after goal scored, false again after next match state change
    bool ballIsInGoal;
    int lastGoalTeamID;
    Player *lastGoalScorer;
    int lastTouchTeamIDs[e_TouchType_SIZE];
    int lastTouchTeamID;
    signed int bestPossessionTeamID;
    Player *designatedPossessionPlayer;
    Player *ballRetainer;

    bool gameOver;

    boost::intrusive_ptr<Node> fullbodyNode;
    std::map<Vector3, Vector3> colorCoords;

    ValueHistory<float> *possessionSideHistory;

    bool autoUpdateIngameCamera;

    // camera
    Quaternion cameraOrientation;
    Quaternion cameraNodeOrientation;
    Vector3 cameraNodePosition;
    float cameraFOV;
    float cameraNearCap;
    float cameraFarCap;

    TemporalSmoother<Quaternion> buf_cameraOrientation;
    TemporalSmoother<Quaternion> buf_cameraNodeOrientation;
    TemporalSmoother<Vector3> buf_cameraNodePosition;
    TemporalSmoother<float> buf_cameraFOV;
    float buf_cameraNearCap;
    float buf_cameraFarCap;
    Quaternion fetchedbuf_cameraOrientation;
    Quaternion fetchedbuf_cameraNodeOrientation;
    Vector3 fetchedbuf_cameraNodePosition;
    float fetchedbuf_cameraFOV;
    float fetchedbuf_cameraNearCap;
    float fetchedbuf_cameraFarCap;

    int fetchedbuf_timeDelta;

    unsigned int lastBodyBallCollisionTime_ms;

    std::deque<Vector3> camPos; // todo: circular buffer?

    Referee *referee;

    boost::shared_ptr<MenuTask> menuTask;

    boost::shared_ptr<Scene3D> scene3D;

    boost::intrusive_ptr<Sound> crowd01;
    boost::intrusive_ptr<Sound> crowd02;

    std::vector<ReplaySpatial*> replay;
    boost::circular_buffer<ReplayBallTouchesNetFrame> replayBallTouchesNetFrames;
    bool resetNetting;
    bool nettingHasChanged;

    float excitement;

    Vector3 previousBallPos;

    float matchDurationFactor;

    std::map < Animation*, std::vector<Vector3> > animPositionCache;

    std::vector<Vector3> nettingMeshesSrc[2];
    std::vector<float*> nettingMeshes[2];

    //boost::intrusive_ptr<Light> lightTest[100];

    // todo: this is temporary
    bool _positionLogging;
    std::ofstream positionLogFile;

    //std::vector<MissingAnim> missingAnims;

    float matchDifficulty;
};

#endif
