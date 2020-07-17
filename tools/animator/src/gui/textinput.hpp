// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_TEXTINPUT
#define _HPP_GUI_TEXTINPUT

#include "view.hpp"
#include "scene/objects/image2d.hpp"

#include "caption.hpp"

namespace blunted {

  class GuiTextInput : public GuiView {

    public:
      GuiTextInput(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &initialValue);
      virtual ~GuiTextInput();

      virtual void Init();

      virtual void ChangeColor(int r, int g, int b, int a);

      virtual void OnFocus();
      virtual void OnLoseFocus();
      void SetValue(const std::string &newValue);
      std::string GetValue();
      virtual void OnKey(int sdlkID);

    protected:
      boost::intrusive_ptr<Image2D> textInput;
      std::string value;
      GuiCaption *text;
      bool active;

  };

}

#endif
