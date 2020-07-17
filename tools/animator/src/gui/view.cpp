// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "view.hpp"

#include "base/log.hpp"

namespace blunted {

  GuiView::GuiView(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent) : scene2D(scene2D), name(name), x1_percent(x1_percent), y1_percent(y1_percent), x2_percent(x2_percent), y2_percent(y2_percent) {
    focussedView = 0;
    parent = 0;

    assert(x2_percent > x1_percent);
    assert(y2_percent > y1_percent);
  }

  GuiView::~GuiView() {
    for (int i = 0; i < (signed int)views.size(); i++) {
      //printf("removing child view %i.. ", i);
      delete views.at(i);
      //printf("done\n");
    }
    views.clear();
  }


  std::string GuiView::GetName() {
    return name;
  }

  void GuiView::GetCoord(float x_percent, float y_percent, int &x, int &y) {
    if (parent) {
      parent->GetCoord(x_percent, y_percent, x, y);
    } else {
      Log(e_FatalError, "GuiView", "GetCoord", "GUI root did not properly implement GetCoord");
    }
  }

  int GuiView::GetX(float x_percent) {
    if (parent) {
      return parent->GetX(x_percent);
    } else {
      Log(e_FatalError, "GuiView", "GetX", "GUI root did not properly implement GetX()");
      return 0;
    }
  }

  int GuiView::GetY(float y_percent) {
    if (parent) {
      return parent->GetY(y_percent);
    } else {
      Log(e_FatalError, "GuiView", "GetY", "GUI root did not properly implement GetY()");
      return 0;
    }
  }


  void GuiView::AddView(GuiView *view) {
    views.push_back(view);
    view->SetParent(this);
    view->Init();
    if (focussedView == 0) SetFocussedView(view);
  }

  void GuiView::DeleteView(GuiView *view) {
    if (focussedView == view) {
      if (views.size() == 0) SetFocussedView(0);
        else SetFocussedView(views.at(0));
    }
    std::vector<GuiView*>::iterator viewIter = views.begin();
    while (viewIter != views.end()) {
      if ((*viewIter) == view) {
        views.erase(viewIter);
        delete view;
        view = 0;
        viewIter = views.end();
      } else {
        viewIter++;
      }
    }
  }

  void GuiView::SetParent(GuiView *view) {
    parent = view;
  }

  void GuiView::SetFocussedView(GuiView *view) {
    if (focussedView != view && focussedView) focussedView->OnLoseFocus();
    focussedView = view;
    if (focussedView) focussedView->OnFocus();
  }

  void GuiView::SetFocus() {
    if (parent) parent->SetFocussedView(this);
  }

  void GuiView::OnKey(int sdlkID) {
    if (focussedView) focussedView->OnKey(sdlkID);
  }

  void GuiView::SwitchFocus(signed int offset) {
    // find focussed view in view vector
    signed int found = -1;
    int i = 0;
    while (found == -1 && i < (signed int)views.size()) {
      if (views.at(i) == focussedView) found = i;
      i++;
    }
    if (found != -1) {
      signed int target = found + offset;
      if (target > (signed int)views.size() - 1) target = 0;
      if (target < 0) target = (signed int)views.size() - 1;
      if (target != found) SetFocussedView(views.at(target)); else SetFocussedView(0);
    } else {
      SetFocussedView(0);
    }
  }


  void GuiView::EmitSignal(GuiView *sender, SDLKey key) {
    if (parent) {
      parent->EmitSignal(sender, key);
    } else {
      Log(e_FatalError, "GuiView", "EmitSignal", "GUI root did not properly implement EmitSignal()");
    }
  }

  void GuiView::EmitString(GuiView *sender, const std::string &someString) {
    if (parent) {
      parent->EmitString(sender, someString);
    } else {
      Log(e_FatalError, "GuiView", "EmitString", "GUI root did not properly implement EmitString()");
    }
  }

}
