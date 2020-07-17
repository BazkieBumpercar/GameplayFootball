// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI_DIALOG_FILE
#define _HPP_GUI_DIALOG_FILE

#include "scene/objects/image2d.hpp"

#include "view.hpp"
#include "filebrowser.hpp"
#include "textinput.hpp"

namespace blunted {

  class GuiFileDialog : public GuiView {

    public:
      GuiFileDialog(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &directory, const std::string &initialFile);
      virtual ~GuiFileDialog();

      virtual void Init();

      std::string GetFilename();
      std::string GetDirectory();

      virtual void OnKey(int sdlkID);
      virtual void EmitString(GuiView *sender, const std::string &someString);

    protected:
      boost::intrusive_ptr<Image2D> background;
      std::string directory;
      std::string initialFile;

      GuiTextInput *filenameInput;
      GuiFileBrowser *fileBrowser;

  };

}

#endif
