// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MANAGERS_ENVIRONMENT
#define _HPP_MANAGERS_ENVIRONMENT

#include "defines.hpp"

#include "types/singleton.hpp"
#include "types/lockable.hpp"

namespace blunted {

  class EnvironmentManager : public Singleton<EnvironmentManager> {

    public:
      EnvironmentManager();
      virtual ~EnvironmentManager();

      void SignalQuit();
      bool GetQuit();

      unsigned long GetTime_ms();
      void Pause_ms(int duration);

    protected:
      Lockable<bool> quit;
      unsigned long startTime_ms;
      boost::posix_time::ptime startTime;

  };

}

#endif
