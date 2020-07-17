// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MANAGERS_TASK
#define _HPP_MANAGERS_TASK

#include "defines.hpp"

#include "types/singleton.hpp"

#include "systems/isystemtask.hpp"
#include "base/properties.hpp"

namespace blunted {

  class WorkerThread;

  class TaskManager : public Singleton<TaskManager> {

    public:
      TaskManager();
      ~TaskManager();

      /// start worker threads
      void Initialize();

      /// wait until the workQueue is processed
      void EmptyQueue();

      /// gracefully exit and then kill worker threads
      void Exit();

      int GetWorkerThreadCount();
      e_ThreadState GetWorkerThreadState(int workerThreadIndex);
      void GetFullWorkerThreadState(int workerThreadIndex, e_ThreadState &state, std::string &commandName);

      /// insert a message in the workQueue
      void EnqueueWork(boost::intrusive_ptr<Command> message, bool notify = true); // ATOMIC

      void NotifyWorkers(e_NotificationSubject notificationSubject = e_NotificationSubject_All);

      /// wait on a new workQueue item
      boost::intrusive_ptr<Command> WaitForWork(); // ATOMIC

    protected:
      MessageQueue < boost::intrusive_ptr<Command> > workQueue;
      std::vector<WorkerThread*> pool;


  };

}

#endif
