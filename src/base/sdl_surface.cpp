// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "sdl_surface.hpp"
#include "geometry/triangle.hpp"

namespace blunted {

  SDL_Surface *CreateSDLSurface(int width, int height) {
    return SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL, width, height, 32, r_mask, g_mask, b_mask, a_mask);
  }

  void sdl_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {

    int bpp = surface->format->BytesPerPixel;
    // Here p is the address to the pixel we want to set
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
  }

  Uint32 sdl_getpixel(const SDL_Surface *surface, int x, int y) {

    int bpp = surface->format->BytesPerPixel;
    // Here p is the address to the pixel we want to retrieve
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0; // shouldn't happen, but avoids warnings
    }
  }

  void sdl_line(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color) {
    sge_Line(surface, x1, y1, x2, y2, color);
    sge_AALine(surface, x1, y1, x2, y2, color);
  }

  void sdl_triangle_filled(SDL_Surface *surface, const Triangle &triangle, Uint8 r, Uint8 g, Uint8 b) {
    // uses only the x, y part of the triangle

    // sge: inverse-wounded triangles won't be drawn, so draw twice, both ways round. lame fix :D

    sge_FilledTrigon(surface, int(triangle.GetVertex(0).coords[0]), int(triangle.GetVertex(0).coords[1]),
                              int(triangle.GetVertex(1).coords[0]), int(triangle.GetVertex(1).coords[1]),
                              int(triangle.GetVertex(2).coords[0]), int(triangle.GetVertex(2).coords[1]),
                              r, g, b);

    sge_FilledTrigon(surface, int(triangle.GetVertex(1).coords[0]), int(triangle.GetVertex(1).coords[1]),
                              int(triangle.GetVertex(0).coords[0]), int(triangle.GetVertex(0).coords[1]),
                              int(triangle.GetVertex(2).coords[0]), int(triangle.GetVertex(2).coords[1]),
                              r, g, b);
  }

  void sdl_rectangle_filled(SDL_Surface *surface, int x, int y, int width, int height, Uint32 color) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    SDL_FillRect(surface, &rect, color);
  }

  void sdl_alphablit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
    // http://listas.apesol.org/pipermail/sdl-libsdl.org/2002-August/029180.html
    SDL_LockSurface(src);
    SDL_LockSurface(dst);

    int x, y;

    Uint8 *pdst;
    Uint8 *psrc;

    SDL_Rect srect;
    if (srcrect) {
      srect = SDL_Rect(*srcrect);
    } else {
      srect.x = 0;
      srect.y = 0;
      srect.w = src->w;
      srect.h = src->h;
    }

    SDL_Rect drect;
    if (dstrect) {
      drect = SDL_Rect(*dstrect);
    } else {
      drect.x = 0;
      drect.y = 0;
      drect.w = dst->w;
      drect.h = dst->h;
    }

    /* loop through all pixels */
    for (y = 0; y < (srect.h < drect.h ? srect.h : drect.h); y++) {
      for (x = 0; x < (srect.w < drect.w ? srect.w : drect.w); x++) {
        psrc = (Uint8 *)src->pixels
                    + (y + srect.y)*src->pitch
                          + (x + srect.x)*src->format->BytesPerPixel;

        /* copy value from source to destination */
        pdst = (Uint8 *)dst->pixels
                  + (y + drect.y)*dst->pitch
                        + (x + drect.x)*dst->format->BytesPerPixel;

        float srcOpacity = psrc[3] / 256.0f;
        float dstOpacity = pdst[3] / 256.0f;
        float srcBias = srcOpacity;
        pdst[0] = psrc[0] * srcBias + pdst[0] * (1.0f - srcBias);
        pdst[1] = psrc[1] * srcBias + pdst[1] * (1.0f - srcBias);
        pdst[2] = psrc[2] * srcBias + pdst[2] * (1.0f - srcBias);
        pdst[3] = psrc[3] * srcBias + pdst[3] * (1.0f - srcBias);

      }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dst);
  }

  void sdl_flipsurface(SDL_Surface *surface) {
    // tnx Sebastian Beschke!
    SDL_Surface *result = SDL_CreateRGBSurface(surface->flags, surface->w, surface->h,
      surface->format->BytesPerPixel * 8, surface->format->Rmask, surface->format->Gmask,
      surface->format->Bmask, surface->format->Amask);

    Uint8 *pixels = (Uint8*)surface->pixels;
    Uint8 *rpixels = (Uint8*)result->pixels;
    Uint32 pitch = surface->pitch;
    Uint32 pxlength = pitch * surface->h;

    for (int line = 0; line < surface->h; ++line) {
      int pos = line * pitch;
      memcpy(&rpixels[pos], &pixels[pxlength - pos - pitch], pitch);
    }

    memcpy(&pixels[0], &rpixels[0], pxlength);
    SDL_FreeSurface(result);
  }

  void sdl_setsurfacealpha(SDL_Surface *surface, int alpha) {
    // original from: Warren Schwader
    // sdl at libsdl.org

    // the surface width and height
    int width = surface->w;
    int height = surface->h;

    // the pitch of the surface. Used to determine where the next vertical line in pixels starts
    int pitch = surface->pitch;
    int bpp = surface->format->BytesPerPixel;

    // NOTE: since we are only modifying the alpha bytes we could simply read only that byte and then add 4 to get
    // to the next alpha byte. That approach depends on the endianess of the alpha byte and also the 32 bit pixel depth
    // so if those things change then this code will need changing too. but we already rely on 32 bpp

    SDL_LockSurface(surface);
    for (int h = 0; h < height; h++) {

      for (int w = 0; w < width; w++) {
        Uint8 *p = (Uint8 *)surface->pixels + h * pitch + w * bpp;
        // todo: endiannes!! *update: seems to work as is.. maybe alpha is always in index 3 and only the colors are inverted?
        p[3] = clamp((p[3] / 256.0f) * (alpha / 256.0f) * 256, 0, 255);
      }

    }
    SDL_UnlockSurface(surface);

  }

}
