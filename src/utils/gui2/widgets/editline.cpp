// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "editline.hpp"

#include "../windowmanager.hpp"

namespace blunted {

  Gui2EditLine::Gui2EditLine(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &defaultText) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent), currentText(defaultText) {
    color = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);
    outlineColor = windowManager->GetStyle()->GetColor(e_DecorationType_Dark1);

    caption = new Gui2Caption(windowManager, GetName() + "_caption", 0, 0, width_percent, height_percent, defaultText);
    caption->SetCaption(defaultText);
    caption->SetColor(color);
    caption->SetOutlineColor(outlineColor);
    this->AddView(caption);
    caption->Show();

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

    cursor = new Gui2Image(windowManager, GetName() + "_cursor", 0, 0, 0.3f, height_percent);

    Vector3 cursorSize = cursor->GetImage2D()->GetSize();
    cursor->GetImage2D()->DrawRectangle(0, 0, cursorSize.coords[0], cursorSize.coords[1], windowManager->GetStyle()->GetColor(e_DecorationType_Bright1), 127);
    cursor->GetImage2D()->OnChange();
    this->AddView(cursor);
    cursor->Hide();
    cursorPos = defaultText.size();
    cursor->SetPosition(caption->GetTextWidthPercent(cursorPos), 0);

    isSelectable = true;

    maxLength = 32;
    allowedChars = "";
  }

  Gui2EditLine::~Gui2EditLine() {
  }

  void Gui2EditLine::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
    Gui2View::GetImages(target);
  }

  void Gui2EditLine::SetColor(const Vector3 &color) {
    this->color = color;
    caption->SetColor(color);
  }

  void Gui2EditLine::SetOutlineColor(const Vector3 &outlineColor) {
    this->outlineColor = outlineColor;
    caption->SetOutlineColor(outlineColor);
  }

  void Gui2EditLine::SetText(const std::string &newText) {
    caption->SetCaption(newText);
  }

  void Gui2EditLine::OnGainFocus() {
    caption->SetColor(windowManager->GetStyle()->GetColor(e_DecorationType_Bright2));
    cursor->Show();
    sig_OnGainFocus(this);
  }

  void Gui2EditLine::OnLoseFocus() {
    caption->SetColor(color);
    cursor->Hide();
    sig_OnLoseFocus(this);
  }

  void Gui2EditLine::ProcessKeyboardEvent(KeyboardEvent *event) {
    bool nothingHappened = true;

    std::vector<SDL_keysym> &keysym = event->GetKeysymRepeated();
    std::vector<SDL_keysym>::iterator iter = keysym.begin();
    while (iter != keysym.end()) {

      if (event->GetKeyRepeated((*iter).sym)) {

        nothingHappened = false;

        switch ((*iter).sym) {
          case SDLK_LEFT: {
            cursorPos--;
            if (cursorPos < 0) cursorPos = 0;
            break;
          }
          case SDLK_RIGHT: {
            cursorPos++;
            if (cursorPos > (signed int)currentText.length()) cursorPos = (signed int)currentText.length();
            break;
          }
          case SDLK_BACKSPACE: {
            if (cursorPos > 0) {
              cursorPos--;
              currentText.erase(cursorPos, 1);
              sig_OnChange(this);
            }
            break;

          }
          case SDLK_DELETE: {
            if (cursorPos < (signed int)currentText.length()) {
              currentText.erase(cursorPos, 1);
              sig_OnChange(this);
            }
            break;
          }
          case SDLK_RETURN:
          case SDLK_TAB: {
            if (GetParent()) {
              WindowingEvent *wEvent = new WindowingEvent();
              wEvent->SetDirection(Vector3(0, 1, 0));
              GetParent()->ProcessEvent(wEvent);
              delete wEvent;
              sig_OnEnter(this);
            }
            break;
          }
          default:
          break;
        }

        // ascii char
        if (((*iter).unicode & 0xFF80) == 0 && (*iter).unicode >= 0x20 && currentText.length() <= maxLength) {
          char ch = ((*iter).unicode & 0x7F);
          if (allowedChars.length() == 0 || allowedChars.find_first_of(ch) != std::string::npos) {
            currentText.insert(cursorPos, 1, ch);
            cursorPos++;
            sig_OnChange(this);
          }
        }

      }

      iter++;
    }

    if (!nothingHappened) {
      SetText(currentText);

      float x, y;
      cursor->GetPosition(x, y);
      cursor->SetPosition(caption->GetTextWidthPercent(cursorPos), y);
    }
  }

}
