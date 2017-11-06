// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_PLAYER
#define _HPP_PLAYER

#include "humanoid/humanoid.hpp"
#include "playerbase.hpp"

#include "utils/gui2/widgets/caption.hpp"

#include "../../menu/menutask.hpp"

class Match;
class Team;
class ElizaController;

struct TacticalPlayerSituation {
  float forwardSpaceRating;
  float toGoalSpaceRating;
  float spaceRating;
  float forwardRating;
};

class Player : public PlayerBase {

  public:
    Player(Team *team, PlayerData *playerData);
    virtual ~Player();

    Humanoid *CastHumanoid();
    ElizaController *CastController();

    int GetTeamID() const;
    Team *GetTeam();

    // get ready for some action
    virtual void Activate(boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection);
    // go back to bench/take a shower
    virtual void Deactivate();

    bool TouchPending() { return CastHumanoid()->TouchPending(); }
    bool TouchAnim() { return CastHumanoid()->TouchAnim(); }
    Vector3 GetTouchPos() { return CastHumanoid()->GetTouchPos(); }
    int GetTouchFrame() { return CastHumanoid()->GetTouchFrame(); }
    int GetCurrentFrame() { return CastHumanoid()->GetCurrentFrame(); }

    void SelectRetainAnim() { CastHumanoid()->SelectRetainAnim(); }

    inline e_FunctionType GetCurrentFunctionType() { return CastHumanoid()->GetCurrentFunctionType(); }
    FormationEntry GetFormationEntry();
    inline void SetDynamicFormationEntry(FormationEntry entry) { dynamicFormationEntry = entry; }
    inline FormationEntry GetDynamicFormationEntry() { return dynamicFormationEntry; }
    inline void SetManMarkingID(int id) { manMarkingID = id; }
    inline int GetManMarkingID() { return manMarkingID; }

    bool HasPossession() const;
    bool HasBestPossession() const;
    bool HasUniquePossession() const;
    inline int GetPossessionDuration_ms() const { return possessionDuration_ms; }
    inline int GetTimeNeededToGetToBall_ms() const { return timeNeededToGetToBall_ms; }
    inline int GetTimeNeededToGetToBall_optimistic_ms() const { return timeNeededToGetToBall_optimistic_ms; }
    inline int GetTimeNeededToGetToBall_previous_ms() const { return timeNeededToGetToBall_previous_ms; }
    void SetDesiredTimeToBall_ms(int ms) { desiredTimeToBall_ms = ms; }
    int GetDesiredTimeToBall_ms() const { return clamp(desiredTimeToBall_ms, timeNeededToGetToBall_ms, 1000000.0f); }
    bool AllowLastDitch(bool includingPossessionAmount = true) const;

    void TriggerControlledBallCollision() { triggerControlledBallCollision = true; }
    bool IsControlledBallCollisionTriggered() { return triggerControlledBallCollision; }
    void ResetControlledBallCollisionTrigger() { triggerControlledBallCollision = false; }

    float GetAverageVelocity(float timePeriod_sec); // is reset on ResetSituation() calls

    void UpdatePossessionStats(bool onInterval = true);

    float GetClosestOpponentDistance() const;

    const TacticalPlayerSituation &GetTacticalSituation() { return tacticalSituation; }

    virtual void Process();
    virtual void PreparePutBuffers(unsigned long snapshotTime_ms);
    virtual void FetchPutBuffers(unsigned long putTime_ms);
    void Put2D();
    void Hide2D();

    void GiveYellowCard(unsigned long giveTime_ms) { cards++; cardEffectiveTime_ms = giveTime_ms; }
    void GiveRedCard(unsigned long giveTime_ms) { cards += 3; cardEffectiveTime_ms = giveTime_ms; }
    int GetCards() const { return cards; }

    void SendOff();

    float GetStaminaStat() const;
    virtual float GetStat(const char *name) const;

    virtual void ResetSituation(const Vector3 &focusPos);

  protected:
    void _CalculateTacticalSituation();

    Team *team;

    signed int manMarkingID;

    FormationEntry dynamicFormationEntry;

    bool hasPossession;
    bool hasBestPossession;
    bool hasUniquePossession;
    int possessionDuration_ms;
    unsigned int timeNeededToGetToBall_ms;
    unsigned int timeNeededToGetToBall_optimistic_ms;
    unsigned int timeNeededToGetToBall_previous_ms;

    bool triggerControlledBallCollision;

    TacticalPlayerSituation tacticalSituation;

    bool buf_nameCaptionShowCondition;
    bool buf_debugCaptionShowCondition;
    std::string buf_nameCaption;
    std::string buf_debugCaption;
    Vector3 buf_nameCaptionPos;
    Vector3 buf_debugCaptionPos;
    Vector3 buf_playerColor;
    Vector3 buf_debugCaptionColor;

    bool fetchedbuf_nameCaptionShowCondition;
    bool fetchedbuf_debugCaptionShowCondition;
    std::string fetchedbuf_nameCaption;
    std::string fetchedbuf_debugCaption;
    Vector3 fetchedbuf_nameCaptionPos;
    Vector3 fetchedbuf_debugCaptionPos;
    Vector3 fetchedbuf_playerColor;
    Vector3 fetchedbuf_debugCaptionColor;

    Gui2Caption *nameCaption;
    Gui2Caption *debugCaption;

    boost::shared_ptr<MenuTask> menuTask;

    int desiredTimeToBall_ms;
    Vector3 idealMovement;

    int cards; // 1 == 1 yellow; 2 == 2 yellow; 3 == 1 red; 4 == 1 yellow, 1 red

    unsigned long cardEffectiveTime_ms;

};

#endif
