// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "console.hpp"

#include "base/log.hpp"

#include "managers/resourcemanagerpool.hpp"

namespace blunted {

  Console::Console(boost::shared_ptr<Scene2D> scene2D, int numlines) : scene2D(scene2D), numlines(numlines) {
    text2D = new Text2D(scene2D);

    lineheight = 20;

    input = text2D->Create(100);
    text2D->SetText(input, "hier komt de inputline", Vector3(1, 1, 1));
    input->SetPosition(10, CalculateY(numlines));
    scene2D->AddObject(input);

    logConn = BindLog(boost::bind(&Console::PrintLog, this, _1, _2, _3, _4));
  }

  Console::~Console() {
    logConn.disconnect();
    input.reset();
    lines.clear();
    colors.clear();
    delete text2D;
  }

  void Console::PrintLog(e_LogType logType, std::string className, std::string methodName, std::string message) {
    std::string logTypeString;
    Vector3 color;
    switch (logType) {
      case e_Notice: logTypeString = "Notice"; color.Set(0.5, 0.5, 1.0); break;
      case e_Warning: logTypeString = "Warning"; color.Set(1.0, 1.0, 0.0); break;
      case e_Error: logTypeString = "ERROR"; color.Set(1.0, 0.3, 0.3); break;
      case e_FatalError: logTypeString = "FATAL ERROR !!! N00000!!!"; color.Set(1.0, 0.0, 0.0); break;
    }

    char bla[2048];
    sprintf(bla, "[%s] in [%s::%s]: %s", logTypeString.c_str(), className.c_str(), methodName.c_str(), message.c_str());

    Print(bla, color);
  }

  void Console::Print(const std::string &string, const Vector3 &color) {
    lines.push_back(string);
    colors.push_back(color);
    UpdateImages();
    ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->RemoveUnused();
  }

  void Console::UpdateImages() {

    // clear old
    for (int i = 0; i < (signed int)images.size(); i++) {
      scene2D->RemoveObject(images.at(i));
    }
    images.clear();

    // insert new
    int lineCount = lines.size();
    int numToDraw = lineCount;
    if (lineCount > numlines) numToDraw = numlines;
    int startOffset = lineCount - numToDraw;

    for (int i = 0; i < numToDraw; i++) {
      int offset = startOffset + i;

      boost::intrusive_ptr<Image2D> text = text2D->Create(100);
      text2D->SetText(text, lines.at(offset), colors.at(offset));
      text->SetPosition(10, CalculateY(i));
      scene2D->AddObject(text);

      images.push_back(text);
    }
  }

  int Console::CalculateY(int lineNum) {
    int context_width, context_height, context_bpp;
    scene2D->GetContextSize(context_width, context_height, context_bpp);
    int height = context_height - (lineheight * numlines) - 30;
    height += (lineheight * lineNum);
    return height;
  }

}
