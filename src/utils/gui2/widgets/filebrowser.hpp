// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI2_VIEW_FILEBROWSER
#define _HPP_GUI2_VIEW_FILEBROWSER

#include "../view.hpp"

#include "grid.hpp"
#include "button.hpp"

namespace blunted {

  enum e_DirEntryType {
    e_DirEntryType_File,
    e_DirEntryType_Directory,
    e_DirEntryType_None
  };

  struct DirEntry {
    DirEntry() {
      name = "";
      type = e_DirEntryType_None;
      button = 0;
    }
    std::string name;
    e_DirEntryType type;
    Gui2Button *button;
  };

  class Gui2FileBrowser : public Gui2View {

    public:
      Gui2FileBrowser(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &startDir, e_DirEntryType selectType = e_DirEntryType_File);
      virtual ~Gui2FileBrowser();

      virtual void OnGainFocus();
      virtual void OnClick();

      DirEntry GetClickedEntry();

      boost::signals2::signal<void(Gui2FileBrowser*)> sig_OnClick;

    protected:
      void DisplayDirectory();
      void GetDirectoryContents();

      std::string directory;
      e_DirEntryType selectType;

      std::vector<DirEntry> dirContents;
      int clickedID;

      Gui2Grid *grid;

  };

}

#endif
