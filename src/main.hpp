// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MAIN
#define _HPP_MAIN

#include "blunted.hpp"

#include "gametask.hpp"
#include "menu/menutask.hpp"
#include "hid/ihidevice.hpp"

#include "systems/graphics/graphics_system.hpp"

#include "base/properties.hpp"

#include "utils/database.hpp"
#include "misc/sqlite3.h"

enum e_DebugMode {
  e_DebugMode_Off,
  e_DebugMode_Tactical,
  e_DebugMode_AI
};

class Match;

void SetGreenDebugPilon(const Vector3 &pos);
void SetBlueDebugPilon(const Vector3 &pos);
void SetYellowDebugPilon(const Vector3 &pos);
void SetRedDebugPilon(const Vector3 &pos);

void SetSmallDebugCircle1(const Vector3 &pos);
void SetSmallDebugCircle2(const Vector3 &pos);
void SetLargeDebugCircle(const Vector3 &pos);

boost::intrusive_ptr<Geometry> GetGreenDebugPilon();
boost::intrusive_ptr<Geometry> GetBlueDebugPilon();
boost::intrusive_ptr<Geometry> GetYellowDebugPilon();
boost::intrusive_ptr<Geometry> GetRedDebugPilon();

boost::intrusive_ptr<Geometry> GetSmallDebugCircle1();
boost::intrusive_ptr<Geometry> GetSmallDebugCircle2();
boost::intrusive_ptr<Geometry> GetLargeDebugCircle();

std::string GetConfigFilename();
boost::shared_ptr<Scene2D> GetScene2D();
boost::shared_ptr<Scene3D> GetScene3D();
GraphicsSystem *GetGraphicsSystem();
boost::shared_ptr<GameTask> GetGameTask();
boost::shared_ptr<MenuTask> GetMenuTask();

bool IsReleaseVersion();
bool Verbose();
bool UpdateNonImportableDB();

Database *GetDB();
Properties *GetConfiguration();
std::string GetActiveSaveDirectory();
void SetActiveSaveDirectory(const std::string &dir);
bool SuperDebug();
e_DebugMode GetDebugMode();
boost::intrusive_ptr<Image2D> GetDebugImage();
boost::intrusive_ptr<Image2D> GetDebugOverlay();
void GetDebugOverlayCoord(Match *match, const Vector3 &worldPos, int &x, int &y);

int PredictFrameTimeToGo_ms(int frameCount);

const std::vector<IHIDevice*> &GetControllers();

int main(int argc, const char** argv);

#endif
