// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_STRATEGY
#define _HPP_STRATEGY

#include "../../../../gamedefines.hpp"

#include "../../../AIsupport/AIfunctions.hpp"
#include "../../../AIsupport/mentalimage.hpp"

#include "../elizacontroller.hpp"

#include "../../player.hpp"
#include "../../../team.hpp"
#include "../../../match.hpp"

using namespace blunted;

class Strategy {

  public:
    Strategy(ElizaController *controller) : controller(controller), player(controller->GetPlayer()), team(controller->GetTeam()), match(controller->GetMatch()) {};
    virtual ~Strategy() {};

    Player *CastPlayer() { return static_cast<Player*>(player); }

    virtual void RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity) = 0;

    std::string GetName() { return name; }

  protected:
    ElizaController *controller;

    std::string name;

    // for convenience
    PlayerBase *player;
    Team *team;
    Match *match;

};

#endif
