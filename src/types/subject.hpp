// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SUBJECT
#define _HPP_SUBJECT

#include "defines.hpp"

#include "types/command.hpp"
#include "types/interpreter.hpp"

#include "base/log.hpp"

namespace blunted {

  // this class is not reentrant, derived classes and their users take this responsibility (by using Subject::subjectMutex)
  template <class T = Observer>
  class Subject {

    public:
      Subject() {
      }
      
      virtual ~Subject() {
        observers.clear();
      }

      virtual void Attach(boost::intrusive_ptr<T> observer, void *thisPtr = 0) {

        observer->SetSubjectPtr(thisPtr);

        observers.push_back(observer);
      }

      virtual void Detach(boost::intrusive_ptr<T> observer) {
        typename std::vector < boost::intrusive_ptr<T> >::iterator o_iter = observers.begin();
        while (o_iter != observers.end()) {
          if ((*o_iter).get() == observer.get()) {
            (*o_iter).reset();
            o_iter = observers.erase(o_iter);
          } else {
            o_iter++;
          }
        }
      }

      virtual void DetachAll() {
        typename std::vector < boost::intrusive_ptr<T> >::iterator o_iter = observers.begin();
        while (o_iter != observers.end()) {
          (*o_iter).reset();
          o_iter = observers.erase(o_iter);
        }
      }

      std::vector < boost::intrusive_ptr<T> > GetObservers() {
        return observers;
      }

      mutable boost::mutex subjectMutex;

    protected:
      std::vector < boost::intrusive_ptr<T> > observers;

  };

}

#endif
