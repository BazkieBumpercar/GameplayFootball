// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_GRID
#define _HPP_GUI2_VIEW_GRID

#include "../view.hpp"
#include "scrollbar.hpp"

#include "scene/objects/image2d.hpp"

namespace blunted {

  struct GridContainer {
    int row;
    int col;
    Gui2View *view;
  };

  class Gui2Grid : public Gui2View {

    public:
      Gui2Grid(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2Grid();

      virtual void Process();

      virtual void AddView(Gui2View *view, int row = -1, int col = 0);
      virtual void RemoveView(Gui2View *view);
      virtual void RemoveView(int row, int col);
      Gui2View *FindView(int row, int col);

      virtual int GetRow(Gui2View *view);
      virtual int GetColumn(Gui2View *view);

      Gui2View *GetSelectedView() { return FindView(selectedRow, selectedCol); }

      virtual void SetQuickScroll(bool onOff) { quickScroll = onOff; }
      virtual void SetWrapping(bool rowOnOff = true, bool colOnOff = true) { rowWrap = rowOnOff; colWrap = colOnOff; }

      virtual void SetMaxVisibleRows(int visibleRowCount);
      virtual void UpdateLayout(float margin_left_percent = 0.5f, float margin_right_percent = 0.5f, float margin_top_percent = 0.5f, float margin_bottom_percent = 0.5f);
      void UpdateScrolling();
      void UpdateScrollbars();

      virtual void ProcessWindowingEvent(WindowingEvent *event);

      virtual void OnGainFocus();
      virtual void SetInFocusPath(bool onOff);

      virtual bool IsSelectable() { return hasSelectables; }

      virtual void Show();
      virtual void Hide();

    protected:
      boost::intrusive_ptr<Image2D> image;

      std::vector<GridContainer> container;

      bool hasSelectables;

      int rows;
      int cols;

      int selectedRow;
      int selectedCol;

      int offsetRows;
      int maxVisibleRows;
      float margin_left_percent;
      float margin_right_percent;
      float margin_top_percent;
      float margin_bottom_percent;

      int switchDelay_ms;
      int minSwitchDelay_ms;

      bool quickScroll; // left/right == pageup/pagedown (if 1 column)
      bool rowWrap;
      bool colWrap;

      Gui2Scrollbar *scrollX;
      Gui2Scrollbar *scrollY;

  };

}

#endif
