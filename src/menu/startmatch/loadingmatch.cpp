// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "loadingmatch.hpp"

#include "main.hpp"

#include "../pagefactory.hpp"

using namespace blunted;

LoadingMatchPage::LoadingMatchPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Image *loading = new Gui2Image(windowManager, "image_main_loading", 0, 0, 100, 100);
  loading->LoadImage("media/menu/main/loading01.png");
  this->AddView(loading);
  loading->Show();

  // logos
  MatchData *matchData = new MatchData(GetMenuTask()->GetTeamID(0), GetMenuTask()->GetTeamID(1));
  GetMenuTask()->SetMatchData(matchData);
  TeamData *teamData1 = matchData->GetTeamData(0);
  TeamData *teamData2 = matchData->GetTeamData(1);

  Gui2Caption *caption1 = new Gui2Caption(windowManager, "main_loading_team1caption", 20, 35, 40, 5, teamData1->GetName());
  float w = caption1->GetTextWidthPercent();
  caption1->SetPosition(30 - w * 0.5, 35);
  this->AddView(caption1);
  Gui2Image *logo1 = new Gui2Image(windowManager, "main_loading_team1logo", 25, 48, 10, 12.5);
  this->AddView(logo1);
  logo1->LoadImage(teamData1->GetLogoUrl());

  Gui2Caption *caption2 = new Gui2Caption(windowManager, "main_loading_team2caption", 60, 35, 40, 5, teamData2->GetName());
  w = caption2->GetTextWidthPercent();
  caption2->SetPosition(70 - w * 0.5, 35);
  this->AddView(caption2);
  Gui2Image *logo2 = new Gui2Image(windowManager, "main_loading_team2logo", 65, 48, 10, 12.5);
  this->AddView(logo2);
  logo2->LoadImage(teamData2->GetLogoUrl());

  caption1->Show();
  caption2->Show();
  logo1->Show();
  logo2->Show();

  this->SetFocus();

  this->Show();

  GetMenuTask()->SetActiveJoystickID(0);
  GetMenuTask()->EnableKeyboard();
  windowManager->GetPagePath()->Clear();

  if (Verbose()) printf("hoi\n");
  sentStartGameSignal = false;
}

LoadingMatchPage::~LoadingMatchPage() {
}

void LoadingMatchPage::Process() {
  Gui2Page::Process();

  if (!sentStartGameSignal) {
    sentStartGameSignal = true;
    EnvironmentManager::GetInstance().Pause_ms(100); // make 'sure' the logos have been displayed by the graphics thread. todonow: this is a terrible hax, and you know it. you are a bad person!
    GetMenuTask()->SetMenuAction(e_MenuAction_Game);
  }
}

void LoadingMatchPage::Close() {

  this->Exit();

  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_Game, properties, 0);

  if (Verbose()) printf("auw\n");
  delete this;

}
