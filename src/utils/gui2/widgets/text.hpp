// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_TEXT
#define _HPP_GUI2_VIEW_TEXT

#include "../view.hpp"

#include "caption.hpp"

#include "scene/objects/image2d.hpp"

namespace blunted {

  class Gui2Text : public Gui2View {

    public:
      Gui2Text(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, float fontsize_percent, unsigned int maxHorizChars, const std::string &text);
      virtual ~Gui2Text();

      void SetColor(const Vector3 &color);
      void SetOutlineColor(const Vector3 &outlineColor);

      void ClearText();
      void AddEmptyLine();
      void AddText(const std::string &newText);
      std::string GetText() { return text; }

    protected:
      boost::intrusive_ptr<Image2D> image;
      float fontsize_percent;
      unsigned int maxHorizChars;
      std::string text;
      std::vector<std::string> resultText;
      std::vector<Gui2Caption*> resultCaptions;
      Vector3 color;
      Vector3 outlineColor;

  };

}

#endif
