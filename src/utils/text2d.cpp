// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

/* DEPRECATED CLASS */

#include "text2d.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "managers/systemmanager.hpp"

#include "scene/iscene.hpp"
#include "scene/objectfactory.hpp"

#include "base/sdl_surface.hpp"

namespace blunted {

  Text2D::Text2D(boost::shared_ptr<Scene2D> scene2D, const std::string &fontFile, int pts) : count(0), scene2D(scene2D) {
    assert(1 == 2); // todo: error, this file is deprecated, can't do ttf_init for one, already done that. check if this file/this whole old 2d/gui system is used anyway, and if not, remove from project
    TTF_Init();
    font = TTF_OpenFont(fontFile.c_str(), pts);
  }

  Text2D::~Text2D() {
    TTF_CloseFont(font);
    TTF_Quit();
  }


  boost::intrusive_ptr<Image2D> Text2D::Create(int width) {
    std::string empty;
    for (int i = 0; i < width; i++) empty.append("x");

    // this is to get width/height
    SDL_Surface *sdlText = RenderTextSurface(empty, Vector3(0, 0, 0));

    // create power-of-2 version
    SDL_Surface *sdlTextPow2 = SDL_CreateRGBSurface(sdlText->flags | SDL_SRCALPHA, pot(sdlText->w), pot(sdlText->h), 32, r_mask, g_mask, b_mask, a_mask);

    //printf("address old: %i\n", sdlTextPow2);

    // test
    //sdl_putpixel(sdlTextPow2, 1, 1, SDL_MapRGB(sdlTextPow2->format, 255, 255, 255));
    //sdl_putpixel(sdlTextPow2, 4, 4, SDL_MapRGB(sdlTextPow2->format, 255, 155, 155));

    SDL_FreeSurface(sdlText);

// todo: unique name?
    boost::intrusive_ptr < Resource<Surface> > surface = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("text2D #" + int_to_str(count), false, false);
    surface->GetResource()->SetData(sdlTextPow2);

// todo: unique name?
    boost::intrusive_ptr<Image2D> image = static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("text2D #" + int_to_str(count), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(image);

    image->SetImage(surface);

    count++;

    return image;
  }

  void Text2D::SetText(boost::intrusive_ptr<Image2D> image, const std::string &text, const Vector3 &color) const {
    SDL_Surface *sdlText = RenderTextSurface(text, color);
    assert(sdlText);

    // keep the same image, but change its surface
    boost::intrusive_ptr < Resource<Surface> > surface = image->GetImage();

    // convert to power-of-2 sized surface

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = sdlText->w;
    rect.h = sdlText->h;

    image->subjectMutex.lock();
    surface->resourceMutex.lock();

    SDL_Surface *sdlTextPow2 = surface->GetResource()->GetData();

    //printf("address new: %i\n", sdlTextPow2);
    assert(sdlText);
    assert(sdlTextPow2);
    // no clipping
    if (rect.w > sdlTextPow2->w) rect.w = sdlTextPow2->w;
    //printf("%i %i %i %i\n", rect.x, rect.y, sdlTextPow2->w, sdlTextPow2->h);
    SDL_Rect bla;
    bla.x = 0;
    bla.y = 0;
    bla.w = sdlTextPow2->w;
    bla.h = sdlTextPow2->h;

    SDL_SetAlpha(sdlText, 0, SDL_ALPHA_OPAQUE);
    SDL_FillRect(sdlTextPow2, &bla, SDL_MapRGBA(sdlTextPow2->format, 0, 0, 0, 0));
    SDL_BlitSurface(sdlText, &rect, sdlTextPow2, &bla);
    SDL_FreeSurface(sdlText);
    //sdl_flipsurface(sdlTextPow2);
    SDL_SetAlpha(sdlTextPow2, SDL_SRCALPHA, 128);

    surface->resourceMutex.unlock();
    image->subjectMutex.unlock();

    image->OnChange();
  }

  SDL_Surface *Text2D::RenderTextSurface(const std::string &text, const Vector3 &color) const {
    SDL_Color sdlColor = { int(floor(color.coords[0] * 255.0)), int(floor(color.coords[1] * 255.0)), int(floor(color.coords[2] * 255.0)), 0 };
    SDL_Color sdlBackColor = { 0, 0, 0, 0 };

    SDL_Surface *sdlText = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
    //printf("Oh My Goodness, an error : %s\n", TTF_GetError());
    //SDL_Surface *sdlText = TTF_RenderText_Solid(font, text.c_str(), sdlColor);

    return sdlText;
  }

}
