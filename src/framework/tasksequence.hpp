// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_TASKSEQUENCE
#define _HPP_TASKSEQUENCE

#include "defines.hpp"

#include "types/command.hpp"

#include "systems/isystem.hpp"
#include "systems/isystemtask.hpp"
#include "types/iusertask.hpp"

namespace blunted {

  class ITaskSequenceEntry {

    public:
      virtual ~ITaskSequenceEntry() {};
      virtual bool Execute() = 0;
      virtual bool IsReady() = 0;
      virtual void Wait() {};
      virtual bool Reset() { return true; };

    protected:

  };

  class TaskSequenceEntry_SystemTaskMessage : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_SystemTaskMessage(boost::intrusive_ptr<ISystemTaskMessage> command);
      virtual ~TaskSequenceEntry_SystemTaskMessage();

      virtual bool Execute();
      virtual bool IsReady();
      virtual void Wait();
      virtual bool Reset();

    protected:
      boost::intrusive_ptr<ISystemTaskMessage> command;

  };

  class TaskSequenceEntry_UserTaskMessage : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_UserTaskMessage(boost::intrusive_ptr<IUserTaskMessage> command);
      virtual ~TaskSequenceEntry_UserTaskMessage();

      virtual bool Execute();
      virtual bool IsReady();
      virtual void Wait();
      virtual bool Reset();

    protected:
      boost::intrusive_ptr<IUserTaskMessage> command;

  };

  class TaskSequenceEntryLockThread {

    public:
      TaskSequenceEntryLockThread(boost::mutex &sequenceLock);
      void operator()();
      void Reset();
      bool IsReady();

    protected:
      boost::mutex &sequenceLock;
      Lockable<bool> isReady;

  };

  class TaskSequenceEntry_Lock : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_Lock(boost::mutex &sequenceLock);
      virtual ~TaskSequenceEntry_Lock();

      virtual bool Execute();
      virtual bool IsReady();
      virtual bool Reset();

    protected:
      boost::mutex &sequenceLock;
      boost::thread lockThread;
      TaskSequenceEntryLockThread *lockThreadObject;

  };

  class TaskSequenceEntry_Unlock : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_Unlock(boost::mutex &sequenceLock);
      virtual ~TaskSequenceEntry_Unlock();

      virtual bool Execute();
      virtual bool IsReady();

    protected:
      boost::mutex &sequenceLock;

  };

  class TaskSequenceEntry_Terminator : public ITaskSequenceEntry {

    public:
      TaskSequenceEntry_Terminator();
      virtual ~TaskSequenceEntry_Terminator();

      virtual bool Execute();
      virtual bool IsReady();

  };

  enum e_TaskPhase {
    e_TaskPhase_Get,
    e_TaskPhase_Process,
    e_TaskPhase_Put
  };

  enum e_LockAction {
    e_LockAction_Lock,
    e_LockAction_Unlock
  };

  class TaskSequence {

    public:
      TaskSequence(const std::string &name, int sequenceTime_ms, bool skipOnTooLate = true);
      virtual ~TaskSequence();

      void AddEntry(boost::shared_ptr<ITaskSequenceEntry> entry);
      void AddSystemTaskEntry(ISystem *system, e_TaskPhase taskPhase);
      void AddUserTaskEntry(boost::shared_ptr<IUserTask> userTask, e_TaskPhase taskPhase);
      void AddLockEntry(boost::mutex &theLock, e_LockAction lockAction);
      void AddTerminator();

      int GetEntryCount() const;
      boost::shared_ptr<ITaskSequenceEntry> GetEntry(int num);
      int GetSequenceTime() const;
      void SetSequenceTime(int value);
      const std::string GetName() const;
      bool GetSkippable() const { return skipOnTooLate; }

    protected:
      std::string name;

      std::vector < boost::shared_ptr<ITaskSequenceEntry> > entries;

      // time assigned for 1 run of this sequence
      // if 0, run continuously
      int sequenceTime_ms;

      // if at due start time the previous run is not ready yet,
      // if true: just forget about the lost time
      // if false: start as soon as previous run is ready, to keep up with the sync
      bool skipOnTooLate;

  };

}

#endif
