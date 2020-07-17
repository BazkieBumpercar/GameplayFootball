// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_UTILS_OBJECTLOADER
#define _HPP_UTILS_OBJECTLOADER

#include "scene/scene3d/scene3d.hpp"
#include "scene/scene3d/node.hpp"
#include "utils/xmlloader.hpp"

namespace blunted {

  class ObjectLoader {

    public:
      ObjectLoader();
      ~ObjectLoader();

      // depricated: LoadObject now recurses, this would take away the need for a level->objects hierarchy
      boost::intrusive_ptr<Node> LoadLevel(boost::shared_ptr<Scene3D> scene3D, const std::string &filename) const;

      boost::intrusive_ptr<Node> LoadObject(boost::shared_ptr<Scene3D> scene3D, const std::string &filename, const Vector3 &offset = Vector3(0)) const;

    protected:
      boost::intrusive_ptr<Node> LoadObjectImpl(boost::shared_ptr<Scene3D> scene3D, const std::string &nodename, const XMLTree &objectTree, const Vector3 &offset) const;

      void InterpretProperties(const map_XMLTree &tree, Properties &properties) const;
      e_LocalMode InterpretLocalMode(const std::string &value) const;

  };

}

#endif
