// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_LINE
#define _HPP_LINE

#include "base/math/vector3.hpp"

namespace blunted {

  class AABB;

  class Line {

    public:
      Line();
      Line(const Vector3 vec1, const Vector3 vec2);
      virtual ~Line();

      virtual void SetVertex(unsigned char pos, const real x, const real y, const real z);
      virtual void SetVertex(unsigned char pos, const Vector3 &vec);
      virtual const Vector3 &GetVertex(unsigned char pos) const;
      virtual const AABB GetAABB() const;

      virtual float GetClosestToPoint(const Vector3 &point) const; // returns offset from p1 towards p2 (0 == p1, 1 == p2)
      virtual float GetDistanceToPoint(const Vector3 &point, float &u) const;

      virtual Vector3 GetIntersectionPoint(const Line &line) const;
      virtual Vector3 GetIntersectionPoint(const Line &line, float &u) const;

      virtual bool WhatSide(const Vector3 &point);
      virtual float GetLength() const { return (vertices[0] - vertices[1]).GetLength(); }

    protected:
      Vector3 vertices[2];

    private:

  };

}

#endif
