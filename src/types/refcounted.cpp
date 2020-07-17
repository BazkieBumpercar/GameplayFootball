// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "refcounted.hpp"

namespace blunted {

  RefCounted::RefCounted() : refCount(0) {
  }

  RefCounted::~RefCounted() {
  }

  RefCounted::RefCounted(const RefCounted &src) : refCount(0) {
  }

  RefCounted& RefCounted::operator=(const RefCounted &src) {
    return *this;
  }

  unsigned long RefCounted::GetRefCount() {
    int i = refCount;
    return i;
  }

  void intrusive_ptr_add_ref(RefCounted *p) {
    assert(p);
#ifdef WIN32
    InterlockedIncrement(&(p->refCount));
#else
    p->refCountMutex.lock();
    ++(p->refCount);
    p->refCountMutex.unlock();
#endif

  }

  void intrusive_ptr_release(RefCounted *p) {
    assert(p);

#ifdef WIN32
    if (InterlockedDecrement(&(p->refCount)) == 0) delete p;
#else
    p->refCountMutex.lock();
    if (--(p->refCount) == 0) {
      p->refCountMutex.unlock();
      delete p;
    } else {
      p->refCountMutex.unlock();
    }
#endif

  }

}
