// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _hpp_plane
#define _hpp_plane

#include "base/math/vector3.hpp"

#include <vector>

namespace blunted {

  class Plane;
  typedef std::vector<Plane> vector_Planes;

  class Plane {
    // vector 0 = a position on the plane
    // vector 1 = plane normal

    public:
      Plane();
      Plane(const Vector3 vec1, const Vector3 vec2);
      virtual ~Plane();

      virtual void Set(const Vector3 &pos, const Vector3 &dir);
      virtual void SetVertex(unsigned char pos, const real x, const real y, const real z);
      virtual void SetVertex(unsigned char pos, const Vector3 &vec);
      virtual const Vector3 &GetVertex(unsigned char pos) const;

      virtual void CalculateDeterminant() const;
      virtual real GetDeterminant() const;

    protected:
      Vector3 vertices[2];
      mutable real determinant;
      mutable bool _dirty_determinant;

    private:

  };

}

#endif
