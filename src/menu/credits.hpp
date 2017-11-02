// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MENU_CREDITS
#define _HPP_MENU_CREDITS

#include "utils/gui2/windowmanager.hpp"

#include "utils/gui2/widgets/menu.hpp"
#include "utils/gui2/widgets/root.hpp"
#include "utils/gui2/widgets/image.hpp"
#include "utils/gui2/widgets/caption.hpp"

#include "base/math/vector3.hpp"

using namespace blunted;

const int numtexts = 24;
const int numballs = 12;

struct CreditsContents {
  std::string text;
  Vector3 color;
};

class CreditsPage : public Gui2Page {

  public:
    CreditsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~CreditsPage();

    void InitCreditsContents();
    void AddHeader(const std::string &blah);
    void AddSubHeader(const std::string &blah);
    void AddCredit(const std::string &blah);
    void AddWhitespace();

    virtual void Process();
    virtual void ProcessJoystickEvent(JoystickEvent *event);

  protected:
    Gui2Caption *text[numtexts];
    Gui2Image *balls[numballs];
    Vector3 ballPos[numballs];
    Vector3 ballMov[numballs];

    float scrollOffset;
    unsigned int creditOffset;
    unsigned int previousStartIndex;

    std::vector<CreditsContents> credits;

    Gui2Image *bg;

};

#endif
