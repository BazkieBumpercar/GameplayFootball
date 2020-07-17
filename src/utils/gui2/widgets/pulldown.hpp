// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_PULLDOWN
#define _HPP_GUI2_VIEW_PULLDOWN

#include "../view.hpp"

#include "grid.hpp"
#include "button.hpp"
#include "image.hpp"

namespace blunted {

  struct PulldownEntry {
    std::string name;
    Gui2Button *button;
  };

  class Gui2Pulldown : public Gui2View {

    public:
      Gui2Pulldown(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Pulldown();

      void AddEntry(const std::string &caption, const std::string &name);
      void PullDownOrUp();
      void SetSelected(int selectedEntry);
      std::string GetSelected() const;

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      boost::signals2::signal<void(Gui2Pulldown*)> sig_OnChange;

    protected:
      void Select(int selectedEntry);

      Gui2Button *pulldownButton;

      std::vector<PulldownEntry> entries;
      int selectedEntry;

      Gui2Image *bg;
      Gui2Grid *grid;

      bool pulledDown;

  };

}

#endif
