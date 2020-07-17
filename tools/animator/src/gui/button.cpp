// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "button.hpp"

#include "base/sdl_surface.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "scene/objectfactory.hpp"

namespace blunted {

  GuiButton::GuiButton(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &caption) : GuiView(scene2D, name, x1_percent, y1_percent, x2_percent, y2_percent), caption(caption) {
  }

  GuiButton::~GuiButton() {
    scene2D->DeleteObject(button);
    button.reset();
  }


  void GuiButton::Init() {
    SDL_Surface *sdlSurface = CreateSDLSurface(GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent));

    boost::intrusive_ptr < Resource<Surface> > buttonResource = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("button: " + GetName(), false, false);
    Surface *surface = buttonResource->GetResource();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 200);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    surface->SetData(sdlSurface);

    button = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("button: " + GetName(), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(button);
    button->SetImage(buttonResource);

    scene2D->AddObject(button);
    button->SetPosition(GetX(x1_percent), GetY(y1_percent));

    guiCaption = new GuiCaption(scene2D, GetName(), x1_percent, y1_percent, x2_percent, y2_percent, caption);
    this->AddView(guiCaption);
  }


  void GuiButton::OnFocus() {
    boost::intrusive_ptr < Resource<Surface> > buttonResource = button->GetImage();
    buttonResource->resourceMutex.lock();

    Surface *surface = buttonResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 80, 80, 128, 200);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    buttonResource->resourceMutex.unlock();

    button->OnChange();
  }

  void GuiButton::OnLoseFocus() {
    boost::intrusive_ptr < Resource<Surface> > buttonResource = button->GetImage();
    buttonResource->resourceMutex.lock();

    Surface *surface = buttonResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 200);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    buttonResource->resourceMutex.unlock();

    button->OnChange();
  }

  void GuiButton::OnKey(int sdlkID) {
    if (sdlkID == SDLK_UP || sdlkID == SDLK_LEFT) parent->SwitchFocus(-1);
    if (sdlkID == SDLK_DOWN || sdlkID == SDLK_RIGHT) parent->SwitchFocus(1);

    if (sdlkID == SDLK_RETURN) EmitSignal(this, SDLK_RETURN);
  }

}
