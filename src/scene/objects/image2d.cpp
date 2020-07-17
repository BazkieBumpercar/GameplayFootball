// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "image2d.hpp"

#include "base/log.hpp"

#include "systems/isystemobject.hpp"

#include "SDL/SDL_gfxBlitFunc.h"

namespace blunted {

  Image2D::Image2D(std::string name) : Object(name, e_ObjectType_Image2D) {
    //printf("CREATING IMAGE\n");
  }

  Image2D::~Image2D() {
    //printf("DELETING IMAGE\n");
  }

  void Image2D::Exit() { // ATOMIC
    //printf("EXITING IMAGE\n");

    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers.at(i).get());
      image2DInterpreter->OnUnload();
    }

    Object::Exit();

    if (image) image.reset();

    subjectMutex.unlock();
  }

  // todo: rename to SetSurface and GetSurface?
  void Image2D::SetImage(boost::intrusive_ptr < Resource<Surface> > image) {
    subjectMutex.lock();

    this->image = image;

    image->resourceMutex.lock();

    position[0] = 0;
    position[1] = 0;
    size[0] = image->GetResource()->GetData()->w;
    size[1] = image->GetResource()->GetData()->h;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers.at(i).get());
      image2DInterpreter->OnLoad(image);
    }

    image->resourceMutex.unlock();

    subjectMutex.unlock();
  }

  boost::intrusive_ptr < Resource<Surface> > Image2D::GetImage() {
    // todo: this mutex lock makes little sense or so it seems. will have to force users to lock from outside this function
    boost::mutex::scoped_lock blah(subjectMutex);
    return image;
  }

  void Image2D::SetPosition(int x, int y) {
    subjectMutex.lock();

    position[0] = x;
    position[1] = y;

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers.at(i).get());
      image2DInterpreter->OnMove(x, y);
    }

    subjectMutex.unlock();
  }

  void Image2D::SetPosition(const Vector3 &newPosition, bool updateSpatialData) {
    SetPosition(int(floor(newPosition.coords[0])), int(floor(newPosition.coords[1])));
  }

  Vector3 Image2D::GetPosition() const {
    subjectMutex.lock();
    Vector3 tmp(position[0], position[1], 0);
    subjectMutex.unlock();

    return tmp;
  }

  Vector3 Image2D::GetSize() const {
    subjectMutex.lock();
    Vector3 tmp(size[0], size[1], 0);
    subjectMutex.unlock();

    return tmp;
  }

  void Image2D::PutPixel(int x, int y, const Vector3 &color, int alpha) {
    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();
    SDL_LockSurface(surface);

    Uint32 color32;
    if (surface->flags && SDL_SRCALPHA) color32 = SDL_MapRGBA(surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])), alpha);
                                   else color32 = SDL_MapRGB( surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])));

    assert(x < surface->w && y < surface->h);
    sdl_putpixel(surface, x, y, color32);

    SDL_UnlockSurface(surface);

    image->resourceMutex.unlock();
    subjectMutex.unlock();
  }

  Vector3 Image2D::GetPixel(int x, int y) const {
    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();
    Uint32 color = sdl_getpixel(surface, x, y);
    Uint8 r, g, b;
    SDL_GetRGB(color, surface->format, &r, &g, &b);
    Vector3 returnValue(r, g, b);

    image->resourceMutex.unlock();
    subjectMutex.unlock();

    return returnValue;
  }

  void Image2D::Blur(int radius) const {
    // todo: implement alpha
    // todo: why is this so slow? or is it the put/getpixel code?

    if (radius < 1) radius = 1;

    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();
    SDL_Surface *target = CreateSDLSurface(surface->w, surface->h);

    SDL_LockSurface(surface); // added after latest test

    for (int x = 0; x < surface->w; x++) {
      for (int y = 0; y < surface->h; y++) {

        Vector3 color;
        int counter = 0;
        for (int xsub = -radius; xsub < radius + 1; xsub++) {
          for (int ysub = -radius; ysub < radius + 1; ysub++) {

            if (x + xsub >= 0 && x + xsub < surface->w &&
                y + ysub >= 0 && y + ysub < surface->h) {
              Uint32 color32 = sdl_getpixel(surface, x + xsub, y + ysub);
              Uint8 r, g, b;
              SDL_GetRGB(color32, surface->format, &r, &g, &b);
              color += Vector3(r, g, b);
              counter++;
            }

          }
        }

        color = color / counter;

        Uint32 color32;
        if (surface->flags && SDL_SRCALPHA) color32 = SDL_MapRGBA(surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])), 255);
                                       else color32 = SDL_MapRGB(surface->format,  int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])));
        sdl_putpixel(target, x, y, color32);
      }

    }

    SDL_UnlockSurface(surface);

    image->GetResource()->SetData(target);

    image->resourceMutex.unlock();
    subjectMutex.unlock();
  }

  void Image2D::DrawLine(const Line &line, const Vector3 &color, int alpha) {
    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();
    SDL_LockSurface(surface);

    Uint32 color32;
    if (surface->flags && SDL_SRCALPHA) color32 = SDL_MapRGBA(surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])), alpha);
                                   else color32 = SDL_MapRGB(surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])));

    sdl_line(surface, line.GetVertex(0).coords[0], line.GetVertex(0).coords[1], line.GetVertex(1).coords[0], line.GetVertex(1).coords[1], color32);

    SDL_UnlockSurface(surface);

    image->resourceMutex.unlock();
    subjectMutex.unlock();

    //OnChange();
  }

  void Image2D::DrawTriangle(const Triangle &triangle, const Vector3 &color, int alpha) {
    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();
    SDL_LockSurface(surface);

    sdl_triangle_filled(surface, triangle, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])));

    SDL_UnlockSurface(surface);

    image->resourceMutex.unlock();
    subjectMutex.unlock();

    //OnChange();
  }

  void Image2D::DrawRectangle(int x, int y, int w, int h, const Vector3 &color, int alpha) {
    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();
    //SDL_LockSurface(surface);

    Uint32 color32;
    if (surface->flags && SDL_SRCALPHA) color32 = SDL_MapRGBA(surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])), alpha);
                                   else color32 = SDL_MapRGB(surface->format, int(floor(color.coords[0])), int(floor(color.coords[1])), int(floor(color.coords[2])));

    sdl_rectangle_filled(surface, x, y, w, h, color32);

    //SDL_UnlockSurface(surface);

    image->resourceMutex.unlock();
    subjectMutex.unlock();

    //OnChange();
  }

  void Image2D::SetAlpha(float alpha) {
    subjectMutex.lock();
    image->resourceMutex.lock();

    image->GetResource()->SetAlpha(alpha);

    image->resourceMutex.unlock();
    subjectMutex.unlock();

    //OnChange();
  }

  void Image2D::Resize(int w, int h) {
    subjectMutex.lock();
    image->resourceMutex.lock();

    image->GetResource()->Resize(w, h);

    image->resourceMutex.unlock();

    size[0] = w;
    size[1] = h;

    subjectMutex.unlock();
  }

  void Image2D::DrawSimpleText(const std::string &caption, int x, int y, TTF_Font *font, const Vector3 &color, int alpha) { // todo: alpha doesn't work properly yet

    SDL_Color sdlColor = { int(color.coords[0]), int(color.coords[1]), int(color.coords[2]), 0 };
    SDL_Surface *sdlText = TTF_RenderUTF8_Blended(font, caption.c_str(), sdlColor);
    if (alpha != 255) sdl_setsurfacealpha(sdlText, alpha);

    assert(sdlText);


    subjectMutex.lock();
    image->resourceMutex.lock();

    SDL_Surface *surface = image->GetResource()->GetData();

    SDL_LockSurface(surface);

    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = surface->w - x;
    dstRect.h = surface->h - y;
    sdl_alphablit(sdlText, NULL, surface, &dstRect);

    SDL_UnlockSurface(surface);

    image->resourceMutex.unlock();
    subjectMutex.unlock();


    SDL_FreeSurface(sdlText);
  }

  void Image2D::Poke(e_SystemType targetSystemType) {
    subjectMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers.at(i).get());
      if (image2DInterpreter->GetSystemType() == targetSystemType) image2DInterpreter->OnPoke();
    }

    subjectMutex.unlock();
  }


  // events

  void Image2D::OnChange() {
    subjectMutex.lock();
    image->resourceMutex.lock();

    int observersSize = observers.size();
    for (int i = 0; i < observersSize; i++) {
      IImage2DInterpreter *image2DInterpreter = static_cast<IImage2DInterpreter*>(observers.at(i).get());
      image2DInterpreter->OnChange(image);
    }

    image->resourceMutex.unlock();
    subjectMutex.unlock();
  }

}
