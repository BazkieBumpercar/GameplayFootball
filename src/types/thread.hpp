// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_THREAD
#define _HPP_THREAD

#include "defines.hpp"

#include "types/messagequeue.hpp"
#include "types/lockable.hpp"

#include "boost/thread.hpp"

namespace blunted {

  enum e_ThreadState {
    e_ThreadState_Idle,
    e_ThreadState_Sleeping,
    e_ThreadState_Busy,
    e_ThreadState_Exiting
  };

  class Thread {

    public:
      Thread() {
      }

      virtual ~Thread() {
      }

      e_ThreadState GetState() { // ATOMIC
        state.Lock();
        e_ThreadState curstate = state.data;
        state.Unlock();
        return curstate;
      }

      void SetState(e_ThreadState newState) { // ATOMIC
        state.Lock();
        state.data = newState;
        state.Unlock();
      }


      // --- USE WITH CARE: USER LOCKING RESPONSIBILITY

      void LockState() {
        state.Lock();
      }

      e_ThreadState GetState_NoLock() {
        return state.data;
      }

      void SetState_NoLock(e_ThreadState newState) {
        state.data = newState;
      }

      void UnlockState() {
        state.Unlock();
      }

      // --- /CARE


      void Run() {
        thread = boost::thread(boost::ref( *this ));
      }

      void Join() {
        thread.join();
      }

      // thread main loop
      virtual void operator()() = 0;

      MessageQueue < boost::intrusive_ptr<Command> > messageQueue;

      boost::thread thread;

    protected:
      Lockable<e_ThreadState> state;

  };


  // messages

  class Message_Shutdown : public Command {

    public:
      Message_Shutdown() : Command("shutdown") {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        return false;
      }

  };

}

#endif
