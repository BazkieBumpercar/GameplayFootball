// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SCENE
#define _HPP_SCENE

#include "defines.hpp"

#include "iscene.hpp"

#include "types/lockable.hpp"

namespace blunted {

  typedef Lockable < std::vector < boost::intrusive_ptr<Object> > > vector_Objects;

  class Scene : public IScene {

    public:
      Scene(std::string name, e_SceneType sceneType);
      virtual ~Scene();

      virtual void Init() = 0; // ATOMIC
      virtual void Exit(); // ATOMIC

      virtual void CreateSystemObjects(boost::intrusive_ptr<Object> object);

      virtual const std::string GetName() const;
      virtual e_SceneType GetSceneType() const;

      virtual bool SupportedObjectType(e_ObjectType objectType) const;

    protected:
      std::string name;

      e_SceneType sceneType;

      std::vector<e_ObjectType> supportedObjectTypes;

  };

}

#endif
