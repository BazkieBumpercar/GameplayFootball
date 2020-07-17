// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_RESOURCE
#define _HPP_RESOURCE

#include "defines.hpp"

#include "managers/resourcemanager.hpp"

#include "types/refcounted.hpp"

namespace blunted {

  enum e_ResourceType {
    e_ResourceType_GeometryData = 1,
    e_ResourceType_Surface = 2,
    e_ResourceType_Texture = 3,
    e_ResourceType_VertexBuffer = 4,
    e_ResourceType_SoundBuffer = 5,
    e_ResourceType_AudioSoundBuffer = 6
  };

  template <typename T>
  class Resource : public RefCounted {

    public:
      Resource(std::string identString) : resource(0), identString(identString) {
        resource = new T();
      }

      virtual ~Resource() {
        delete resource;
        resource = 0;
      }

      Resource(const Resource &src, const std::string &identString) : identString(identString) {
        //src.resourceMutex.lock(); terribly slow, why? (note: moved to resourcemanager's fetchcopy)
        this->resource = new T(*src.resource);
        //src.resourceMutex.unlock();
      }

      // todo: LoadStream / SaveStream

      T *GetResource() {
        return resource;
      }

      std::string GetIdentString() {
        return identString;
      }

      mutable boost::mutex resourceMutex;

      T *resource;

    protected:
      const std::string identString;

  };

}

#endif
