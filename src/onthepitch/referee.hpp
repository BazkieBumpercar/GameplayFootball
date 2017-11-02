// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_REFEREE
#define _HPP_REFEREE

#include "defines.hpp"
#include "../gamedefines.hpp"

#include "scene/scene3d/scene3d.hpp"
#include "scene/objects/sound.hpp"

using namespace blunted;

class Match;

struct RefereeBuffer {
  bool active;
  e_SetPiece desiredSetPiece;
  signed int teamID;
  unsigned long stopTime;
  unsigned long prepareTime;
  unsigned long startTime;
  Vector3 restartPos;
  Player *taker;
  bool endPhase;
};

struct Foul {
  Player *foulPlayer;
  Player *foulVictim;
  int foulType; // 0: nothing, 1: foul, 2: yellow, 3: red
  bool advantage;
  unsigned long foulTime;
  Vector3 foulPosition;
  bool hasBeenProcessed;
};

class Referee {

  public:
    Referee(Match *match);
    virtual ~Referee();

    void Process();

    void PrepareSetPiece(e_SetPiece setPiece);

    const RefereeBuffer &GetBuffer() { return buffer; };

    void AlterSetPiecePrepareTime(unsigned long newTime_ms);

    void BallTouched();
    void TripNotice(Player *tripee, Player *tripper, int tackleType); // 1 == standing tackle resulting in little trip, 2 == standing tackle resulting in fall, 3 == sliding tackle
    bool CheckFoul();

    Player *GetCurrentFoulPlayer() { return foul.foulPlayer; }
    int GetCurrentFoulType() { return foul.foulType; }

  protected:
    Match *match;

    boost::shared_ptr<Scene3D> scene3D;

    RefereeBuffer buffer;

    int afterSetPieceRelaxTime_ms; // throw-ins cause immediate new throw-ins, because ball is still outside the lines at the moment of throwing ;)

    std::map<Player*, Vector3> offsidePlayers; // player, position at time of touch

    Foul foul;

    boost::intrusive_ptr<Sound> whistle[4]; // 0: short, 1: long, 2: half time, 3: full time

};

#endif
