// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "league_standings.hpp"

#include "../pagefactory.hpp"

LeagueStandingsPage::LeagueStandingsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings", 20, 20, 60, 3, "Standings");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsPage::~LeagueStandingsPage() {
}



LeagueStandingsLeaguePage::LeagueStandingsLeaguePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_league", 20, 20, 60, 3, "League");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsLeaguePage::~LeagueStandingsLeaguePage() {
}



LeagueStandingsLeagueTablePage::LeagueStandingsLeagueTablePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_league_table", 20, 20, 60, 3, "Table");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsLeagueTablePage::~LeagueStandingsLeagueTablePage() {
}



LeagueStandingsLeagueStatsPage::LeagueStandingsLeagueStatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_league_stats", 20, 20, 60, 3, "Stats");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsLeagueStatsPage::~LeagueStandingsLeagueStatsPage() {
}



LeagueStandingsNCupPage::LeagueStandingsNCupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_ncup", 20, 20, 60, 3, "National Cup");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsNCupPage::~LeagueStandingsNCupPage() {
}



LeagueStandingsNCupTreePage::LeagueStandingsNCupTreePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_ncup_tree", 20, 20, 60, 3, "Tournament tree");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsNCupTreePage::~LeagueStandingsNCupTreePage() {
}



LeagueStandingsNCupStatsPage::LeagueStandingsNCupStatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_ncup_stats", 20, 20, 60, 3, "Stats");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsNCupStatsPage::~LeagueStandingsNCupStatsPage() {
}



LeagueStandingsICup1Page::LeagueStandingsICup1Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup1", 20, 20, 60, 3, "International Cup 1");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup1Page::~LeagueStandingsICup1Page() {
}



LeagueStandingsICup1GroupTablePage::LeagueStandingsICup1GroupTablePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup1_grouptable", 20, 20, 60, 3, "Group table");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup1GroupTablePage::~LeagueStandingsICup1GroupTablePage() {
}



LeagueStandingsICup1TreePage::LeagueStandingsICup1TreePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup1_tree", 20, 20, 60, 3, "Tournament tree");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup1TreePage::~LeagueStandingsICup1TreePage() {
}



LeagueStandingsICup1StatsPage::LeagueStandingsICup1StatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup1_stats", 20, 20, 60, 3, "Stats");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup1StatsPage::~LeagueStandingsICup1StatsPage() {
}



LeagueStandingsICup2Page::LeagueStandingsICup2Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup2", 20, 20, 60, 3, "International Cup 2");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup2Page::~LeagueStandingsICup2Page() {
}



LeagueStandingsICup2GroupTablePage::LeagueStandingsICup2GroupTablePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup2_grouptable", 20, 20, 60, 3, "Group table");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup2GroupTablePage::~LeagueStandingsICup2GroupTablePage() {
}



LeagueStandingsICup2TreePage::LeagueStandingsICup2TreePage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup2_tree", 20, 20, 60, 3, "Tournament tree");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup2TreePage::~LeagueStandingsICup2TreePage() {
}



LeagueStandingsICup2StatsPage::LeagueStandingsICup2StatsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_league_standings_icup2_stats", 20, 20, 60, 3, "Stats");
  this->AddView(title);
  title->Show();

  this->SetFocus();

  this->Show();
}

LeagueStandingsICup2StatsPage::~LeagueStandingsICup2StatsPage() {
}
