// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_VIEW
#define _HPP_GUI_VIEW

#include "defines.hpp"

#include "scene/scene2d/scene2d.hpp"

#include <SDL/SDL.h>

namespace blunted {

  class GuiView {

    public:
      GuiView(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent);
      virtual ~GuiView();

      virtual void Init() = 0;
      std::string GetName();

      virtual void GetCoord(float x_percent, float y_percent, int &x, int &y);
      virtual int GetX(float x_percent);
      virtual int GetY(float y_percent);

      void AddView(GuiView *view);
      void DeleteView(GuiView *view);
      void SetParent(GuiView *view);
      void SetFocussedView(GuiView *view);
      void SetFocus();
      virtual void OnFocus() {};
      virtual void OnLoseFocus() {};
      virtual void OnKey(int sdlkID);
      virtual void SwitchFocus(signed int offset = 1);

      virtual void EmitSignal(GuiView *sender, SDLKey key);
      virtual void EmitString(GuiView *sender, const std::string &someString);

    protected:
      boost::shared_ptr<Scene2D> scene2D;
      std::string name;

      GuiView *parent;
      std::vector<GuiView*> views;
      GuiView *focussedView;

      float x1_percent, y1_percent, x2_percent, y2_percent;

  };

}

#endif
