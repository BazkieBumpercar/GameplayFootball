// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "threadhud.hpp"

#include "managers/taskmanager.hpp"
#include "base/utils.hpp"
#include "scene/objectfactory.hpp"
#include "managers/systemmanager.hpp"
#include "managers/resourcemanagerpool.hpp"
#include "managers/usereventmanager.hpp"

#include "blunted.hpp"

namespace blunted {

  ThreadHud::ThreadHud(boost::shared_ptr<Scene2D> scene2D) : scene2D(scene2D) {

    font = TTF_OpenFont("media/blunted/HUD/terminus.fon", 8);
    assert(font);

    gui_autoPlay = true;
    gui_focusTime_ms = EnvironmentManager::GetInstance().GetTime_ms();
    gui_zoomLevel = 0.5f;
    gui_zoomMomentum = 0.0f;

    Vector3 contextSize = scene2D->GetContextSize();

    int x = 10;

    _width = contextSize.coords[0] - x * 2;
    _lineheight = 20;
    _headerheight = 30;

    skipRedrawCounter = 0;

    TaskManager *taskManager = TaskManager::GetInstancePtr();
    unsigned int workerThreadNum = taskManager->GetWorkerThreadCount();
    _height = _headerheight + _lineheight * workerThreadNum;

    int y = contextSize.coords[1] - _height - 10;


    graph = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("threadHUD graphic", e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(graph);

    boost::intrusive_ptr < Resource<Surface> > surface = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("threadHUD graphic", false, false);
    SDL_Surface *surfaceSDL = CreateSDLSurface(_width, _height);
    surface->GetResource()->SetData(surfaceSDL);

    graph->SetImage(surface);
    graph->SetPosition(x, y);

    scene2D->AddObject(graph);


    // create empty entries in workerThreadHistory

    for (unsigned int thread = 0; thread < workerThreadNum; thread++) {
      workerThreadHistory.push_back(std::vector<WorkerThreadItem>());
    }

  }

  ThreadHud::~ThreadHud() {
    //for (unsigned int i = 0; i < HUDLines.size(); i++) {
      //HUDLines.at(i)->Exit();
      scene2D->DeleteObject(graph);//HUDLines.at(i));
    //}
    //HUDLines.clear();
    workerThreadHistory.clear();

    TTF_CloseFont(font);
  }

  void ThreadHud::Execute() {

    unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();

    TaskManager *taskManager = TaskManager::GetInstancePtr();
    unsigned int workerThreadNum = taskManager->GetWorkerThreadCount();
    assert(workerThreadNum == workerThreadHistory.size()); // this class needs work if we ever decide to make the number of worker threads dynamic

    for (unsigned int thread = 0; thread < workerThreadNum; thread++) {

      e_ThreadState state = e_ThreadState_Idle;
      std::string commandName;
      taskManager->GetFullWorkerThreadState(thread, state, commandName);

      // check for change
      bool isChanged = false;
      if (!workerThreadHistory.at(thread).empty()) {
        WorkerThreadItem &currentItem = workerThreadHistory.at(thread).back();

        currentItem.endTime_ms = time_ms; // maybe not actually ended yet, but 'ended for now' since we cannot foresee the future. this way at least it can be displayed.

        if (commandName.compare(currentItem.commandName) != 0) {
          // close current
          isChanged = true;
        }
      } else {
        // empty
        isChanged = true;
      }

      if (isChanged) {
        WorkerThreadItem item;
        item.state = state;
        item.commandName = commandName;
        item.beginTime_ms = time_ms;
        workerThreadHistory.at(thread).push_back(item);
      }

      // clean old items
      std::vector<WorkerThreadItem>::iterator iter = workerThreadHistory.at(thread).begin();
      while (iter != workerThreadHistory.at(thread).end()) {
        if (iter->endTime_ms + 5000 < gui_focusTime_ms) {
          iter = workerThreadHistory.at(thread).erase(iter);
        } else {
          break; // no more old records are going to show up, are there?
        }
      }

      //printf("%i: %i %s\n", thread, state, commandName.c_str());
    }

    bool forceRedraw = false;

    UserEventManager *eventManager = UserEventManager::GetInstancePtr();
    if (eventManager->GetKeyboardState(SDLK_F4)) {
      gui_autoPlay ? gui_autoPlay = false : gui_autoPlay = true; gui_focusTime_ms = time_ms;

      eventManager->SetKeyboardState(SDLK_F4, false);
      forceRedraw = true;
    }
    if (eventManager->GetKeyboardState(SDLK_F5)) {
      gui_focusTime_ms -= ceil(20 * gui_zoomLevel);
      eventManager->SetKeyboardState(SDLK_F5, false);
      forceRedraw = true;
    }
    if (eventManager->GetKeyboardState(SDLK_F6)) {
      gui_focusTime_ms += ceil(20 * gui_zoomLevel);
      eventManager->SetKeyboardState(SDLK_F6, false);
      forceRedraw = true;
    }
    if (eventManager->GetKeyboardState(SDLK_F7)) {
      gui_zoomMomentum = -0.005f;
      //eventManager->SetKeyboardState(SDLK_F7, false);
      forceRedraw = true;
    }
    if (eventManager->GetKeyboardState(SDLK_F8)) {
      gui_zoomMomentum = 0.005f;
      //eventManager->SetKeyboardState(SDLK_F8, false);
      forceRedraw = true;
    }

    //gui_zoomMomentum = clamp(zoomMomentum, 0.5f, 2.0f);

    gui_zoomMomentum *= 0.95f;
    gui_zoomLevel *= 1.0f + gui_zoomMomentum;

    Redraw(forceRedraw);
  }

  void ThreadHud::Redraw(bool forceRedraw) {

    if (skipRedrawCounter == 0 || forceRedraw) {

      TaskManager *taskManager = TaskManager::GetInstancePtr();
      unsigned int workerThreadNum = taskManager->GetWorkerThreadCount();

      //graph->DrawRectangle(0, 0, _width, _height, Vector3(0), 255); // clear first
      graph->DrawRectangle(0, 0, _width, _height, Vector3(0), 50);

      if (gui_autoPlay) gui_focusTime_ms = EnvironmentManager::GetInstance().GetTime_ms();

      unsigned long gui_beginTime_ms = gui_focusTime_ms - gui_zoomLevel * _width * 0.5f;
      unsigned long gui_endTime_ms = gui_focusTime_ms + gui_zoomLevel * _width * 0.5f;

      graph->DrawSimpleText("time: " + int_to_str(gui_beginTime_ms) + " ms", 5, 5, font, Vector3(200, 200, 200), 255);
      graph->DrawSimpleText("range: " + int_to_str(gui_endTime_ms - gui_beginTime_ms) + " ms", _width * 0.5 - 50, 5, font, Vector3(200, 200, 200), 255);
      graph->DrawSimpleText("time: " + int_to_str(gui_endTime_ms) + " ms", _width - 100, 5, font, Vector3(200, 200, 200), 255);

      for (unsigned int thread = 0; thread < workerThreadNum; thread++) {

        for (unsigned int entry = 0; entry < workerThreadHistory.at(thread).size(); entry++) {
          // visible?
          if (workerThreadHistory.at(thread).at(entry).state != e_ThreadState_Idle) {
            if (workerThreadHistory.at(thread).at(entry).beginTime_ms <= gui_endTime_ms && workerThreadHistory.at(thread).at(entry).endTime_ms >= gui_beginTime_ms) {

              // calculate min/max visible time of this item
              int beginDisplayTime_ms = std::max(workerThreadHistory.at(thread).at(entry).beginTime_ms, gui_beginTime_ms);
              int endDisplayTime_ms = std::min(workerThreadHistory.at(thread).at(entry).endTime_ms, gui_endTime_ms);

              // calculate coords
              int range_ms = gui_endTime_ms - gui_beginTime_ms;
              float x1percentage = (beginDisplayTime_ms - gui_beginTime_ms) / (float)range_ms;
              float x2percentage = (endDisplayTime_ms - gui_beginTime_ms) / (float)range_ms;
              //printf("%f, %f\n", x1percentage, x2percentage);
              int x1 = int(floor(x1percentage * _width));
              int x2 = int(floor(x2percentage * _width));
              int y1 = int(floor((thread + 0.2f) * _lineheight)) + _headerheight;
              int y2 = int(floor((thread + 0.8f) * _lineheight)) + _headerheight;
              int w = x2 - x1;
              int h = y2 - y1;

              unsigned char r = GetHashFromCharString((workerThreadHistory.at(thread).at(entry).commandName + "blabliebloe").c_str());
              unsigned char g = GetHashFromCharString((workerThreadHistory.at(thread).at(entry).commandName + "pirpriii").c_str());
              unsigned char b = GetHashFromCharString((workerThreadHistory.at(thread).at(entry).commandName + "poing").c_str());
              //printf("%i %i %i\n", r, g, b);
              Vector3 color(r, g, b);
              color = color * 0.5f + Vector3(127);
              //color.Print();
              graph->DrawRectangle(x1, y1, w, h, color, 255);
              Vector3 textcolor(256 - r, 256 - g, 256 - b);
              textcolor = textcolor * 0.3f;
              //textcolor.Print();

              if (x2percentage - x1percentage > 0.05) {
                graph->DrawSimpleText(workerThreadHistory.at(thread).at(entry).commandName, x1, y1, font, textcolor, 255);
              }
            }
          }
        }

      }

      graph->OnChange();
    }

    skipRedrawCounter++;
    if (skipRedrawCounter > 50) skipRedrawCounter = 0;
  }

}
