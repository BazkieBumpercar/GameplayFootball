// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_PAGE
#define _HPP_GUI2_VIEW_PAGE

#include "base/properties.hpp"
#include "widgets/frame.hpp"

namespace blunted {

  class Gui2WindowManager;
  class Gui2Page;

  struct Gui2PageData {
    int pageID;
    boost::shared_ptr<Properties> properties;
    void *data;
  };

  class Gui2Page : public Gui2Frame {

    public:
      Gui2Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
      virtual ~Gui2Page();

      void GoBack();
      virtual void ProcessWindowingEvent(WindowingEvent *event);

      // moved to View class: boost::signal<void()> sig_OnClose;

      void CreatePage(int pageID, void *data = 0);
      void CreatePage(int pageID, const Properties &properties, void *data = 0);

    protected:
      Gui2PageData pageData;

  };

  class Gui2PageFactory {

    public:
      Gui2PageFactory();
      virtual ~Gui2PageFactory();

      virtual void SetWindowManager(Gui2WindowManager *wm);

      virtual Gui2Page *CreatePage(int pageID, const Properties &properties, void *data = 0);
      virtual Gui2Page *CreatePage(const Gui2PageData &pageData) = 0;
      Gui2Page *GetMostRecentlyCreatedPage() { return mostRecentlyCreatedPage; }

    protected:
      Gui2WindowManager *windowManager;
      Gui2Page *mostRecentlyCreatedPage;

  };

  class Gui2PagePath {

    public:
      Gui2PagePath() {};
      virtual ~Gui2PagePath() { Clear(); };

      const std::vector<Gui2PageData> &GetPath() { return path; }
      void Push(const Gui2PageData &pageData) { path.push_back(pageData); }
      void Pop() { path.pop_back(); }
      Gui2PageData GetLast() { return path.back(); }
      void Clear() { path.clear(); }

    protected:
      std::vector<Gui2PageData> path;

  };

}

#endif
