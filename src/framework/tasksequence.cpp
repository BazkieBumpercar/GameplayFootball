// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "tasksequence.hpp"

#include "managers/taskmanager.hpp"

#include "framework/scheduler.hpp"
#include "blunted.hpp"

namespace blunted {

  
  // TaskSequenceEntry_SystemTaskMessage

  TaskSequenceEntry_SystemTaskMessage::TaskSequenceEntry_SystemTaskMessage(boost::intrusive_ptr<ISystemTaskMessage> command) : command(command) {
  }

  TaskSequenceEntry_SystemTaskMessage::~TaskSequenceEntry_SystemTaskMessage() {
  }

  bool TaskSequenceEntry_SystemTaskMessage::Execute() {
    command->GetTask()->messageQueue.PushMessage(command, true);
    return true;
  }

  bool TaskSequenceEntry_SystemTaskMessage::IsReady() {
    return command->IsReady();
  }

  void TaskSequenceEntry_SystemTaskMessage::Wait() {
    command->Wait();
  }

  bool TaskSequenceEntry_SystemTaskMessage::Reset() {
    command->Reset();
    return true;
  }


  // TaskSequenceEntry_UserTaskMessage

  TaskSequenceEntry_UserTaskMessage::TaskSequenceEntry_UserTaskMessage(boost::intrusive_ptr<IUserTaskMessage> command) : command(command) {
  }

  TaskSequenceEntry_UserTaskMessage::~TaskSequenceEntry_UserTaskMessage() {
  }

  bool TaskSequenceEntry_UserTaskMessage::Execute() {
    TaskManager::GetInstance().EnqueueWork(command, true);
    return true;
  }

  bool TaskSequenceEntry_UserTaskMessage::IsReady() {
    return command->IsReady();
  }

  void TaskSequenceEntry_UserTaskMessage::Wait() {
    command->Wait();
  }

  bool TaskSequenceEntry_UserTaskMessage::Reset() {
    command->Reset();
    return true;
  }


  // TaskSequenceEntry_Lock

  TaskSequenceEntryLockThread::TaskSequenceEntryLockThread(boost::mutex &sequenceLock) : sequenceLock(sequenceLock) {
    Reset();
  }

  void TaskSequenceEntryLockThread::operator()() {
    sequenceLock.lock();
    isReady.SetData(true);
    boost::mutex::scoped_lock lock(GetScheduler()->somethingIsDoneMutex);
    GetScheduler()->somethingIsDone.notify_one();
  }

  void TaskSequenceEntryLockThread::Reset() {
    isReady.SetData(false);
  }

  bool TaskSequenceEntryLockThread::IsReady() {
    return isReady.GetData();
  }

  TaskSequenceEntry_Lock::TaskSequenceEntry_Lock(boost::mutex &sequenceLock) : sequenceLock(sequenceLock) {
    lockThreadObject = new TaskSequenceEntryLockThread(sequenceLock);
  }

  TaskSequenceEntry_Lock::~TaskSequenceEntry_Lock() {
    delete lockThreadObject;
  }

  bool TaskSequenceEntry_Lock::Execute() {
    lockThread = boost::thread(boost::ref(*lockThreadObject)); // do not make copy; the contained Lockable is not copyable
    return true;
  }

  bool TaskSequenceEntry_Lock::IsReady() {
    return lockThreadObject->IsReady();
  }

  bool TaskSequenceEntry_Lock::Reset() {
    lockThreadObject->Reset();
    return true;
  }


  // TaskSequenceEntry_Unlock

  TaskSequenceEntry_Unlock::TaskSequenceEntry_Unlock(boost::mutex &sequenceLock) : sequenceLock(sequenceLock) {
  }

  TaskSequenceEntry_Unlock::~TaskSequenceEntry_Unlock() {
  }

  bool TaskSequenceEntry_Unlock::Execute() {
    sequenceLock.unlock();
    return true;
  }

  bool TaskSequenceEntry_Unlock::IsReady() {
    return true;
  }


  // TaskSequenceEntry_Terminator

  TaskSequenceEntry_Terminator::TaskSequenceEntry_Terminator() {
  }

  TaskSequenceEntry_Terminator::~TaskSequenceEntry_Terminator() {
  }

