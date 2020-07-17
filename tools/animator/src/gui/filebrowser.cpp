// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "filebrowser.hpp"

#include "base/sdl_surface.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "scene/objectfactory.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"


// WARNING!!! DISCLAIMER

// THIS PROBABLY IS THE NASTIEST CODE I'VE EVER WRITTEN

// but hey, it works ;)


namespace fs = boost::filesystem;

namespace blunted {

  GuiFileBrowser::GuiFileBrowser(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &directory) : GuiView(scene2D, name, x1_percent, y1_percent, x2_percent, y2_percent), directory(directory) {
    active = false;
    selected = 0;
    offset = 0;
  }

  GuiFileBrowser::~GuiFileBrowser() {
    scene2D->DeleteObject(fileBrowser);
    fileBrowser.reset();
  }


  void GuiFileBrowser::Init() {
    SDL_Surface *sdlSurface = CreateSDLSurface(GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent));

    boost::intrusive_ptr < Resource<Surface> > fileBrowserResource = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("fileBrowser: " + GetName(), false, false);
    Surface *surface = fileBrowserResource->GetResource();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 200);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    surface->SetData(sdlSurface);

    fileBrowser = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("fileBrowser: " + GetName(), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(fileBrowser);
    fileBrowser->SetImage(fileBrowserResource);

    scene2D->AddObject(fileBrowser);
    fileBrowser->SetPosition(GetX(x1_percent), GetY(y1_percent));

    for (int i = 0; i < (y2_percent - y1_percent) / 5; i++) {
      GuiCaption *text = new GuiCaption(scene2D, GetName() + int_to_str(i), x1_percent, y1_percent + i * 5, x2_percent, y1_percent + i * 5 + 5, "");
      this->AddView(text);
      fileCaptions.push_back(text);
    }

    GetDirectoryContents();
    Redraw();
  }


  void GuiFileBrowser::Redraw() {
    for (int i = 0; i < (signed int)fileCaptions.size(); i++) {
      if (i + offset < (signed int)dirContents.size()) {
        fileCaptions.at(i)->Set(dirContents.at(i + offset).name);
      } else {
        fileCaptions.at(i)->Set("");
      }
    }

    // select bar
    if (active) {
      ChangeColor(0, 0, 0, 200);
      DrawSelected();
    }
  }

  void GuiFileBrowser::GetDirectoryContents() {
    dirContents.clear();

    DirEntry entry;
    entry.name = "../";
    entry.type = e_DirEntryType_Directory;
    dirContents.push_back(entry);

    char *cstr = new char[directory.length() + 1];
    std::strcpy(cstr, directory.c_str());
    fs::path path = fs::system_complete(fs::path(cstr));
    delete [] cstr;
    if (!fs::exists(path) || !fs::is_directory(path)) Log(e_Error, "GuiFileBrowser", "GetDirectoryContents", "Could not open directory " + directory + " for reading");

    fs::directory_iterator dirIter(path);
    fs::directory_iterator endIter;
    while (dirIter != endIter) {
      DirEntry entry;
      //char *cstr = new char[dirIter->path().filename().length() + 1];
      //std::strcpy(cstr, dirIter->path().filename().c_str());
      entry.name = dirIter->path().filename().string();
      //delete [] cstr;
      if (is_directory(dirIter->status())) entry.name.append("/");
      entry.type = is_directory(dirIter->status()) ? e_DirEntryType_Directory : e_DirEntryType_File;
      dirContents.push_back(entry);

      dirIter++;
    }

    std::sort(dirContents.begin(), dirContents.end());

/* old fashioned way
    dir = opendir(directory.c_str());
    if (dir != NULL) {
      while (entry = readdir(dir)) {
        DirEntry dEntry;
        dEntry.name = entry->d_name;
        if (entry->d_type != isFile) dEntry.type = e_DirEntryType_Directory; else
                                     dEntry.type = e_DirEntryType_File;
        dirContents.push_back(dEntry);
      }
      closedir(dir);
    } else {
    }
*/

  }

  void GuiFileBrowser::ChangeColor(int r, int g, int b, int a) {
    boost::intrusive_ptr < Resource<Surface> > fileBrowserResource = fileBrowser->GetImage();
    fileBrowserResource->resourceMutex.lock();

    Surface *surface = fileBrowserResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, r, g, b, a);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    fileBrowserResource->resourceMutex.unlock();

    fileBrowser->OnChange();
  }

  void GuiFileBrowser::DrawSelected() {
    boost::intrusive_ptr < Resource<Surface> > fileBrowserResource = fileBrowser->GetImage();
    fileBrowserResource->resourceMutex.lock();

    Surface *surface = fileBrowserResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 128, 128, 128, 200);
    sdl_rectangle_filled(sdlSurface, 0, (GetY(y2_percent) - GetY(y1_percent)) / fileCaptions.size() * (selected - offset), GetX(x2_percent) - GetX(x1_percent), (GetY(y2_percent) - GetY(y1_percent)) / fileCaptions.size(), color);
    fileBrowserResource->resourceMutex.unlock();

    fileBrowser->OnChange();
  }

  void GuiFileBrowser::OnFocus() {
    ChangeColor(80, 80, 128, 200);
  }

  void GuiFileBrowser::OnLoseFocus() {
    ChangeColor(0, 0, 0, 200);
  }

  std::string GuiFileBrowser::GetDirectory() {
    if (directory.substr(directory.length() - 1, 1) != "/") directory += "/";
    return directory;
  }

  void GuiFileBrowser::OnKey(int sdlkID) {

    if (active && sdlkID == SDLK_ESCAPE) {
      active = false;
      OnFocus();
    }

    if (!active) {
      if (sdlkID == SDLK_UP || sdlkID == SDLK_LEFT) parent->SwitchFocus(-1);
      if (sdlkID == SDLK_DOWN || sdlkID == SDLK_RIGHT) parent->SwitchFocus(1);
    } else {
      if (sdlkID == SDLK_UP) {
        if (selected > 0) selected--;
        if (selected < offset) offset--;
        Redraw();
      }
      if (sdlkID == SDLK_DOWN) {
        if (selected < (int)dirContents.size() - 1) selected++;
        if (selected > (int)fileCaptions.size() - 1 + offset) offset++;
        Redraw();
      }
    }

    if (sdlkID == SDLK_RETURN) {
      if (!active) {
        active = true;
        Redraw();
      } else {
        if (dirContents.at(selected).type == e_DirEntryType_Directory) {
          if (dirContents.at(selected).name != "../") {
            if (directory.substr(directory.length() - 1, 1) != "/") directory += "/";
            directory += dirContents.at(selected).name;
          } else {
            if (directory.substr(directory.length() - 1, 1) != "/") directory += "/";
            directory = directory.substr(0, directory.find_last_of("/"));
            directory = directory.substr(0, directory.find_last_of("/"));
            if (directory.length() == 0) directory = "/";
          }
          selected = 0;
          offset = 0;
          GetDirectoryContents();
          Redraw();
        } else { // file
          EmitString(this, dirContents.at(selected).name);
          active = false;
          Redraw();
          parent->SwitchFocus(-1);
        }
        //OnFocus();
      }
    }

    if (active) {
    }
  }

}
