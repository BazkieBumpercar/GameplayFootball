// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_AISUPPORT_MENTALIMAGE
#define _HPP_AISUPPORT_MENTALIMAGE

#include "defines.hpp"

#include "base/math/vector3.hpp"

#include "../../gamedefines.hpp"

using namespace blunted;

class Match;

class MentalImage {

  public:
    MentalImage(Match *match);
    virtual ~MentalImage();

    void TakeSnapshot();

    PlayerImage GetPlayerImage(int playerID) const;
    void GetTeamPlayerImages(int teamID, int exceptPlayerID, std::vector<PlayerImage> &playerImages) const;

    void UpdateBallPredictions();
    Vector3 GetBallPrediction(unsigned int time_ms) const;

    void SetTimeStampNeg_ms(unsigned int history_ms) { timeStampNeg_ms = history_ms; }
    int GetTimeStampNeg_ms() const { return timeStampNeg_ms; }

  protected:
    Match *match;

    std::vector<PlayerImage> players;
    Vector3 ballPredictions[ballPredictionSize_ms / 10];

    unsigned int timeStampNeg_ms;

    float maxDistanceDeviation;
    float maxMovementDeviation;

};

#endif
