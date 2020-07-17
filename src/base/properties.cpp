// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "properties.hpp"

#include "defines.hpp"
#include "utils.hpp"
#include "log.hpp"

#include <fstream>
#include <iostream>

using namespace std;

namespace blunted {

  std::string Properties::emptyString = "";

  Properties::Properties() {
    cacheString.reserve(256);
  }

  Properties::~Properties() {
  }

  bool Properties::Exists(const char *name) const {
    map_Properties::const_iterator iter = properties.find(name);
    if (iter == properties.end()) {
      return false;
    } else {
      return true;
    }
  }

  void Properties::Set(const char *name, const std::string &value) {
    std::pair <map_Properties::iterator, bool> result = properties.insert(map_Properties::value_type(name, value));
    if (result.second == false) {
      // property already exists, replace its value
      result.first->second = value;
    }
  }

  void Properties::Set(const char *name, real value) {
    std::string value_str = real_to_str(value);
    Set(name, value_str);
  }

  void Properties::SetBool(const char *name, bool value) {
    std::string value_str = value ? "true" : "false";
    Set(name, value_str);
  }

  const std::string &Properties::Get(const char *name, const std::string &defaultValue) const {
    cacheString.assign(name);
    map_Properties::const_iterator iter = properties.find(cacheString);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      return iter->second;
    }
  }

  bool Properties::GetBool(const char *name, bool defaultValue) const {
    cacheString.assign(name);
    map_Properties::const_iterator iter = properties.find(cacheString);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      if (iter->second.compare("true") == 0) return true; else return false;
    }
  }

  real Properties::GetReal(const char *name, real defaultValue) const {
    cacheString.assign(name);
    map_Properties::const_iterator iter = properties.find(cacheString);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      return atof(iter->second.c_str());
    }
  }

  int Properties::GetInt(const char *name, int defaultValue) const {
    cacheString.assign(name);
    map_Properties::const_iterator iter = properties.find(cacheString);
    if (iter == properties.end()) {
      return defaultValue;
    } else {
      return int(floor(atof(iter->second.c_str())));
    }
  }

  Vector3 Properties::GetVector3(const char *name, Vector3 defaultValue) const {
    if (Exists(name)) {
      const std::string &str = Get(name);
      return GetVectorFromString(str);
    } else {
      return defaultValue;
    }
  }

  void Properties::AddProperties(const Properties *userprops) {
    if (!userprops) return;

    const map_Properties *userpropdata = userprops->GetProperties();
    map_Properties::const_iterator iter = userpropdata->begin();

    while (iter != userpropdata->end()) {
      std::pair <map_Properties::iterator, bool> result = properties.insert(map_Properties::value_type(iter->first, iter->second));
      if (result.second == false) {
        Log(e_Warning, "Properties", "AddProperties", "property " + iter->first + " already defined");
      }
      iter++;
    }
  }

  void Properties::AddProperties(const Properties &userprops) {
    const map_Properties *userpropdata = userprops.GetProperties();
    map_Properties::const_iterator iter = userpropdata->begin();

    while (iter != userpropdata->end()) {
      std::pair <map_Properties::iterator, bool> result = properties.insert(map_Properties::value_type(iter->first, iter->second));
      if (result.second == false) {
        Log(e_Warning, "Properties", "AddProperties", "property " + iter->first + " already defined");
      }
      iter++;
    }
  }

  const map_Properties *Properties::GetProperties() const {
    return &properties;
  }

  void Properties::LoadFile(const std::string &filename) {
    std::vector <std::string> content;
    file_to_vector(filename, content);

    int contentSize = content.size();
    for (int i = 0; i < contentSize; i++) {
      int param_start = content.at(i).find_first_of('"') + 1;
      int param_end = content.at(i).find_first_of('"', param_start);
      int value_start = content.at(i).find_first_of('"', param_end + 1) + 1;
      int value_end = content.at(i).find_first_of('"', value_start);

      std::string param, value;

      if (param_start < param_end && value_start < value_end && param_end < value_start) {
        param = content.at(i).substr(param_start, param_end - param_start);
        value = content.at(i).substr(value_start, value_end - value_start);
        this->Set(param.c_str(), value);
      }
    }
  }

  void Properties::SaveFile(const std::string &filename) const {
    ofstream cfile;
    cfile.open(filename.c_str(), ios::out);
    map_Properties::const_iterator iter = properties.begin();
    while (iter != properties.end()) {
      std::string bla = "\"" + iter->first + "\" \"" + iter->second + "\"\n";
      cfile << bla.c_str();

      iter++;
    }
    cfile.close();
  }

  void Properties::Print() const {
    map_Properties::const_iterator iter = properties.begin();
    while (iter != properties.end()) {
      std::string bla = "\"" + iter->first + "\" \"" + iter->second + "\"\n";
      cout << bla.c_str();

      iter++;
    }
  }

}
