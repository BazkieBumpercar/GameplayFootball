// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_LOG
#define _HPP_LOG

#include "defines.hpp"

namespace blunted {

  enum e_LogType {
    e_Notice,
    e_Warning,
    e_Error,
    e_FatalError
  };

  typedef boost::signals2::signal < void(e_LogType, std::string, std::string, std::string) > signal_LogCallback;

  void LogOpen();
  void LogClose();

  boost::signals2::connection BindLog(const signal_LogCallback::slot_type &slot);

  void Log(e_LogType logType, std::string className, std::string methodName, std::string message);

}

#endif
