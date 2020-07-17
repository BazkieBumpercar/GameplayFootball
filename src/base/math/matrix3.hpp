// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _hpp_bluntmath_matrix3
#define _hpp_bluntmath_matrix3

#include "defines.hpp"

#include "bluntmath.hpp"

namespace blunted {

  class Vector3;
  class Matrix4;

  // column major, row minor matrix
  class Matrix3 {

    public:
      Matrix3();
      Matrix3(real values[9]);
      Matrix3(real v1, real v2, real v3, real v4, real v5, real v6, real v7, real v8, real v9);
      Matrix3(const Matrix3 &mat3);
      Matrix3(const Matrix4 &mat4);
      virtual ~Matrix3();

      // ----- operator overloading
      void operator = (const Matrix4 &mat4);
      Matrix3 operator * (const Matrix3 &multiplier) const;
      Vector3 operator * (const Vector3 &multiplier) const;

      // ----- mathematics
      void Transpose();

      static const Matrix3 ZERO;
      static const Matrix3 IDENTITY;
      real elements[9];

    protected:

    private:

  };

}

#endif
