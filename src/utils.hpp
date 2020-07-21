// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_FOOTBALL_UTILS
#define _HPP_FOOTBALL_UTILS

#include "gamedefines.hpp"

#include "scene/objects/camera.hpp"

#include <boost/circular_buffer.hpp>

using namespace blunted;

float GetQuantizedDirectionBias();
void QuantizeDirection(Vector3 &inputDirection, float bias = 1.0f);
Vector3 GetProjectedCoord(const Vector3 &pos3D, boost::intrusive_ptr<Camera> camera);

int GetVelocityID(e_Velocity velo, bool treatDribbleAsWalk = false);


// stats fiddling

enum e_PositionName {
  e_PositionName_GK,
  e_PositionName_SW,
  e_PositionName_D,
  e_PositionName_WB,
  e_PositionName_DM,
  e_PositionName_M,
  e_PositionName_AM,
  e_PositionName_F,
  e_PositionName_ST
};

struct WeightedPosition {
  e_PositionName positionName;
  float weight;
};

struct Stat {
  std::string name;
  float value;
};

enum e_DevelopmentCurveType {
  e_DevelopmentCurveType_Early,
  e_DevelopmentCurveType_Normal,
  e_DevelopmentCurveType_Late
};

// converts FM positions string into weighted positions vector
void GetWeightedPositions(const std::string &positionString, std::vector<WeightedPosition> &weightedPositions);

void InitDefaultProfiles();
void GetDefaultProfile(const std::vector<WeightedPosition> &weightedPositions, std::vector<Stat> &averageProfile);
std::string GetProfileString(const std::vector<Stat> &profileStats);

float GetAverageStatFromValue(int age, int value);

float CalculateStat(float baseStat, float profileStat, float age, e_DevelopmentCurveType developmentCurveType);
/* ^ above one supersedes these
float GetIndividualStat(float averageStat, float profileStat, float age);
float GetAverageStatFromBaseStat(int age, float baseStat, e_DevelopmentCurveType developmentCurveType);
*/

// /stats fiddling

template <typename T> struct TemporalValue {
  TemporalValue() {
    data = 0;
    time_ms = 0;
  }
  T data;
  unsigned long time_ms;
};

template <> TemporalValue<Quaternion>::TemporalValue();

template <typename T> class TemporalSmoother {

  public:
    TemporalSmoother();
    virtual ~TemporalSmoother() {}

    void SetValue(const T &data, unsigned long valueTime_ms);
    T GetValue(unsigned long currentTime_ms, unsigned long history_ms = temporalSmoother_history_ms) const; // get interpolated measurement, history_ms seconds ago from now

    void Clear() {
      values.clear();
    }

  protected:
    T MixData(const T &data1, const T &data2, float bias = 0.0f) const;

    boost::circular_buffer< TemporalValue<T> > values;
    unsigned int snapshotSize;

};

template <> Quaternion TemporalSmoother<Quaternion>::MixData(const Quaternion &data1, const Quaternion &data2, float bias) const;

#endif
