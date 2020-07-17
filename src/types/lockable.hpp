// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_LOCKABLE
#define _HPP_LOCKABLE

#include "defines.hpp"

namespace blunted {

  enum e_NotificationSubject {
    e_NotificationSubject_One,
    e_NotificationSubject_All,
  };

  template <typename T>
  class Lockable {

    public:

      Lockable() {}

      T GetData() const {
        mutex.lock();
        T tmp = data;
        mutex.unlock();
        return tmp;
      }

      void SetData(const T &newdata) {
        mutex.lock();
        this->data = newdata;
        mutex.unlock();
      }

      inline T operator=(const T &param) {
        SetData(param);
        return param;
      }

      inline T *operator->() {
        return &data;
      }

      inline void Lock() {
        mutex.lock();
      }

      inline void Unlock() {
        mutex.unlock();
      }

      T data;

      mutable boost::mutex mutex;

    protected:

  };

}

#endif
