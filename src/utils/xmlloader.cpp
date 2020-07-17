// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "xmlloader.hpp"

#include "base/utils.hpp"
#include "base/log.hpp"

namespace blunted {

  XMLLoader::XMLLoader() {
  }

  XMLLoader::~XMLLoader() {
  }


  XMLTree XMLLoader::LoadFile(const std::string &filename) {
    std::string source;
    source = file_to_string(filename);

    XMLTree tree;
    BuildTree(tree, source);

    return tree;
  }

  XMLTree XMLLoader::Load(const std::string &file) {
    XMLTree tree;
    BuildTree(tree, file);

    return tree;
  }

  void XMLLoader::Save(XMLTree &source, const std::string &filename) {
  }

  std::string XMLLoader::GetSource(const XMLTree &source, int depth) const {
    std::string result;

    std::multimap<std::string, XMLTree>::const_iterator iter = source.children.begin();
    if (iter == source.children.end()) {
      if (source.value != "") {
        for (int i = 0; i < depth; i++) result.append("\t");
        result.append(source.value);
        result.append("\n");
      }
    } else {
      while (iter != source.children.end()) {
        for (int i = 0; i < depth; i++) result.append("\t");
        result.append("<" + (*iter).first + ">\n");

        result.append(GetSource((*iter).second, depth + 1));

        for (int i = 0; i < depth; i++) result.append("\t");
        result.append("</" + (*iter).first + ">\n");

        iter++;
      }
    }

    return result;
  }

  void XMLLoader::PrintTree(const XMLTree &source) const {
    /*std::multimap<std::string, XMLTree>::const_iterator iter = source.children.begin();
    if (iter == source.children.end()) {
      if (source.value != "") {
        for (int i = 0; i < depth; i++) printf("\t");
        printf("'%s'\n", source.value.c_str());
      }
    } else {
      while (iter != source.children.end()) {
        for (int i = 0; i < depth; i++) printf("\t");
        printf("<%s>\n", (*iter).first.c_str());

        PrintTree((*iter).second, depth + 1);

        for (int i = 0; i < depth; i++) printf("\t");
        printf("</%s>\n", (*iter).first.c_str());

        iter++;
      }
    }*/
    printf("%s\n", GetSource(source).c_str());
  }

  void XMLLoader::BuildTree(XMLTree &tree, const std::string &source) {

    size_t index_end = 0;
    size_t index = source.find("<", 0);

    if (index == std::string::npos) {
      // no tags, must be a value
      tree.value = source;
      tree.value.erase(remove_if(tree.value.begin(), tree.value.end(), isspace), tree.value.end());
      //printf("value: '%s'\n", source.c_str());
      return;
    }


    // a tag (or multiple), so must contain children

    while (index != std::string::npos) {
      index_end = source.find(">", index);
      std::string tag = source.substr(index + 1, index_end - index - 1);
      //printf("tag: '%s'\n", tag.c_str());
      index = index_end;
      // index is now directly behind opening tag

      // find closing tag
      int recurse_counter = 1;
      size_t index_nexttag_open = 0;
      size_t index_nexttag_close = 0;
      while (recurse_counter != 0) {
        index_nexttag_open = source.find("<" + tag + ">", index_end + 1);
        index_nexttag_close = source.find("</" + tag + ">", index_end + 1);
        //printf("%i %i\n", index_nexttag_open, index_nexttag_close);
        if (index_nexttag_open > index_nexttag_close || index_nexttag_open == std::string::npos) {
          recurse_counter--;
          index_end = index_nexttag_close;
        } else {
          recurse_counter++;
          index_end = index_nexttag_open;
        }
        //printf("%i\n", recurse_counter);
        if (index_end == std::string::npos) {
          Log(e_FatalError, "XMLLoader", "BuildTree", "No closing tag found for <" + tag + ">");
        }
      }

      std::string data = source.substr(index + 1, index_end - index - 1);
      //printf("data: '%s'\n", data.c_str());

      XMLTree child;
      BuildTree(child, data);
      tree.children.insert(std::make_pair(tag, child));

      // close
      index = source.find(">", index_end);

      // find next tag
      index = source.find("<", index);
    }
  }

}
