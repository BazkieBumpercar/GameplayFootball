// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_UTILS_THREADHUD
#define _HPP_UTILS_THREADHUD

#include "managers/resourcemanager.hpp"
#include "types/thread.hpp"
#include "scene/resources/surface.hpp"
#include "scene/scene2d/scene2d.hpp"
#include "scene/objects/image2d.hpp"

#include <SDL/SDL_ttf.h>

namespace blunted {

  struct WorkerThreadItem {
    unsigned long beginTime_ms;
    unsigned long endTime_ms;
    e_ThreadState state;
    std::string commandName;
  };

  class ThreadHud {

    public:
      ThreadHud(boost::shared_ptr<Scene2D> scene2D);
      virtual ~ThreadHud();

      void Execute();

      void Redraw(bool forceRedraw = false);

    protected:
      boost::shared_ptr<Scene2D> scene2D;

      std::vector < std::vector<WorkerThreadItem> > workerThreadHistory; // vector content is guaranteed to be chronologic
      boost::intrusive_ptr<Image2D> graph;

      int skipRedrawCounter;

      int _width;
      int _headerheight;
      int _lineheight;
      int _height;

      bool gui_autoPlay;
      unsigned long gui_focusTime_ms;
      float gui_zoomLevel;
      float gui_zoomMomentum;

      TTF_Font *font;

  };

}

#endif
