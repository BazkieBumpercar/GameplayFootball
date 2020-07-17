// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_LOADERS_ASE
#define _HPP_LOADERS_ASE

#include "defines.hpp"
#include "base/utils.hpp"
#include "managers/resourcemanager.hpp"
#include "scene/resources/geometrydata.hpp"
#include "scene/objects/geometry.hpp"

namespace blunted {

  struct s_Material {
    std::string maps[4];
    std::string shininess;
    std::string specular_amount;
    Vector3 self_illumination;
  };

  class ASELoader : public Loader<GeometryData> {

    public:
      ASELoader();
      virtual ~ASELoader();

      // ----- encapsulating load function
      virtual void Load(std::string filename, boost::intrusive_ptr < Resource <GeometryData> > resource);

      // ----- interpreter for the .ase treedata
      void Build(const s_tree *data, boost::intrusive_ptr < Resource <GeometryData> > resource);

      // ----- per-object interpreters
      void BuildTriangleMesh(const s_tree *data, boost::intrusive_ptr < Resource <GeometryData> > resource, std::vector <s_Material> materialList);

    protected:

      int triangleCount;

  };

}

#endif
