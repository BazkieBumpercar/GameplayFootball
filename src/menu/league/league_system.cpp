// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "league_system.hpp"

#include "../pagefactory.hpp"

LeagueSystemPage::LeagueSystemPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_system", 20, 20, 60, 3, "System");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueSystemPage::~LeagueSystemPage() {
}



LeagueSystemSavePage::LeagueSystemSavePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_system_save", 20, 20, 60, 3, "Save");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueSystemSavePage::~LeagueSystemSavePage() {
}



LeagueSystemSettingsPage::LeagueSystemSettingsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_system_settings", 20, 20, 60, 3, "Settings");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueSystemSettingsPage::~LeagueSystemSettingsPage() {
}
