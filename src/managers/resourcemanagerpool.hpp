// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_RESOURCEMANAGERPOOL
#define _HPP_RESOURCEMANAGERPOOL

#include "managers/resourcemanager.hpp"
#include "types/singleton.hpp"
#include "types/resource.hpp"

namespace blunted {

  class ResourceManagerPool : public Singleton<ResourceManagerPool> {

    public:
      ResourceManagerPool() {};
      virtual ~ResourceManagerPool() {};

      virtual void Exit() {
        resourceManagers.clear();
      }

      void CleanUp() {
        std::map < e_ResourceType, boost::shared_ptr<void> >::iterator resmanIter = resourceManagers.begin();
        while (resmanIter != resourceManagers.end()) {
          boost::static_pointer_cast < ResourceManager<void> > ((*resmanIter).second)->RemoveUnused();
          resmanIter++;
        }
      }

      template <typename T> void RegisterManager(e_ResourceType resourceType, boost::shared_ptr < ResourceManager<T> > resourceManager) {
        resourceManagers.insert(std::make_pair(resourceType, resourceManager));
      }

      template <typename T> boost::shared_ptr < ResourceManager<T> > GetManager(e_ResourceType resourceType) {
        typename std::map < e_ResourceType, boost::shared_ptr<void> >::iterator iter = resourceManagers.find(resourceType);
        if (iter != resourceManagers.end()) {
          return boost::static_pointer_cast < ResourceManager<T> > ((*iter).second);
        } else {
          Log(e_FatalError, "ResourceManagerPool", "GetManager", "Could not find manager for type " + resourceType);
          return boost::shared_ptr < ResourceManager<T> >();
        }
      }

    protected:
      std::map < e_ResourceType, boost::shared_ptr<void> > resourceManagers;

  };

}

#endif
