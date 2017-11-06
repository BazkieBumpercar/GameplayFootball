// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "icontroller.hpp"

#include "../../match.hpp"
#include "../playerbase.hpp"

void IController::SetPlayer(PlayerBase *player) {
  this->player = player;
}

int IController::GetReactionTime_ms() {
  return int(round(80.0f - player->GetStat("physical_reaction") * 40.0f));
}
