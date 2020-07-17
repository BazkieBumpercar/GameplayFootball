// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW
#define _HPP_GUI2_VIEW

#include "defines.hpp"

#include "scene/objects/image2d.hpp"

#include "events.hpp"

namespace blunted {

  class Gui2WindowManager;

  class Gui2View : public boost::signals2::trackable {

    public:
      Gui2View(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2View();

      virtual void Exit();

      std::string GetName() { return name; }
      void SetName(const std::string &newName) { name = newName; }

      virtual void UpdateImagePosition();
      virtual void UpdateImageVisibility();
      virtual void AddView(Gui2View *view);
      virtual void RemoveView(Gui2View *view);
      virtual void SetParent(Gui2View *view);
      virtual Gui2View *GetParent();
      virtual void SetSize(float new_width_percent, float new_height_percent) { width_percent = new_width_percent; height_percent = new_height_percent; }
      virtual void SetPosition(float x_percent, float y_percent);
      virtual void GetSize(float &width_percent, float &height_percent) const;
      virtual void GetPosition(float &x_percent, float &y_percent) const;
      virtual void GetDerivedPosition(float &x_percent, float &y_percent) const;
      virtual void SnuglyFitSize(float margin = 0.5f);
      virtual void CenterPosition();
      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      std::vector<Gui2View*> GetChildren() { return children; }

      virtual void Process();
      virtual void Redraw() {}

      virtual bool ProcessEvent(Gui2Event *event);

      virtual void ProcessWindowingEvent(WindowingEvent *event);
      virtual void ProcessKeyboardEvent(KeyboardEvent *event);
      virtual void ProcessJoystickEvent(JoystickEvent *event);

      bool IsFocussed();
      void SetFocus();
      virtual void OnGainFocus() { if (!children.empty()) children.at(0)->SetFocus(); }
      virtual void OnLoseFocus() {}
      virtual void SetInFocusPath(bool onOff) { isInFocusPath = onOff; if (parent) parent->SetInFocusPath(onOff); }
      bool IsInFocusPath() { return isInFocusPath; }

      virtual bool IsVisible() { if (isVisible) { if (parent) return parent->IsVisible(); else return true; } else return false; }
      virtual bool IsSelectable() { return isSelectable; }
      virtual bool IsOverlay() { return isOverlay; }

      virtual void Show();
      virtual void ShowAllChildren();
      virtual void Hide();
      virtual void HideAllChildren();

      void SetRecursiveZPriority(int prio);
      virtual void SetZPriority(int prio);
      virtual int GetZPriority() const { return zPriority; }
      void PrintTree(int depth);

      boost::signals2::signal<void()> sig_OnClose;

    protected:
      Gui2WindowManager *windowManager;
      std::string name;
      Gui2View *parent;

      std::vector<Gui2View*> children;

      float x_percent;
      float y_percent;
      float width_percent;
      float height_percent;

      bool isVisible;
      bool isSelectable;
      bool isInFocusPath;
      bool isOverlay;

      int zPriority;

  };

}

#endif
