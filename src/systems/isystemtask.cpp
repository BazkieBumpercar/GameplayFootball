// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "isystemtask.hpp"

#include "framework/scheduler.hpp"
#include "blunted.hpp"

namespace blunted {

  bool SystemTaskMessage_GetPhase::Execute(void *caller) {
    task->GetPhase();
    boost::mutex::scoped_lock lock(GetScheduler()->somethingIsDoneMutex);
    GetScheduler()->somethingIsDone.notify_one();
    return true;
  }

  bool SystemTaskMessage_ProcessPhase::Execute(void *caller) {
    task->ProcessPhase();
    boost::mutex::scoped_lock lock(GetScheduler()->somethingIsDoneMutex);
    GetScheduler()->somethingIsDone.notify_one();
    return true;
  }

  bool SystemTaskMessage_PutPhase::Execute(void *caller) {
    task->PutPhase();
    boost::mutex::scoped_lock lock(GetScheduler()->somethingIsDoneMutex);
    GetScheduler()->somethingIsDone.notify_one();
    return true;
  }

}
