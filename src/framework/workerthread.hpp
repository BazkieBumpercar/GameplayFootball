// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_WORKERTHREAD
#define _HPP_WORKERTHREAD

#include "types/thread.hpp"

namespace blunted {

  class TaskManager;

  class WorkerThread : public Thread {

    public:
      WorkerThread(int affinity = -1);
      ~WorkerThread();

      void operator()();

      void GetWorkerState(e_ThreadState &state, std::string &commandName);

    protected:
      TaskManager *taskManager;
      int affinity;
      unsigned long messagesHandled;

      Lockable<std::string> currentCommandName;

  };

}

#endif
