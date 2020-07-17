// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "workerthread.hpp"

#include "base/log.hpp"

#include "managers/taskmanager.hpp"
#include "base/utils.hpp"

namespace blunted {

  WorkerThread::WorkerThread(int affinity) : Thread(), affinity(affinity), messagesHandled(0) {
    taskManager = TaskManager::GetInstancePtr();
  }

  WorkerThread::~WorkerThread() {
    Log(e_Notice, "WorkerThread", "~WorkerThread", int_to_str(messagesHandled) + " messages handled");
  }

  void WorkerThread::operator()() {
    Log(e_Notice, "WorkerThread", "operator()()", "Starting worker thread");

    bool forceAffinity = false; // for debugging
    if (forceAffinity && affinity != -1) {
      #ifdef __linux__
        cpu_set_t cpus;
        CPU_ZERO(&cpus);
        CPU_SET(affinity, &cpus);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpus);
      #endif
      #ifdef WIN32
        DWORD_PTR affinityMask = 1 << affinity;
        bool success = SetThreadAffinityMask(GetCurrentThread(), affinityMask);
        if (!success) printf("set process affinity failed: %li\n", GetLastError());
      #endif
    }


    SetState(e_ThreadState_Idle);

    bool quit = false;
    while (!quit) {

      // check mailbox
      boost::intrusive_ptr<Command> message = taskManager->WaitForWork();

      // need to set thread state and command name at the same time
      LockState();
      SetState_NoLock(e_ThreadState_Busy);
      currentCommandName.SetData(message->GetName());
      UnlockState();

      if (!message->Handle(this)) quit = true;
      message.reset();

      LockState();
      SetState_NoLock(e_ThreadState_Idle);
      currentCommandName.SetData("");
      UnlockState();

      messagesHandled++;
    }

    Log(e_Notice, "WorkerThread", "operator()()", "Exiting worker thread");
  }

  void WorkerThread::GetWorkerState(e_ThreadState &state, std::string &commandName) {
    LockState();
    state = GetState_NoLock();
    commandName = currentCommandName.GetData();
    UnlockState();
  }

}
