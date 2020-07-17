// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_FILEBROWSER
#define _HPP_GUI_FILEBROWSER

#include "view.hpp"
#include "scene/objects/image2d.hpp"

#include "caption.hpp"

namespace blunted {

  enum e_DirEntryType {
    e_DirEntryType_File,
    e_DirEntryType_Directory
  };

  struct DirEntry {
    bool operator<(const DirEntry &comp) const {
      return name < comp.name;
    }
    std::string name;
    e_DirEntryType type;
  };

  class GuiFileBrowser : public GuiView {

    public:
      GuiFileBrowser(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &initialValue);
      virtual ~GuiFileBrowser();

      virtual void Init();

      void Redraw();
      void GetDirectoryContents();

      virtual void ChangeColor(int r, int g, int b, int a);
      virtual void DrawSelected();

      virtual void OnFocus();
      virtual void OnLoseFocus();
      std::string GetDirectory();
      virtual void OnKey(int sdlkID);

    protected:
      boost::intrusive_ptr<Image2D> fileBrowser;
      std::string directory;
      std::vector<GuiCaption*> fileCaptions;
      std::vector<DirEntry> dirContents;
      bool active;
      int selected;
      int offset;

  };

}

#endif
