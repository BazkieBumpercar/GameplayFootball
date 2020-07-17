// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "log.hpp"

#include <iostream>
#include <fstream>

namespace blunted {

  signal_LogCallback callback;

  std::ofstream logFile;
  boost::mutex mutex;

  void LogOpen() {
    logFile.open("log.txt", std::ios::out);
  }

  void LogClose() {
    if (logFile.is_open()) logFile.close();
  }

  boost::signals2::connection BindLog(const signal_LogCallback::slot_type &slot) {
    return callback.connect(slot);
  }

  void Log(e_LogType logType, std::string className, std::string methodName, std::string message) {
    std::string logTypeString;

    switch (logType) {
      case e_Notice: logTypeString = "Notice"; break;
      case e_Warning: logTypeString = "Warning"; break;
      case e_Error: logTypeString = "ERROR"; break;
      case e_FatalError: logTypeString = "FATAL ERROR !!! N00000 !!!"; break;
    }

    char bla[2048];
    sprintf(bla, "[%s] in [%s::%s]: %s\n", logTypeString.c_str(), className.c_str(), methodName.c_str(), message.c_str());
    callback(logType, className.c_str(), methodName.c_str(), message.c_str());

    mutex.lock();
    printf("%s", bla);
    if (logFile.is_open()) logFile << bla;
    logFile.flush();
    mutex.unlock();

    if (logType == e_FatalError) {
      LogClose();

#ifndef NDEBUG
      // for gdb backtracing
      int *foo = (int*)-1; // make a bad pointer
      printf("%d\n", *foo); // causes segfault
#endif

      exit(1);
    }
  }

}
