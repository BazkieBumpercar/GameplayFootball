// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "taskmanager.hpp"

#include "framework/workerthread.hpp"

#include "base/log.hpp"

namespace blunted {

  template<> TaskManager* Singleton<TaskManager>::singleton = 0;

  TaskManager::TaskManager() {
  }

  TaskManager::~TaskManager() {
  }

  void TaskManager::Initialize() {

    int numthreads = boost::thread::hardware_concurrency();
    if (numthreads < 3) numthreads = 3; // todo: make minimum configurable
    for (int i = 0; i < numthreads; i++) {
      WorkerThread *worker = new WorkerThread(i);
      pool.push_back(worker);
      worker->Run();
    }
  }

  void TaskManager::EmptyQueue() {
    // wait till workqueue empty
    while (workQueue.GetPending() > 0) {}
  }

  void TaskManager::Exit() {
    EmptyQueue();

    // gracefully shutdown threads
    int poolSize = pool.size();
    for (int i = 0; i < poolSize; i++) {
      boost::intrusive_ptr<Message_Shutdown> shutdown(new Message_Shutdown());
      workQueue.PushMessage(shutdown);
      //printf("waiting for wait\n");
      shutdown->Wait();
      //printf("ready waiting\n");
    }

    // then STRIKE THEM DOWN WITH FURIOUS ANGER
    for (int i = 0; i < poolSize; i++) {
      pool.at(i)->Join();
      // raAAAH!!
      delete pool.at(i);
    }

    pool.clear();

    if (workQueue.GetPending() > 0) Log(e_FatalError, "TaskManager", "Exit", workQueue.GetPending() + " messages left on quit!");
  }

  int TaskManager::GetWorkerThreadCount() {
    return pool.size();
  }

  e_ThreadState TaskManager::GetWorkerThreadState(int workerThreadIndex) {
    return pool.at(workerThreadIndex)->GetState();
  }

  void TaskManager::GetFullWorkerThreadState(int workerThreadIndex, e_ThreadState &state, std::string &commandName) {
    pool.at(workerThreadIndex)->GetWorkerState(state, commandName);
  }

  void TaskManager::EnqueueWork(boost::intrusive_ptr<Command> message, bool notify) { // ATOMIC
    if (pool.size() > 0) {

      if (workQueue.GetPending() > 50) {
        Log(e_Warning, "TaskManager", "EnqueueWork", "Too much work to do! Blocking for 1 ms!");
        while (workQueue.GetPending() > 0) {
          EnvironmentManager::GetInstance().Pause_ms(1);
        }
      }
      workQueue.PushMessage(message, notify);
    } else {
      message->Handle(this);
      message.reset();
    }
  }

  // todo: won't work optimally with multiple threads notifying
  void TaskManager::NotifyWorkers(e_NotificationSubject notificationSubject) {
    workQueue.NotifyWaiting(notificationSubject);
  }

  boost::intrusive_ptr<Command> TaskManager::WaitForWork() { // ATOMIC
    return workQueue.WaitForMessage();
  }

}
