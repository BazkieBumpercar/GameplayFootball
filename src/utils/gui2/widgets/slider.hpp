// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_SLIDER
#define _HPP_GUI2_VIEW_SLIDER

#include "SDL2/SDL_ttf.h"

#include "../view.hpp"

#include "scene/objects/image2d.hpp"

#include "caption.hpp"

namespace blunted {

  struct Gui2Slider_HelperValue {
    int index;
    Vector3 color;
    float value;
    Gui2Caption *descriptionCaption;
  };

  class Gui2Slider : public Gui2View {

    public:
      Gui2Slider(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption);
      virtual ~Gui2Slider();

      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      virtual void Process();
      virtual void Redraw();

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      virtual void Show() { titleCaption->Show(); Gui2View::Show(); } // ignore helper descriptions

      virtual void OnGainFocus();
      virtual void OnLoseFocus();

      void SetValue(float newValue);
      float GetValue() { return quantizedValue; }

      void SetQuantization(int steps) { quantizationSteps = std::max(steps, 2); }

      int AddHelperValue(const Vector3 &color, const std::string &description, float initialValue = 0.0f);
      void SetHelperValue(int index, float value);
      void DeleteHelperValue(int index);

      boost::signals2::signal<void(Gui2Slider*)> sig_OnChange;

    protected:
      boost::intrusive_ptr<Image2D> image;

      int fadeOut_ms;
      int fadeOutTime_ms;
      int switchHelperDescription_ms;
      int switchHelperDescriptionTime_ms;
      int activeDescription;
      int quantizationSteps;

      std::string caption;

      Gui2Caption *titleCaption;

      std::vector<Gui2Slider_HelperValue> helperValues;

      float value;
      float quantizedValue;

  };

}

#endif
