// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_RADAR
#define _HPP_GUI2_VIEW_RADAR

#include "utils/gui2/view.hpp"
#include "utils/gui2/widgets/image.hpp"

#include "scene/objects/image2d.hpp"

class Match;

namespace blunted {

  class Gui2Radar : public Gui2View {

    public:
      Gui2Radar(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, Match *match, const Vector3 &color1_1, const Vector3 &color1_2, const Vector3 &color2_1, const Vector3 &color2_2);
      virtual ~Gui2Radar();

      void ReloadAvatars(int teamID, unsigned int playerCount);

      virtual void Process();
      virtual void Put();

    protected:
      Gui2Image *bg;
      std::vector<Gui2Image*> team1avatars;
      std::vector<Gui2Image*> team2avatars;
      Gui2Image* ball;

      Match *match;

      Vector3 color1_1, color1_2, color2_1, color2_2;

  };

}

#endif
