// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "exception.hpp"

#include "base/log.hpp"

namespace blunted {

  BluntedException::BluntedException(std::string classname, std::string methodname, std::string description) {
    printf("exception in [%s::%s] %s\n", classname.c_str(), methodname.c_str(), description.c_str());
    char errorString[256];
    sprintf(errorString, "exception in [%s::%s] %s\n", classname.c_str(), methodname.c_str(), description.c_str());
    Log(e_FatalError, "blunted", "exception", errorString);
    exit(1);
  }

}
