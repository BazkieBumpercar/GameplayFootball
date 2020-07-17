// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "systemmanager.hpp"
#include "scenemanager.hpp"

#include "systems/isystem.hpp"
#include "base/log.hpp"
#include "scene/object.hpp"

namespace blunted {

  template<> SystemManager* Singleton<SystemManager>::singleton = 0;

  SystemManager::SystemManager() {
  }

  SystemManager::~SystemManager() {
  }

  void SystemManager::Exit() {
    map_Systems::iterator s_iter = systems.begin();
    while (s_iter != systems.end()) {
      ISystem *system = s_iter->second;
      Log(e_Notice, "SystemManager", "Exit", "Deleting system named '" + s_iter->first + "'");
      system->Exit();
      delete system;
      s_iter++;
    }
    systems.clear();
  }

  bool SystemManager::RegisterSystem(const std::string systemName, ISystem *system) {
    std::pair <map_Systems::iterator, bool> result = systems.insert(map_Systems::value_type(systemName, system));
    if (result.second == false) {
      // property already exists, replace its value?
      return false;
    }
    Log(e_Notice, "SystemManager", "RegisterSystem", "Added system named '" + systemName + "'");
    return true;
  }

  void SystemManager::CreateSystemScenes(boost::shared_ptr<IScene> scene) {
    map_Systems::iterator s_iter = systems.begin();
    while (s_iter != systems.end()) {
      ISystem *system = s_iter->second;
      system->CreateSystemScene(scene);
      s_iter++;
    }
    scene->Init();
  }

  const map_Systems &SystemManager::GetSystems() const {
    return systems;
  }

  ISystem *SystemManager::GetSystem(const std::string &name) const {
    map_Systems::const_iterator s_iter = systems.find(name);
    if (s_iter != systems.end()) {
      return s_iter->second;
    } else {
      return 0;
    }
  }

}
