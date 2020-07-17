// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "resourcemanagerpool.hpp"

namespace blunted {

  template<> ResourceManagerPool* Singleton<ResourceManagerPool>::singleton = 0;

}
