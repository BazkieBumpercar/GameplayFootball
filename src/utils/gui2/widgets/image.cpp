// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "image.hpp"

#include "../windowmanager.hpp"

#include "SDL/SDL_gfxBlitFunc.h"
#include "SDL/SDL_rotozoom.h"

namespace blunted {

  Gui2Image::Gui2Image(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {
    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    image = windowManager->CreateImage2D(name, w, h, true);
  }

  Gui2Image::~Gui2Image() {
  }

  void Gui2Image::LoadImage(const std::string &filename) {
    SDL_Surface *imageSurfTmp = IMG_Load(filename.c_str());
    imageSource = windowManager->CreateImage2D(name + "source", imageSurfTmp->w, imageSurfTmp->h, false);

    boost::intrusive_ptr < Resource<Surface> > surfaceRes = imageSource->GetImage();
    surfaceRes->resourceMutex.lock();
    surfaceRes->GetResource()->SetData(imageSurfTmp);
    surfaceRes->resourceMutex.unlock();

    Redraw();
  }

  void Gui2Image::Redraw() {

    // paste source image onto screen image
    if (imageSource != boost::intrusive_ptr<Image2D>()) {

      // get image
      boost::intrusive_ptr < Resource<Surface> > surfaceRes = imageSource->GetImage();
      surfaceRes->resourceMutex.lock();

      SDL_Surface *imageSurfTmp = surfaceRes->GetResource()->GetData();

      int x, y, w, h;
      windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

      double zoomx;
      zoomx = (double)w / imageSurfTmp->w;
      double zoomy;
      zoomy = (double)h / imageSurfTmp->h;
      SDL_Surface *imageSurf = zoomSurface(imageSurfTmp, zoomx, zoomy, 1);
      //printf("actually resized to %i %i\n", imageSurf->w, imageSurf->h);

      surfaceRes->resourceMutex.unlock();

      surfaceRes = image->GetImage();
      surfaceRes->resourceMutex.lock();

      surfaceRes->GetResource()->SetData(imageSurf);

      surfaceRes->resourceMutex.unlock();

      image->OnChange();
    }

  }

  void Gui2Image::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
    target.push_back(image);
    Gui2View::GetImages(target);
  }

  void Gui2Image::SetSize(float new_width_percent, float new_height_percent) {
    Gui2View::SetSize(new_width_percent, new_height_percent);

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    //printf("resized to %i %i\n", w, h);

    image->Resize(w, h);
    Redraw();
  }

  void Gui2Image::SetZoom(float zoomx, float zoomy) {

    // paste source image onto screen image
    if (imageSource != boost::intrusive_ptr<Image2D>()) {

      // get image
      boost::intrusive_ptr < Resource<Surface> > surfaceRes = imageSource->GetImage();
      surfaceRes->resourceMutex.lock();

      SDL_Surface *imageSurfTmp = surfaceRes->GetResource()->GetData();

      int x, y, w, h;
      windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

      double zoomx1;
      zoomx1 = (double)w / imageSurfTmp->w * zoomx;
      double zoomy1;
      zoomy1 = (double)h / imageSurfTmp->h * zoomy;
      SDL_Surface *imageSurf = zoomSurface(imageSurfTmp, zoomx1, zoomy1, 1);
      //printf("actually resized to %i %i\n", imageSurf->w, imageSurf->h);

      surfaceRes->resourceMutex.unlock();

      image->DrawRectangle(0, 0, w, h, Vector3(0, 0, 0), 0);

      surfaceRes = image->GetImage();
      surfaceRes->resourceMutex.lock();

      SDL_Surface *surface = surfaceRes->GetResource()->GetData();
      SDL_Rect rect;
      rect.x = w * 0.5 - imageSurf->w * 0.5;
      rect.y = h * 0.5 - imageSurf->h * 0.5;
      SDL_gfxBlitRGBA(imageSurf, NULL, surface, &rect);

      surfaceRes->resourceMutex.unlock();

      SDL_FreeSurface(imageSurf);

      image->OnChange();
    }
  }

}
