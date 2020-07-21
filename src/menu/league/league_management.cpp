// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "league_management.hpp"

#include "../pagefactory.hpp"

LeagueManagementPage::LeagueManagementPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_management", 20, 20, 60, 3, "Management");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueManagementPage::~LeagueManagementPage() {
}



LeagueManagementContractsPage::LeagueManagementContractsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_management_contracts", 20, 20, 60, 3, "Contracts");
  this->AddView(title);

  this->SetFocus();

  this->Show();
}

LeagueManagementContractsPage::~LeagueManagementContractsPage() {
}



LeagueManagementTransfersPage::LeagueManagementTransfersPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_management_transfers", 20, 20, 60, 3, "Transfers");
  this->AddView(title);

  this->SetFocus();

  this->Show();
}

LeagueManagementTransfersPage::~LeagueManagementTransfersPage() {
}
