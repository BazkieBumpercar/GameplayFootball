// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_HUMANGAMER
#define _HPP_HUMANGAMER

#include "defines.hpp"

#include "scene/scene3d/scene3d.hpp"

#include "player/controller/humancontroller.hpp"
#include "../hid/ihidevice.hpp"

using namespace blunted;

class Team;

enum e_PlayerColor {
  e_PlayerColor_Blue,
  e_PlayerColor_Green,
  e_PlayerColor_Red,
  e_PlayerColor_Yellow,
  e_PlayerColor_Purple,
  e_PlayerColor_Default
};

class HumanGamer {

  public:
    HumanGamer(Team *team, IHIDevice *hid, e_PlayerColor color);
    virtual ~HumanGamer();

    int GetSelectedPlayerID() const;
    Player *GetSelectedPlayer() const { return selectedPlayer; }
    void SetSelectedPlayerID(int id);
    IHIDevice *GetHIDevice() { return hid; }
    HumanController *GetHumanController() { return controller; }

    e_PlayerColor GetPlayerColor() const { return playerColor; }

    void PreparePutBuffers();
    void Put();

  protected:
    Team *team;
    IHIDevice *hid;
    HumanController *controller;

    e_PlayerColor playerColor;
    Player *selectedPlayer;

};

#endif
