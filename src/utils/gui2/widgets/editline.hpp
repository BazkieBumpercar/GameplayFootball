// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_EDITLINE
#define _HPP_GUI2_VIEW_EDITLINE

#include "../view.hpp"
#include "caption.hpp"
#include "image.hpp"

namespace blunted {

  class Gui2EditLine : public Gui2View {

    public:
      Gui2EditLine(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &defaultText);
      virtual ~Gui2EditLine();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      void SetColor(const Vector3 &color);
      void SetOutlineColor(const Vector3 &outlineColor);
      void SetText(const std::string &newText);
      std::string GetText() { return currentText; }
      void SetMaxLength(int length) { maxLength = length; }
      void SetAllowedChars(const std::string &chars) { allowedChars = chars; }

      virtual void Show() { caption->Show(); Gui2View::Show(); } // ignore cursor

      virtual void OnGainFocus();
      virtual void OnLoseFocus();
      virtual void ProcessKeyboardEvent(KeyboardEvent *event);

      boost::signals2::signal<void(Gui2EditLine*)> sig_OnEnter;
      boost::signals2::signal<void(Gui2EditLine*)> sig_OnGainFocus;
      boost::signals2::signal<void(Gui2EditLine*)> sig_OnLoseFocus;
      boost::signals2::signal<void(Gui2EditLine*)> sig_OnChange;

    protected:
      Gui2Caption *caption;
      Gui2Image *cursor;

      std::string currentText;
      std::string allowedChars;
      unsigned int maxLength;
      Vector3 color;
      Vector3 outlineColor;
      float textWidth_percent;

      int cursorPos;

  };

}

#endif
