// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_INTERFACE_USERTASK
#define _HPP_INTERFACE_USERTASK

#include "command.hpp"

namespace blunted {

  class IUserTask {

    public:
      virtual ~IUserTask() {};

      virtual void GetPhase() = 0;
      virtual void ProcessPhase() = 0;
      virtual void PutPhase() = 0;

      virtual std::string GetName() const = 0;

    protected:

  };


  // messages

  class IUserTaskMessage : public Command {

    public:
      IUserTaskMessage(const std::string &name, boost::shared_ptr<IUserTask> task) : Command(name), task(task) {};

    protected:
      boost::shared_ptr<IUserTask> task;

  };

  class UserTaskMessage_GetPhase : public IUserTaskMessage {

    public:
      UserTaskMessage_GetPhase(const std::string &name, boost::shared_ptr<IUserTask> task) : IUserTaskMessage(name, task) {};

    protected:
      virtual bool Execute(void *caller = NULL);

  };

  class UserTaskMessage_ProcessPhase : public IUserTaskMessage {

    public:
      UserTaskMessage_ProcessPhase(const std::string &name, boost::shared_ptr<IUserTask> task) : IUserTaskMessage(name, task) {};

    protected:
      virtual bool Execute(void *caller = NULL);

  };

  class UserTaskMessage_PutPhase : public IUserTaskMessage {

    public:
      UserTaskMessage_PutPhase(const std::string &name, boost::shared_ptr<IUserTask> task) : IUserTaskMessage(name, task) {};

    protected:
      virtual bool Execute(void *caller = NULL);

  };

}

#endif
