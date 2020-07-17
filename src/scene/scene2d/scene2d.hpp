// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_SCENE2D
#define _HPP_SCENE2D

#include "defines.hpp"

#include "../scene.hpp"
#include "../object.hpp"
#include "types/lockable.hpp"
#include "base/properties.hpp"

namespace blunted {

  class Scene2D : public Scene {

    public:
      Scene2D(const std::string &name, const Properties &config);
      virtual ~Scene2D();

      virtual void Init();
      virtual void Exit(); // ATOMIC

      void AddObject(boost::intrusive_ptr<Object> object);
      void DeleteObject(boost::intrusive_ptr<Object> object);
      void RemoveObject(boost::intrusive_ptr<Object> object);

      void GetObjects(e_ObjectType targetObjectType, std::vector < boost::intrusive_ptr<Object> > &gatherObjects);
      void PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType);

      void GetContextSize(int &width, int &height, int &bpp);
      Vector3 GetContextSize();

    protected:
      vector_Objects objects;

      int width, height, bpp;

  };

  class IScene2DInterpreter : public ISceneInterpreter {

    public:
      virtual void OnLoad() = 0;
      virtual void OnUnload() = 0;

    protected:

  };

}

#endif
