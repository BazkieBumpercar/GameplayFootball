// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "timeline.hpp"

#include "base/sdl_surface.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "scene/objectfactory.hpp"

#include "base/math/bluntmath.hpp"

namespace blunted {

  GuiTimeline::GuiTimeline(boost::shared_ptr<Scene2D> scene2D, const std::string &name, float x1_percent, float y1_percent, float x2_percent, float y2_percent) : GuiView(scene2D, name, x1_percent, y1_percent, x2_percent, y2_percent) {
    focussed = false;
    selected = 0;
    selectedFrame = 0;
    frameCount = 25;
    grid = 1;
  }

  GuiTimeline::~GuiTimeline() {
    scene2D->DeleteObject(timeline);
    timeline.reset();
  }


  void GuiTimeline::Init() {
    SDL_Surface *sdlSurface = CreateSDLSurface(GetX(x2_percent) - GetX(x1_percent), GetY(y2_percent) - GetY(y1_percent));

    boost::intrusive_ptr < Resource<Surface> > timelineResource = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("Timeline: " + GetName(), false, false);
    Surface *surface = timelineResource->GetResource();

    surface->SetData(sdlSurface);

    timeline = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("Timeline: " + GetName(), e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(timeline);
    timeline->SetImage(timelineResource);

    scene2D->AddObject(timeline);
    timeline->SetPosition(GetX(x1_percent), GetY(y1_percent));

    Redraw();
  }


  void GuiTimeline::AddPart(const std::string id, const std::string caption) {
    TimelinePart part;
    part.name = id;

    for (int i = 0; i < (signed int)parts.size(); i++) {
      DeleteView(parts.at(i).caption);
    }

    parts.push_back(part);

    float height = (y2_percent - y1_percent) / (parts.size() * 1.0) * 1.3;

    for (int i = 0; i < (signed int)parts.size(); i++) {
      float y = y1_percent + ((y2_percent - y1_percent) / (parts.size() * 1.0)) * i;
      TimelinePart &oldPart = parts.at(i);
      oldPart.caption = new GuiCaption(scene2D, parts.at(i).name + int_to_str(parts.size()), x1_percent, y, x2_percent, y + height, parts.at(i).name);
      //printf("%f %f %f %f\n", x1_percent, y, x2_percent, y + height);
      this->AddView(oldPart.caption);
    }

    selected = parts.size() - 1;
    selectedFrame = 0;
    //ToggleKeyFrame();
    selected = 0;

    Redraw();
  }


  void GuiTimeline::Redraw() {
    boost::intrusive_ptr < Resource<Surface> > timelineResource = timeline->GetImage();
    timelineResource->resourceMutex.lock();

    Surface *surface = timelineResource->GetResource();
    SDL_Surface *sdlSurface = surface->GetData();

    for (int i = 0; i < (signed int)parts.size(); i++) {
      Uint32 color;
      if (is_odd(i + 1)) {
        color = SDL_MapRGBA(sdlSurface->format, 20, 20, 20, 100);
      } else {
        color = SDL_MapRGBA(sdlSurface->format, 0, 0, 0, 100);
      }
      if (focussed && selected == i) {
        color = SDL_MapRGBA(sdlSurface->format, 80, 80, 128, 200);
      }
      sdl_rectangle_filled(sdlSurface, 0,
                                       int((GetY(y2_percent) - GetY(y1_percent)) / (parts.size() * 1.0) * i),
                                       GetX(x2_percent) - GetX(x1_percent),
                                       int((GetY(y2_percent) - GetY(y1_percent)) / (parts.size() * 1.0) * i + (GetY(y2_percent) - GetY(y1_percent)) / (parts.size() * 1.0)), color);
    }


    // grid

    for (int i = 0; i < frameCount / grid; i++) {
      Uint32 color = SDL_MapRGBA(sdlSurface->format, 100, 100, 100, 50);
      int width = GetX(x2_percent) - GetX(x1_percent);
      sdl_line(sdlSurface, int(width * (i * grid / (frameCount * 1.0)) + width / (frameCount / (grid * 1.0) * 2.0)), 0,
                           int(width * (i * grid / (frameCount * 1.0)) + width / (frameCount / (grid * 1.0) * 2.0)), GetY(y2_percent) - GetY(y1_percent), color);
    }


    // keys

    for (int part = 0; part < (signed int)parts.size(); part++) {
      for (int keys = 0; keys < (signed int)parts.at(part).keyFrames.size(); keys++) {
        Uint32 color = SDL_MapRGBA(sdlSurface->format, 0, 255, 128, 200);
        int width = GetX(x2_percent) - GetX(x1_percent);
        int x = int(width * (parts.at(part).keyFrames.at(keys) / (frameCount * 1.0)) + width / (frameCount / (grid * 1.0) * 2.0));
        int y = int((GetY(y2_percent) - GetY(y1_percent)) / (parts.size() * 1.0) * part + (GetY(y2_percent) - GetY(y1_percent)) / (parts.size() * 2.0));
        sdl_rectangle_filled(sdlSurface, x - 3, y - 3, 7, 6, color);
      }
    }


    // cursor

    Uint32 color = SDL_MapRGBA(sdlSurface->format, 255, 255, 255, 200);
    int width = GetX(x2_percent) - GetX(x1_percent);
    sdl_line(sdlSurface, int(width * (selectedFrame / (frameCount * 1.0)) + width / (frameCount / (grid * 1.0) * 2.0)), 0,
                         int(width * (selectedFrame / (frameCount * 1.0)) + width / (frameCount / (grid * 1.0) * 2.0)), GetY(y2_percent) - GetY(y1_percent), color);

    timelineResource->resourceMutex.unlock();

    timeline->OnChange();
  }


  void GuiTimeline::OnFocus() {
    focussed = true;
    selected = 0;
    Redraw();
  }

  void GuiTimeline::OnLoseFocus() {
    focussed = false;
    Redraw();
  }

  void GuiTimeline::OnKey(int sdlkID) {
    if (sdlkID == SDLK_UP) {
      if (selected > 0) {
        selected--;
        Redraw();
      } else {
        parent->SwitchFocus(-1);
      }
    }
    else if (sdlkID == SDLK_DOWN) {
      if (selected < (signed int)parts.size() - 1) {
        selected++;
        Redraw();
      } else {
        parent->SwitchFocus(1);
      }
    }
    else if (sdlkID == SDLK_LEFT) {
      selectedFrame -= grid;
      if (selectedFrame < 0) selectedFrame = frameCount - frameCount % grid - grid; //(frameCount - 1) / grid * grid;
      Redraw();
    }
    else if (sdlkID == SDLK_RIGHT) {
      selectedFrame += grid;
      if (selectedFrame >= frameCount) selectedFrame = 0;
      Redraw();
    }
    else if (sdlkID == SDLK_KP_MINUS) {
      // less frames
      if (frameCount > 10) {
        frameCount -= 1;
        if (selectedFrame >= frameCount) selectedFrame = frameCount - grid;
        Redraw();
      }
    }
    else if (sdlkID == SDLK_KP_PLUS) {
      // more frames
      frameCount += 1;
      Redraw();
    }
    else if (sdlkID == SDLK_LEFTBRACKET) {
      // larger grid
      if (grid < 8) grid *= 2;
      if (selectedFrame % grid != 0) selectedFrame = 0;
      Redraw();
    }
    else if (sdlkID == SDLK_RIGHTBRACKET) {
      // smaller grid
      if (grid > 1) grid /= 2;
      Redraw();
    }
    else if (sdlkID == SDLK_RETURN) EmitSignal(this, SDLK_RETURN);
    else (EmitSignal(this, (SDLKey)sdlkID));
  }

  void GuiTimeline::GetLocation(std::string &partName, int &currentFrame, bool &isKeyFrame) {
    partName = parts.at(selected).name;
    currentFrame = selectedFrame;
    isKeyFrame = false;
    for (int i = 0; i < (signed int)parts.at(selected).keyFrames.size(); i++) {
      if (parts.at(selected).keyFrames.at(i) == selectedFrame) isKeyFrame = true;
    }
  }

  void GuiTimeline::ClearKeys() {
    for (int i = 0; i < (signed int)parts.size(); i++) {
      parts.at(i).keyFrames.clear();
    }
  }

  bool GuiTimeline::ToggleKeyFrame() {
    bool isKeyFrame = false;

    std::vector<int>::iterator keyFrameIter = parts.at(selected).keyFrames.begin();
    while (keyFrameIter != parts.at(selected).keyFrames.end()) {
      if ((*keyFrameIter) == selectedFrame) {
        keyFrameIter = parts.at(selected).keyFrames.erase(keyFrameIter);
        isKeyFrame = true;
        break;
      } else {
        keyFrameIter++;
      }
    }

    if (!isKeyFrame) {
      // add key
      parts.at(selected).keyFrames.push_back(selectedFrame);
    }

    Redraw();

    return !isKeyFrame;
  }

  void GuiTimeline::EnableKeyFrame(const std::string &partName, int frame) {
    int part = 0;
    for (int i = 0; i < (signed int)parts.size(); i++) {
      if (parts.at(i).name == partName) {
        part = i;
        break;
      }
    }
    parts.at(part).keyFrames.push_back(frame);
  }

  void GuiTimeline::EnableKeyFrame(int part, int frame) {
    parts.at(part).keyFrames.push_back(frame);
  }

}
