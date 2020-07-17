// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "directoryparser.hpp"

#include "base/log.hpp"

namespace fs = boost::filesystem;

namespace blunted {

  DirectoryParser::DirectoryParser() {
  }

  DirectoryParser::~DirectoryParser() {
  }

  void DirectoryParser::Parse(boost::filesystem::path path, const std::string &extension, std::vector<std::string> &files, bool recurse) {

    if (!fs::exists(path) || !fs::is_directory(path)) Log(e_Error, "DirectoryParser", "Parse", "Could not open directory " + path.string() + " for reading");

    fs::directory_iterator dirIter(path);
    fs::directory_iterator endIter;
    while (dirIter != endIter) {
      if (is_directory(dirIter->status())) {

        if (recurse) {
          boost::filesystem::path thePath(path);
          thePath /= dirIter->path().filename();
          Parse(thePath, extension, files);
        }

      } else {
        boost::filesystem::path thePath(path);
        thePath /= dirIter->path().filename();

        if (thePath.extension() == "." + extension) {

          // add to results
          //printf("adding %s\n", dirIter->path().filename().c_str());
          files.push_back(path.string() + "/" + thePath.filename().string());

        }
      }

      dirIter++;
    }
  }

}
