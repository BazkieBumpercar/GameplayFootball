// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_DIALOG
#define _HPP_GUI2_VIEW_DIALOG

#include "../view.hpp"

#include "frame.hpp"

namespace blunted {

  class Gui2Grid;
  class Gui2Button;

  class Gui2Dialog : public Gui2Frame {

    public:
      Gui2Dialog(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2Dialog();

      virtual void AddContent(Gui2View *view);

      virtual Gui2Button *AddPosNegButtons(const std::string &posName, const std::string &negName);
      virtual Gui2Button *AddSingleButton(const std::string &caption);

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      //boost::signals2::signal<void(Gui2Dialog*)> sig_OnClose;
      boost::signals2::signal<void(Gui2Dialog*)> sig_OnPositive;
      boost::signals2::signal<void(Gui2Dialog*)> sig_OnNegative;

    protected:
      Gui2Grid *grid;

  };

}

#endif
