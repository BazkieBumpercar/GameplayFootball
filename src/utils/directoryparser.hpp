// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_UTILS_DIRECTORYPARSER
#define _HPP_UTILS_DIRECTORYPARSER

#include "defines.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"

#include "base/log.hpp"

namespace fs = boost::filesystem;

namespace blunted {

  class DirectoryParser {

    public:
      DirectoryParser();
      virtual ~DirectoryParser();

      void Parse(boost::filesystem::path path, const std::string &extension, std::vector<std::string> &files, bool recurse = true);

    protected:

  };

}

#endif
