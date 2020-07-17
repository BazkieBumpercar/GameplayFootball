// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_AABB
#define _HPP_AABB

#include "base/math/vector3.hpp"
#include "base/math/quaternion.hpp"

#include "line.hpp"
#include "plane.hpp"

namespace blunted {

  class AABB {

    public:
      AABB();
      AABB(const AABB &src);
      virtual ~AABB();

      AABB operator += (const AABB &add);
      AABB operator + (const Vector3 &vec) const;
      AABB operator * (const Quaternion &rot) const;

      void Reset();

      void SetMinXYZ(const Vector3 &min);
      void SetMaxXYZ(const Vector3 &max);

      real GetRadius() const;
      void GetCenter(Vector3 &center) const;
      bool Intersects(const Vector3 &center, const real radius) const;
      bool Intersects(const vector_Planes &planes) const;
      bool Intersects(const AABB &src) const;

      void MakeDirty() { radius_needupdate = true; center_needupdate = true; }

      bool Intersects(const Line &src) const;

      Vector3 minxyz;
      Vector3 maxxyz;
      mutable real radius;
      mutable Vector3 center;

    protected:
      mutable bool radius_needupdate;
      mutable bool center_needupdate;

  };

  struct AABBCache {
    bool dirty;
    AABB aabb;
  };

}

#endif
