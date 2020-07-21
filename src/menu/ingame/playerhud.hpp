// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_PLAYERHUD
#define _HPP_GUI2_VIEW_PLAYERHUD

#include "utils/gui2/view.hpp"

#include "scene/objects/image2d.hpp"

using namespace blunted;

class Match;

class Gui2PlayerHUD : public Gui2View {

  public:
    Gui2PlayerHUD(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, Match *match);
    virtual ~Gui2PlayerHUD();

    void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

    virtual void Redraw();

  protected:
    boost::intrusive_ptr<Image2D> image;

    Match *match;

};

#endif
