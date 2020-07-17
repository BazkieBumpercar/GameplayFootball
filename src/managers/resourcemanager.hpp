// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MANAGERS_RESOURCE
#define _HPP_MANAGERS_RESOURCE

#include "defines.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

#include "types/lockable.hpp"
#include "types/loader.hpp"

#include "managers/environmentmanager.hpp"

namespace blunted {

  template <typename T>
  class Resource;

  template <typename T>
  class ResourceManager {

    public:
      ResourceManager(const std::string &typeDescription) : typeDescription(typeDescription) {};

      ~ResourceManager() {
        resources.Lock();
        resources.data.clear();
        resources.Unlock();
        loaders.clear();
      };

      void RegisterLoader(const std::string &extension, Loader<T> *loader) {
        //printf("registering loader for extension %s\n", extension.c_str());
        loaders.insert(std::make_pair(extension, loader));
      }

      boost::intrusive_ptr < Resource<T> > Fetch(const std::string &filename, bool load = true, bool useExisting = true) {
        bool foo;
        return Fetch(filename, load, foo, useExisting);
      }

      boost::intrusive_ptr < Resource<T> > Fetch(const std::string &filename, bool load, bool &alreadyThere, bool useExisting) {
        std::string adaptedFilename = get_file_name(filename);

        // resource already loaded?

        bool success = false;
        boost::intrusive_ptr < Resource<T> > foundResource;

        if (useExisting) {
          foundResource = Find(adaptedFilename, success);
        }

        if (success) {
          // resource is already there! w00t, that'll win us some cycles
          // (or user wants a new copy)

          alreadyThere = true;

          return foundResource;
        }

        else {

          // create resource

          alreadyThere = false;
          boost::intrusive_ptr < Resource <T> > resource(new Resource<T>(adaptedFilename));

          // try to load

          if (load) {
            std::string extension = get_file_extension(filename);
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            typename std::map < std::string, Loader<T>* >::iterator iter = loaders.find(extension);
            if (iter != loaders.end()) {
              (*iter).second->Load(filename, resource);
            } else {
              Log(e_FatalError, "ResourceManager<>", "Load", "There is no loader for " + filename);
            }
          }
          Register(resource);
          return resource;
        }
      }

      boost::intrusive_ptr < Resource<T> > FetchCopy(const std::string &filename, const std::string &newName) {
        boost::intrusive_ptr < Resource<T> > resource = Fetch(filename);

        // todo: this lock is probably needed, but slows down everything very much.. find out why
        resource->resourceMutex.lock();
        boost::intrusive_ptr < Resource<T> > resourceCopy(new Resource<T>(*resource, newName));
        resource->resourceMutex.unlock();

        Register(resourceCopy);
        return resourceCopy;
      }

      boost::intrusive_ptr < Resource<T> > FetchCopy(const std::string &filename, const std::string &newName, bool &alreadyThere) {
        boost::intrusive_ptr < Resource<T> > resourceCopy;
        if (resources.data.find(newName) != resources.data.end()) {
          //Log(e_Warning, "ResourceManager", "FetchCopy", "Duplicate key '" + newName + "' - returning existing resource instead of copy (maybe just use Fetch() instead?)");
          resourceCopy = Fetch(newName, false, true);
        } else {
          boost::intrusive_ptr < Resource<T> > resource = Fetch(filename, true, alreadyThere, true);

          // todo: this lock is probably needed, but slows down everything very much.. find out why
          resource->resourceMutex.lock();
          resourceCopy = boost::intrusive_ptr < Resource<T> >(new Resource<T>(*resource, newName));
          resource->resourceMutex.unlock();

          Register(resourceCopy);
        }

        return resourceCopy;
      }

      void RemoveUnused() {
        // periodically execute this cleanup code somewhere
        // currently invoked from scheduler, could be a user task?
        // as if it were a service..
        // would be slower, but somewhat cooler :p

        // cleanup

        resources.Lock();

        typename std::map < std::string, boost::intrusive_ptr< Resource<T> > >::iterator resIter = resources.data.begin();
        while (resIter != resources.data.end()) {
          if (resIter->second->GetRefCount() == 1) {
            //printf("removing unused %s resource '%s'\n", typeDescription.c_str(), resIter->second->GetIdentString().c_str());
            resources.data.erase(resIter++);
          } else {
            ++resIter;
          }
        }

        resources.Unlock();
      }

    protected:

      boost::intrusive_ptr < Resource<T> > Find(const std::string &identString, bool &success) {
        resources.Lock();
        typename std::map < std::string, boost::intrusive_ptr< Resource<T> > >::iterator resIter = resources.data.find(identString);
        if (resIter != resources.data.end()) {
          success = true;
          boost::intrusive_ptr < Resource<T> > resource = (*resIter).second;
          resources.Unlock();
          return resource;
        } else {
          success = false;
          resources.Unlock();
          return boost::intrusive_ptr < Resource<T> >();
        }
      }

      void Register(boost::intrusive_ptr < Resource<T> > resource) {

        resources.Lock();

        //printf("registering %s\n", resource->GetIdentString().c_str());
        if (resources.data.find(resource->GetIdentString()) != resources.data.end()) {
          resources.Unlock(); RemoveUnused(); resources.Lock();
          if (resources.data.find(resource->GetIdentString()) != resources.data.end()) {
            Log(e_FatalError, "ResourceManager", "Register", "Duplicate key '" + resource->GetIdentString() + "'");
          }
        }
        resources.data.insert(std::make_pair(resource->GetIdentString(), resource));

        resources.Unlock();
      }

      std::map < std::string, Loader<T>* > loaders;

      Lockable < std::map < std::string, boost::intrusive_ptr < Resource <T> > > > resources;

      std::string typeDescription;

    private:

  };

}

#endif
