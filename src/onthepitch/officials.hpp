// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OFFICIALS
#define _HPP_OFFICIALS

#include "player/humanoid/animcollection.hpp"

class PlayerBase;
class PlayerOfficial;
class PlayerData;

class Officials {

  public:
    Officials(Match *match, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection);
    virtual ~Officials();

    void GetPlayers(std::vector<PlayerBase*> &players);
    PlayerOfficial *GetReferee() { return referee; }
    PlayerOfficial *GetLinesmanNorth() { return linesmen[0]; }
    PlayerOfficial *GetLinesmanSouth() { return linesmen[1]; }

    virtual void Process();
    virtual void PreparePutBuffers(unsigned long snapshotTime_ms);
    virtual void FetchPutBuffers(unsigned long putTime_ms);
    virtual void Put();

    boost::intrusive_ptr<Geometry> GetYellowCardGeom() { return yellowCard; }
    boost::intrusive_ptr<Geometry> GetRedCardGeom() { return redCard; }

  protected:
    Match *match;

    PlayerOfficial *referee;
    PlayerOfficial *linesmen[2];
    PlayerData *playerData;

    boost::intrusive_ptr<Geometry> yellowCard;
    boost::intrusive_ptr<Geometry> redCard;

};

#endif
