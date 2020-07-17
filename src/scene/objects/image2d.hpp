// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT_IMAGE2D
#define _HPP_OBJECT_IMAGE2D

#include "defines.hpp"
#include "scene/object.hpp"
#include "scene/resources/surface.hpp"
#include "types/interpreter.hpp"
#include "base/math/vector3.hpp"
#include "base/geometry/line.hpp"

#include <SDL/SDL_ttf.h>

namespace blunted {

  class Image2D : public Object {

    public:
      Image2D(std::string name);
      virtual ~Image2D();

      virtual void Exit();

      void SetImage(boost::intrusive_ptr < Resource<Surface> > image);
      boost::intrusive_ptr < Resource<Surface> > GetImage();

      void SetPosition(int x, int y);
      virtual void SetPosition(const Vector3 &newPosition, bool updateSpatialData = true);
      virtual Vector3 GetPosition() const;
      Vector3 GetSize() const;
      void PutPixel(int x, int y, const Vector3 &color, int alpha = 255);
      Vector3 GetPixel(int x, int y) const;
      void Blur(int radius) const;
      void DrawRectangle(int x, int y, int w, int h, const Vector3 &color, int alpha = 255);
      void DrawLine(const Line &line, const Vector3 &color, int alpha = 255);
      void DrawTriangle(const Triangle &triangle, const Vector3 &color, int alpha = 255);
      void SetAlpha(float alpha);
      void Resize(int w, int h);
      void DrawSimpleText(const std::string &caption, int x, int y, TTF_Font *font, const Vector3 &color, int alpha = 255);

      virtual void Poke(e_SystemType targetSystemType);

      void OnChange();

    protected:
      int position[2];
      int size[2];
      boost::intrusive_ptr < Resource<Surface> > image;

  };

  class IImage2DInterpreter : public Interpreter {

    public:
      virtual void OnLoad(boost::intrusive_ptr < Resource<Surface> > surface) = 0;
      virtual void OnUnload() = 0;
      virtual void OnChange(boost::intrusive_ptr < Resource<Surface> > surface) = 0;
      virtual void OnMove(int x, int y) = 0;
      virtual void OnPoke() = 0;

    protected:

  };

}

#endif
