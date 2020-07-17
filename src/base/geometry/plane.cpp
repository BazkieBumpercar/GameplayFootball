// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "plane.hpp"

#include <cassert>
#include <cmath>

namespace blunted {

  Plane::Plane() : determinant(0) {
    _dirty_determinant = true;
  }

  Plane::Plane(const Vector3 vec1, const Vector3 vec2) {
    SetVertex(0, vec1);
    SetVertex(1, vec2);
    _dirty_determinant = true;
  }

  Plane::~Plane() {
  }

  void Plane::Set(const Vector3 &pos, const Vector3 &dir) {
    SetVertex(0, pos);
    SetVertex(1, dir);
  }

  void Plane::SetVertex(unsigned char pos, const real x, const real y, const real z) {
    assert(pos < 2);
    vertices[pos].coords[0] = x;
    vertices[pos].coords[1] = y;
    vertices[pos].coords[2] = z;
    _dirty_determinant = true;
  }

  void Plane::SetVertex(unsigned char pos, const Vector3 &vec) {
    assert(pos < 2);
    vertices[pos].coords[0] = vec.coords[0];
    vertices[pos].coords[1] = vec.coords[1];
    vertices[pos].coords[2] = vec.coords[2];
    _dirty_determinant = true;
  }

  const Vector3 &Plane::GetVertex(unsigned char pos) const {
    assert(pos < 2);
    return vertices[pos];
  }

  void Plane::CalculateDeterminant() const {
    determinant = -GetVertex(0).GetDotProduct(GetVertex(1));
    _dirty_determinant = false;
  }

  real Plane::GetDeterminant() const {
    if (_dirty_determinant) CalculateDeterminant();
    return determinant;
  }

}
