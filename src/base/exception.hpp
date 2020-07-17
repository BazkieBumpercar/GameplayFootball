// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_EXCEPTION
#define _HPP_EXCEPTION

#include "defines.hpp"

#include <exception>

namespace blunted {

  class BluntedException : public std::exception {

    public:
      BluntedException(const std::string classname, const std::string methodname, const std::string description);
      ~BluntedException() throw() {}

  };

  #ifndef ThrowException
  #define ThrowException(classname, methodname, description) throw BluntedException::BluntedException(classname, methodname, description);
  #endif

}

#endif
