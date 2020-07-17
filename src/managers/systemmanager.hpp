// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MANAGERS_SYSTEM
#define _HPP_MANAGERS_SYSTEM

#include "defines.hpp"

#include "types/singleton.hpp"

namespace blunted {

  class ISystem;
  class IScene;
  class Object;

  typedef std::map <std::string, ISystem*> map_Systems;

  /// manages the registration of systems and the creation of their scenes and objects

  class SystemManager : public Singleton<SystemManager> {

    public:
      SystemManager();
      virtual ~SystemManager();

      virtual void Exit();
      bool RegisterSystem(const std::string systemName, ISystem *system);
      void CreateSystemScenes(boost::shared_ptr<IScene> scene);
      const map_Systems &GetSystems() const;
      ISystem *GetSystem(const std::string &name) const;

    protected:
      map_Systems systems;

    private:

  };

}

#endif
