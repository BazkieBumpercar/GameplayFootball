// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_TACTICSDEBUG
#define _HPP_GUI2_VIEW_TACTICSDEBUG

#include "utils/gui2/view.hpp"

#include "scene/objects/image2d.hpp"

using namespace blunted;

class Match;

struct TacticsDebugEntry {
  std::string caption;
  float value[3][2]; // [type][teamid]
  Vector3 color[3][2];
};

class Gui2TacticsDebug : public Gui2View {

  public:
    Gui2TacticsDebug(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, Match *match);
    virtual ~Gui2TacticsDebug();

    void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

    virtual void Redraw();

    virtual unsigned int AddEntry(const std::string &caption, const Vector3 &color1, const Vector3 &color2, const Vector3 &color3);
    virtual void SetValue(unsigned int entryID, int typeID, int teamID, float value);

  protected:
    boost::intrusive_ptr<Image2D> image;

    int w, h;

    SDL_Surface *bg;

    Match *match;

    std::vector<TacticsDebugEntry> entries;

    mutable bool _dirtycache;

};

#endif
