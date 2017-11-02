// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_SCOREBOARD
#define _HPP_GUI2_VIEW_SCOREBOARD

#include "utils/gui2/view.hpp"

#include "scene/objects/image2d.hpp"
#include "utils/gui2/widgets/caption.hpp"
#include "utils/gui2/widgets/image.hpp"

using namespace blunted;

class Match;

class Gui2ScoreBoard : public Gui2View {

  public:
    Gui2ScoreBoard(Gui2WindowManager *windowManager, Match *match);
    virtual ~Gui2ScoreBoard();

    void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

    virtual void Redraw();

    void SetTimeStr(const std::string &timeStr);
    void SetGoalCount(int teamID, int goalCount);

  protected:
    boost::intrusive_ptr<Image2D> image;

    int w, h;
    float content_yOffset;

    Match *match;

    std::string timeStr;
    int goalCount[2];

    Gui2Caption *timeCaption;
    Gui2Caption *teamNameCaption[2];
    Gui2Caption *goalCountCaption[2];

    Gui2Image *leagueLogo;
    Gui2Image *teamLogo[2];

    Gui2Image *tvLogo;
};

#endif
