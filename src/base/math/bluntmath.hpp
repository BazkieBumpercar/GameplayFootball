// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _hpp_bluntmath
#define _hpp_bluntmath

#include "defines.hpp"

#include <limits>

// todo: function names should probably start with caps

namespace blunted {

  real clamp(const real value, const real min, const real max);
  real NormalizedClamp(const real value, const real min, const real max);
  real invsqrt(real fvalue);

  // you can never be too specific ;)
  const real pi = 3.1415926535897932384626433832795028841972f; // last decimal rounded ;)
  extern unsigned int fastrandseed;
  extern unsigned int max_uint;

  typedef real radian;

	void normalize(real v[3]);
  bool sign(real n);
  signed int signSide(real n); // returns -1 or 1
  bool is_odd(int n);
  void randomseed();
  real random(real min, real max);

  inline void fastrandomseed() {
    fastrandseed = static_cast<unsigned int>(std::time(0));
    max_uint = std::numeric_limits<unsigned int>::max();
  }

  inline real fastrandom(real min, real max) {
    real range = max - min;
    real tmp = (fastrandseed / (max_uint * 1.0f)) * range + min;
    fastrandseed = (214013 * fastrandseed + 2531011);
    return tmp;
  }

  inline float curve(float source, float bias = 1.0f) { // make linear / into sined _/-
    return (sin((source - 0.5f) * pi) * 0.5f + 0.5f) * bias +
           source * (1.0f - bias);
  }

  int pot(int x);
  real ModulateIntoRange(real min, real max, real value);

}

#endif
