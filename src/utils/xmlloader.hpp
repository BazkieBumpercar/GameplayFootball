// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_UTILS_XMLLOADER
#define _HPP_UTILS_XMLLOADER

#include "defines.hpp"

namespace blunted {

  struct XMLTree;

  typedef std::multimap<std::string, XMLTree> map_XMLTree;

  struct XMLTree {
    std::string value;
    map_XMLTree children;
  };

  class XMLLoader {

    public:
      XMLLoader();
      ~XMLLoader();

      XMLTree LoadFile(const std::string &filename);
      XMLTree Load(const std::string &file);
      void Save(XMLTree &source, const std::string &filename);
      std::string GetSource(const XMLTree &source, int depth = 0) const;
      void PrintTree(const XMLTree &source) const;

    protected:
      void BuildTree(XMLTree &tree, const std::string &source);

  };

}

#endif
