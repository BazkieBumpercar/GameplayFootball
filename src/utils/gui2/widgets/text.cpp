// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "text.hpp"

#include "../windowmanager.hpp"

#include "SDL/SDL_ttf.h"

namespace blunted {

  Gui2Text::Gui2Text(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, float fontsize_percent, unsigned int maxHorizChars, const std::string &text) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent), fontsize_percent(fontsize_percent), maxHorizChars(maxHorizChars) {
    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

    color = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);
    outlineColor = windowManager->GetStyle()->GetColor(e_DecorationType_Dark1);

    if (text.length() > 0) AddText(text);
  }

  Gui2Text::~Gui2Text() {
  }

  void Gui2Text::SetColor(const Vector3 &color) {
    if (color != this->color) {
      this->color = color;
      Redraw();
    }
  }

  void Gui2Text::SetOutlineColor(const Vector3 &outlineColor) {
    if (outlineColor != this->outlineColor) {
      this->outlineColor = outlineColor;
      Redraw();
    }
  }

  void Gui2Text::ClearText() {
    text.clear();

    resultText.clear();

    std::vector<Gui2View*> childrenCopy = children; // need to make copy: child->Exit will remove itself from *this->children
    for (int i = (signed int)childrenCopy.size() - 1; i >= 0; i--) { // filo
      childrenCopy.at(i)->Exit();
      delete childrenCopy.at(i);
    }
    children.clear();
  }

  void Gui2Text::AddEmptyLine() {
    text.append("\n");

    resultText.push_back("");

    Gui2Caption *caption = new Gui2Caption(windowManager, GetName() + int_to_str(resultText.size() - 1), 0, (resultText.size() - 1) * fontsize_percent * 1.5f, width_percent, fontsize_percent, "");
    this->AddView(caption);
  }

  void Gui2Text::AddText(const std::string &newText) {
    text.append(newText);

    // concatenate letters until desired maxwidth is reached
    std::string line;
    std::string tmpLine;
    bool ready = false;
    size_t processPos = 0;
    while (!ready) {
      size_t spacePos = newText.find_first_of(" ", processPos);

      if (spacePos == std::string::npos) {
        spacePos = newText.length() - 1;
      }
      if (processPos >= newText.length()) ready = true;

      tmpLine.append(newText.substr(processPos, spacePos - processPos + 1));

      // time for newline
      if (tmpLine.length() > maxHorizChars || ready) {
        resultText.push_back(line);
        tmpLine.clear();
      } else {
        line = tmpLine;
        processPos = spacePos + 1;
      }
    }

    for (unsigned int i = 0; i < resultText.size(); i++) {
      Gui2Caption *caption = new Gui2Caption(windowManager, GetName() + int_to_str(i), 0, i * fontsize_percent * 1.5f, width_percent, fontsize_percent, resultText.at(i));
      this->AddView(caption);
      caption->Show();
    }
  }


}