  bool TaskSequenceEntry_Terminator::Execute() {
    return true;
  }

  bool TaskSequenceEntry_Terminator::IsReady() {
    return true;
  }


  // TaskSequence

  TaskSequence::TaskSequence(const std::string &name, int sequenceTime_ms, bool skipOnTooLate) : name(name), sequenceTime_ms(sequenceTime_ms), skipOnTooLate(skipOnTooLate) {
  }

  TaskSequence::~TaskSequence() {
    entries.clear();
  }

  void TaskSequence::AddEntry(boost::shared_ptr<ITaskSequenceEntry> entry) {
    entries.push_back(entry);
  }

  void TaskSequence::AddSystemTaskEntry(ISystem *system, e_TaskPhase taskPhase) {
    boost::intrusive_ptr<ISystemTaskMessage> message;

    std::string name = "sequence:" + GetName() + "/systemtask:" + system->GetName() + "/";

    switch (taskPhase) {
      case e_TaskPhase_Get:
        name.append("get");
        message = boost::intrusive_ptr<ISystemTaskMessage>(new SystemTaskMessage_GetPhase(name, system->GetTask()));
        break;
      case e_TaskPhase_Process:
        name.append("process");
        message = boost::intrusive_ptr<ISystemTaskMessage>(new SystemTaskMessage_ProcessPhase(name, system->GetTask()));
        break;
      case e_TaskPhase_Put:
        name.append("put");
        message = boost::intrusive_ptr<ISystemTaskMessage>(new SystemTaskMessage_PutPhase(name, system->GetTask()));
        break;
    }
    boost::shared_ptr<TaskSequenceEntry_SystemTaskMessage> taskSequenceEntry(new TaskSequenceEntry_SystemTaskMessage(message));

    AddEntry(taskSequenceEntry);
  }

  void TaskSequence::AddUserTaskEntry(boost::shared_ptr<IUserTask> userTask, e_TaskPhase taskPhase) {
    boost::intrusive_ptr<IUserTaskMessage> message;

    std::string name = "sequence:" + GetName() + "/usertask:" + userTask->GetName() + "/";

    switch (taskPhase) {
      case e_TaskPhase_Get:
        name.append("get");
        message = boost::intrusive_ptr<IUserTaskMessage>(new UserTaskMessage_GetPhase(name, userTask));
        break;
      case e_TaskPhase_Process:
        name.append("process");
        message = boost::intrusive_ptr<IUserTaskMessage>(new UserTaskMessage_ProcessPhase(name, userTask));
        break;
      case e_TaskPhase_Put:
        name.append("put");
        message = boost::intrusive_ptr<IUserTaskMessage>(new UserTaskMessage_PutPhase(name, userTask));
        break;
    }
    boost::shared_ptr<TaskSequenceEntry_UserTaskMessage> taskSequenceEntry(new TaskSequenceEntry_UserTaskMessage(message));

    AddEntry(taskSequenceEntry);
  }

  void TaskSequence::AddLockEntry(boost::mutex &theLock, e_LockAction lockAction) {
    boost::shared_ptr<ITaskSequenceEntry> someLock;
    switch (lockAction) {
      case e_LockAction_Lock:
        someLock = boost::shared_ptr<ITaskSequenceEntry>(new TaskSequenceEntry_Lock(theLock));
        break;
      case e_LockAction_Unlock:
        someLock = boost::shared_ptr<ITaskSequenceEntry>(new TaskSequenceEntry_Unlock(theLock));
        break;
    }
    AddEntry(someLock);
  }

  void TaskSequence::AddTerminator() {
    boost::shared_ptr<ITaskSequenceEntry> arnie(new TaskSequenceEntry_Terminator());
    AddEntry(arnie);
  }


  int TaskSequence::GetEntryCount() const {
    return entries.size();
  }

  boost::shared_ptr<ITaskSequenceEntry> TaskSequence::GetEntry(int num) {
    assert(num < (signed int)entries.size());
    return entries.at(num);
  }

  int TaskSequence::GetSequenceTime() const {
    return sequenceTime_ms;
  }

  void TaskSequence::SetSequenceTime(int value) {
    sequenceTime_ms = value;
  }

  const std::string TaskSequence::GetName() const {
    return name;
  }

}
