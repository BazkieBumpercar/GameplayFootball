// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_MENU
#define _HPP_GUI2_VIEW_MENU

#include "frame.hpp"
#include "base/properties.hpp"

namespace blunted {

  class Gui2Menu : public Gui2Frame {

    public:
      Gui2Menu(Gui2WindowManager *windowManager, Gui2Menu *parentMenu, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Menu();

      virtual void Process();

      void DecoupleParent(); // for unlinking parent menu
      void _AddSubmenu(Gui2Menu *view);
      void _RemoveSubmenu(Gui2Menu *view);
      Gui2Menu *GetParentMenu() { return parentMenu; }

      virtual void Deactivate();
      virtual void Reactivate();

      virtual void ProcessWindowingEvent(WindowingEvent *event);

    protected:
      Gui2Menu *parentMenu;
      Gui2View *lastFocus;

      std::vector<Gui2Menu*> subMenus;

  };

}

#endif
