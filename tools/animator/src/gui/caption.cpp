// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "caption.hpp"

#include "base/sdl_surface.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "scene/objectfactory.hpp"

namespace blunted {

  GuiCaption::GuiCaption(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent, const std::string &caption) : GuiView(scene2D, name, x1_percent, y1_percent, x2_percent, y2_percent), caption(caption) {
  }

  GuiCaption::~GuiCaption() {
    scene2D->DeleteObject(captionImage);
    captionImage.reset();

    TTF_CloseFont(font);
    TTF_Quit();
  }


  void GuiCaption::Init() {
    TTF_Init();
    font = TTF_OpenFont("media/fonts/SF Old Republic v2.0/SF Old Republic Bold.ttf", int((GetY(y2_percent) - GetY(y1_percent)) * 0.7));
    if (!font) Log(e_FatalError, "GuiCaption", "Init", "Font not found!");

    SDL_Surface *sdlSurface = CreateSDLSurface(GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent));

// todo: unique names
    captionResource = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("caption: " + GetName(), false, false);
    Surface *surface = captionResource->GetResource();

    surface->SetData(sdlSurface);

// todo: unique names
    captionImage = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("caption: " + GetName(), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(captionImage);
    captionImage->SetImage(captionResource);

    Set(caption);

    scene2D->AddObject(captionImage);
    captionImage->SetPosition(GetX(x1_percent), GetY(y1_percent));
  }

  void GuiCaption::Set(const std::string &value) {
    //if (caption == value) return;
    caption = value;

    if (caption == "") caption = " ";

    SDL_Color sdlColor = { 255, 255, 255, 0 };
    SDL_Color sdlBackColor = { 0, 0, 0, 0 };

    //TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    SDL_Surface *sdlText = TTF_RenderText_Blended(font, caption.c_str(), sdlColor);

    captionResource->resourceMutex.lock();
    Surface *surface = captionResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();
    Uint32 color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 0);
    sdl_rectangle_filled(sdlSurface, 0, 0, GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent), color);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = sdlText->w;
    rect.h = sdlText->h;

    // no clipping
    if (rect.w > sdlSurface->w) rect.w = sdlSurface->w;
    //printf("%i %i %i %i\n", rect.x, rect.y, sdlTextPow2->w, sdlTextPow2->h);
    SDL_Rect bla;
    bla.x = int(sdlSurface->w / 2.0 - sdlText->w / 2.0);
    bla.y = int(sdlSurface->h / 2.0 - sdlText->h / 2.0) + int((GetY(y2_percent) - GetY(y1_percent)) * 0.05);
    bla.w = sdlSurface->w;
    bla.h = sdlSurface->h;

    SDL_SetAlpha(sdlText, 0, SDL_ALPHA_OPAQUE);
    SDL_FillRect(sdlSurface, &bla, SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 0));
    SDL_BlitSurface(sdlText, &rect, sdlSurface, &bla);
    SDL_FreeSurface(sdlText);
    SDL_SetAlpha(sdlSurface, SDL_SRCALPHA, 128);
    captionResource->resourceMutex.unlock();

    captionImage->OnChange();
  }

  void GuiCaption::OnFocus() {
  }

  void GuiCaption::OnLoseFocus() {
  }

  void GuiCaption::OnKey(int sdlkID) {
    // quick hax
    if (sdlkID == SDLK_UP) {
      parent->SwitchFocus(-1);
    }
    else if (sdlkID == SDLK_DOWN) {
      parent->SwitchFocus(1);
    }
  }

}
