// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_TYPES_SINGLETON
#define _HPP_TYPES_SINGLETON

#include "defines.hpp"

namespace blunted {

  template <typename T> class Singleton {

    public:
      Singleton(void) {
        assert(!singleton);
        singleton = static_cast<T*>(this);
      }

      virtual ~Singleton(void) {
        assert(singleton); // isn't this a paradox? :D
        singleton = 0;
      }

      static T& GetInstance(void) {
        assert(singleton);
        return(*singleton);
      }

      static T* GetInstancePtr(void) {
        return singleton;
      }

      virtual void Destroy() {
        delete singleton;
        singleton = 0;
      }

    protected:
      static T* singleton;

  };

}

#endif
