// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_TIMELINE
#define _HPP_GUI_TIMELINE

#include "view.hpp"
#include "scene/objects/image2d.hpp"
#include "caption.hpp"

namespace blunted {

  struct TimelinePart {
    std::string name;
    std::vector<int> keyFrames;
    GuiCaption *caption;
  };

  class GuiTimeline : public GuiView {

    public:
      GuiTimeline(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent);
      virtual ~GuiTimeline();

      virtual void Init();

      void AddPart(const std::string id, const std::string caption);

      void Redraw();

      virtual void OnFocus();
      virtual void OnLoseFocus();
      virtual void OnKey(int sdlkID);

      void GetLocation(std::string &partName, int &currentFrame, bool &isKeyFrame);
      int GetFrameCount() { return frameCount; }
      void ClearKeys();
      bool ToggleKeyFrame();
      void EnableKeyFrame(const std::string &partName, int frame);
      void EnableKeyFrame(int part, int frame);

    protected:
      boost::intrusive_ptr<Image2D> timeline;
      std::vector<TimelinePart> parts;
      bool focussed;
      int selected;
      int selectedFrame;
      int frameCount;
      int grid;

  };

}

#endif
