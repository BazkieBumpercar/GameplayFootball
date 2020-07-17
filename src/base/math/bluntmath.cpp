// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "bluntmath.hpp"

#include <cmath>

#include <boost/random.hpp>

#include <boost/thread/mutex.hpp>

namespace blunted {

  unsigned int fastrandseed;
  unsigned int max_uint;

  boost::mutex randMutex;

  typedef boost::mt19937 BaseGenerator;
  typedef boost::uniform_real<float> Distribution;
  typedef boost::variate_generator<BaseGenerator, Distribution> Generator;
  BaseGenerator base;
  Distribution dist;
  Generator rng(base, dist);

  real clamp(const real value, const real min, const real max) {
    assert(max >= min);
    if (min > value) return min;
    if (max < value) return max;
    return value;
  }

  real NormalizedClamp(const real value, const real min, const real max) {
    assert(max > min);
    real banana = clamp(value, min, max);
    banana = (banana - min) / (max - min);
    return banana;
  }

  real invsqrt(real fvalue) {
    return 1. / sqrt(fvalue);
  }

  float dot_product(real v1[3], real v2[3]) {
    return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
  }

  void normalize(real v[3]) {
    real f = 1.0f / sqrt(dot_product(v, v));

    v[0] *= f;
    v[1] *= f;
    v[2] *= f;
  }

  bool sign(real n) {
    return n >= 0;
  }

  signed int signSide(real n) {
    return n >= 0 ? 1 : -1;
  }

  bool is_odd(int n) {
    return n & 1;
  }

  void randomseed() {
    rng.engine().seed(static_cast<unsigned int>(std::time(0)));
  }

  inline real boostrandom() {
    return rng();
  }

  real random(real min, real max) {
    float stretch = max - min;

    randMutex.lock();
    real value = min + (boostrandom() * stretch);
    randMutex.unlock();
    return value;
  }

  int pot(int x) {
    int val = 1;
    while (val < x) {
      val *= 2;
    }
    return val;
  }

  real ModulateIntoRange(real min, real max, real value) {
    real step = max - min;
    real newValue = value;
    while (newValue < min) newValue += step;
    while (newValue > max) newValue -= step;
    return newValue;
  }

}
