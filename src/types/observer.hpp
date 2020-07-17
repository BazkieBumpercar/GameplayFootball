// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBSERVER
#define _HPP_OBSERVER

#include "defines.hpp"

#include "command.hpp"

#include "types/refcounted.hpp"

namespace blunted {

  class Observer : public RefCounted {

    public:
      Observer();
      virtual ~Observer();

      void SetSubjectPtr(void *subjectPtr);

    protected:
      void *subjectPtr;

  };

}

#endif
