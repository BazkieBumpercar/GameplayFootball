// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GUI
#define _HPP_GUI

#include "types/iusertask.hpp"

#include "scene/scene2d/scene2d.hpp"

#include "guiinterface.hpp"

namespace blunted {

  class GuiTask : public IUserTask {

    public:
      GuiTask(boost::shared_ptr<Scene2D> scene2D, float ratio, int margin);
      virtual ~GuiTask();

      virtual std::string GetName() const { return "guitask"; }

      virtual void GetPhase();
      virtual void ProcessPhase();
      virtual void PutPhase();

      GuiInterface *GetInterface();

    protected:
      boost::shared_ptr<Scene2D> scene2D;

      GuiInterface *guiInterface;

      float ratio;
      int margin;

  };

}

#endif
