// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_DATABASE
#define _HPP_DATABASE

#include "defines.hpp"

struct sqlite3;

namespace blunted {

  class DatabaseResult;

  class Database {

    public:
      Database();
      virtual ~Database();

      bool Load(const std::string &filename);
      DatabaseResult *Query(const std::string &query);

    protected:
      sqlite3 *db;

  };

  class DatabaseResult {

    public:
      std::vector < std::string > header;
      std::vector < std::vector < std::string > > data;

  };

}

#endif
