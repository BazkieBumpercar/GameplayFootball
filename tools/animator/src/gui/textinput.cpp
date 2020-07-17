// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "textinput.hpp"

#include "base/sdl_surface.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "scene/objectfactory.hpp"

namespace blunted {

  GuiTextInput::GuiTextInput(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &initialValue) : GuiView(scene2D, name, x1_percent, y1_percent, x2_percent, y2_percent), value(initialValue) {
    active = false;
  }

  GuiTextInput::~GuiTextInput() {
    scene2D->DeleteObject(textInput);
    textInput.reset();
  }


  void GuiTextInput::Init() {
    SDL_Surface *sdlSurface = CreateSDLSurface(GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent));

    boost::intrusive_ptr < Resource<Surface> > textInputResource = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("textInput: " + GetName(), false, false);
    Surface *surface = textInputResource->GetResource();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 200);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    surface->SetData(sdlSurface);

    textInput = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("textInput: " + GetName(), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(textInput);
    textInput->SetImage(textInputResource);

    scene2D->AddObject(textInput);
    textInput->SetPosition(GetX(x1_percent), GetY(y1_percent));

    text = new GuiCaption(scene2D, GetName(), x1_percent, y1_percent, x2_percent, y2_percent, value);
    this->AddView(text);
  }


  void GuiTextInput::ChangeColor(int r, int g, int b, int a) {
    boost::intrusive_ptr < Resource<Surface> > textInputResource = textInput->GetImage();
    textInputResource->resourceMutex.lock();

    Surface *surface = textInputResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, r, g, b, a);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    textInputResource->resourceMutex.unlock();

    textInput->OnChange();
  }

  void GuiTextInput::OnFocus() {
    ChangeColor(80, 80, 128, 200);
  }

  void GuiTextInput::OnLoseFocus() {
    ChangeColor(0, 0, 0, 200);
  }

  void GuiTextInput::SetValue(const std::string &newValue) {
    text->Set(newValue);
    value = newValue;
  }

  std::string GuiTextInput::GetValue() {
    return value;
  }

  void GuiTextInput::OnKey(int sdlkID) {

    if (!active) {
      if (sdlkID == SDLK_UP) parent->SwitchFocus(-1);
      if (sdlkID == SDLK_DOWN) parent->SwitchFocus(1);
    }

    if (sdlkID == SDLK_RETURN) {
      if (!active) {
        active = true;
        ChangeColor(128, 128, 128, 200);
      } else {
        active = false;
        OnFocus();
      }
    }

    if (active) {
      if ((sdlkID >= SDLK_a && sdlkID <= SDLK_z) || (sdlkID >= SDLK_0 && sdlkID <= SDLK_9) || sdlkID == SDLK_UNDERSCORE || sdlkID == SDLK_PERIOD) {
        value.append((const char*)&sdlkID);
        text->Set(value);
      }
      if (sdlkID == SDLK_BACKSPACE) {
        if (value.length() > 0) {
          value = value.substr(0, value.length() - 1);
          text->Set(value);
        }
      }
    }
  }

}
