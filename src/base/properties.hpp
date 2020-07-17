// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_BASE_PROPERTIES
#define _HPP_BASE_PROPERTIES

#include "defines.hpp"

#include "base/math/vector3.hpp"

namespace blunted {

  typedef std::map <const std::string, std::string> map_Properties;

  class Vector3;

  class Properties {

    public:
      Properties();
      virtual ~Properties();

      bool Exists(const char *name) const;

      void Set(const char *name, const std::string &value);
      void Set(const char *name, real value);
      void SetBool(const char *name, bool value);
      const std::string &Get(const char *name, const std::string &defaultValue = emptyString) const;
      bool GetBool(const char *name, bool defaultValue = false) const;
      real GetReal(const char *name, real defaultValue = 0) const;
      int GetInt(const char *name, int defaultValue = 0) const;
      Vector3 GetVector3(const char *name, Vector3 defaultValue = Vector3(0, 0, 0)) const;
      void AddProperties(const Properties *userprops);
      void AddProperties(const Properties &userprops);
      const map_Properties *GetProperties() const;

      void LoadFile(const std::string &filename);
      void SaveFile(const std::string &filename) const;
      void Print() const;

    protected:
      map_Properties properties;

      static std::string emptyString;
      mutable std::string cacheString;

  };

}

#endif
