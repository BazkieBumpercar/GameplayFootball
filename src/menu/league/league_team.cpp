// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "league_team.hpp"

#include "../pagefactory.hpp"

LeagueTeamPage::LeagueTeamPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team", 20, 20, 60, 3, "Team");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPage::~LeagueTeamPage() {
}



LeagueTeamFormationPage::LeagueTeamFormationPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_formation", 20, 20, 60, 3, "Formation");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamFormationPage::~LeagueTeamFormationPage() {
}



LeagueTeamPlayerSelectionPage::LeagueTeamPlayerSelectionPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_playerselection", 20, 20, 60, 3, "Player selection");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPlayerSelectionPage::~LeagueTeamPlayerSelectionPage() {
}



LeagueTeamTacticsPage::LeagueTeamTacticsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_tactics", 20, 20, 60, 3, "Tactics");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamTacticsPage::~LeagueTeamTacticsPage() {
}



LeagueTeamPlayerOverviewPage::LeagueTeamPlayerOverviewPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_playeroverview", 20, 20, 60, 3, "Player overview");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPlayerOverviewPage::~LeagueTeamPlayerOverviewPage() {
}



LeagueTeamPlayerDevelopmentPage::LeagueTeamPlayerDevelopmentPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_playerdevelopment", 20, 20, 60, 3, "Player development");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamPlayerDevelopmentPage::~LeagueTeamPlayerDevelopmentPage() {
}



LeagueTeamSetupPage::LeagueTeamSetupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_team_setup", 20, 20, 60, 3, "Team setup");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueTeamSetupPage::~LeagueTeamSetupPage() {
}
