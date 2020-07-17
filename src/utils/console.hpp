// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_UTILS_CONSOLE
#define _HPP_UTILS_CONSOLE

#include "defines.hpp"
#include "text2d.hpp"
#include "scene/scene2d/scene2d.hpp"

namespace blunted {

  class Console {

    public:
      Console(boost::shared_ptr<Scene2D> scene2D, int numlines = 6);
      ~Console();

      // accepts values from base/log.hpp
      void PrintLog(e_LogType logType, std::string className, std::string methodName, std::string message);
      void Print(const std::string &string, const Vector3 &color);

    protected:
      void UpdateImages();
      int CalculateY(int lineNum);

      boost::shared_ptr<Scene2D> scene2D;

      Text2D *text2D;

      std::vector < boost::intrusive_ptr<Image2D> > images;
      std::vector < std::string > lines;
      std::vector < Vector3> colors;

      boost::intrusive_ptr<Image2D> input;

      int lineheight;
      int numlines;

      boost::signals2::connection logConn;

  };

}

#endif
