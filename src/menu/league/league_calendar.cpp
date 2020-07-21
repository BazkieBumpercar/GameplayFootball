// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "league_calendar.hpp"

#include "../pagefactory.hpp"

LeagueCalendarPage::LeagueCalendarPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_calendar", 20, 20, 60, 3, "Calendar");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueCalendarPage::~LeagueCalendarPage() {
}
