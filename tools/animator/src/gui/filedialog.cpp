// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "filedialog.hpp"

#include "base/sdl_surface.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "scene/objectfactory.hpp"

#include "button.hpp"

namespace blunted {

  GuiFileDialog::GuiFileDialog(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &directory, const std::string &initialFile) : GuiView(scene2D, name, x1_percent, y1_percent, x2_percent, y2_percent), directory(directory), initialFile(initialFile) {
  }

  GuiFileDialog::~GuiFileDialog() {
    scene2D->DeleteObject(background);
    background.reset();
  }

  void GuiFileDialog::Init() {
    SDL_Surface *sdlSurface = CreateSDLSurface(GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent));

    boost::intrusive_ptr < Resource<Surface> > backgroundResource = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("File dialog: " + GetName(), false, false);
    Surface *surface = backgroundResource->GetResource();

    surface->SetData(sdlSurface);

    background = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("File dialog: " + GetName(), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(background);
    background->SetImage(backgroundResource);

    backgroundResource->resourceMutex.lock();

    Uint32 color;
    color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 100);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    backgroundResource->resourceMutex.unlock();

    scene2D->AddObject(background);
    background->SetPosition(GetX(x1_percent), GetY(y1_percent));

    background->OnChange();

    filenameInput = new GuiTextInput(scene2D, "FileDialog_Filename", x1_percent, y1_percent, x2_percent, y1_percent + 5, initialFile);
    fileBrowser = new GuiFileBrowser(scene2D, "FileDialog_FileBrowser", x1_percent, y1_percent + 5, x2_percent, y2_percent - 5, directory);
    AddView(filenameInput);
    AddView(fileBrowser);
    AddView(new GuiButton(scene2D, GetName() + "_OkayButton", x1_percent, y2_percent - 5, (x1_percent + x2_percent) * 0.5, y2_percent, "Okay"));
    AddView(new GuiButton(scene2D, GetName() + "_CancelButton", (x1_percent + x2_percent) * 0.5, y2_percent - 5, x2_percent, y2_percent, "Cancel"));
  }

  std::string GuiFileDialog::GetFilename() {
    return fileBrowser->GetDirectory() + filenameInput->GetValue();
  }

  std::string GuiFileDialog::GetDirectory() {
    return fileBrowser->GetDirectory();
  }

  void GuiFileDialog::OnKey(int sdlkID) {
    //if (sdlkID == SDLK_ESCAPE) {
    //  parent->EmitSignal(this, (SDLKey)sdlkID);
    //} else {
      GuiView::OnKey(sdlkID);
    //}
  }

  void GuiFileDialog::EmitString(GuiView *sender, const std::string &someString) {
    if (sender == fileBrowser) {
      //printf("%s\n", someString.substr(someString.find_last_of("/"), std::string::npos).c_str());
      filenameInput->SetValue(someString);
    } else GuiView::EmitString(sender, someString);
  }

}
