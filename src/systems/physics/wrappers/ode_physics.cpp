// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "ode_physics.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

namespace blunted {

  OdePhysics::OdePhysics() {
    dInitODE();
  };

  OdePhysics::~OdePhysics() {
    dCloseODE();
  };

  static void OdePhysicsNearCallback(void *data, dGeomID o1, dGeomID o2) {
    if (dGeomIsSpace(o1) || dGeomIsSpace(o2)) {

      // colliding a space with something
      dSpaceCollide2(o1, o2, data, &OdePhysicsNearCallback);

      // collide all geoms internal to the space(s)
      if (dGeomIsSpace(o1)) dSpaceCollide((dSpaceID)o1, data, &OdePhysicsNearCallback);
      if (dGeomIsSpace(o2)) dSpaceCollide((dSpaceID)o2, data, &OdePhysicsNearCallback);

    } else {

      dBodyID bodies[2];
      bodies[0] = dGeomGetBody(o1);
      bodies[1] = dGeomGetBody(o2);

      if (bodies[0] != NULL || bodies[1] != NULL) {

        // todo: find out what exactly these do
        if (bodies[0] && bodies[1] && dAreConnectedExcluding(bodies[0], bodies[1], dJointTypeContact)) return;
        if (bodies[0] && bodies[1] && dAreConnected(bodies[0], bodies[1])) return;

        Properties *properties[2];
        properties[0] = static_cast<Properties*>(dGeomGetData(o1));
        properties[1] = static_cast<Properties*>(dGeomGetData(o2));

        // the properties of the dynamic object
        // if both of the colliding objects are colliding, take the first (kinda hacky ;))
        // example problem is, if both are dynamic, what fdir1 to use? so just use the first
        int dynamicBody = 0;
        if (bodies[0]) dynamicBody = 0;
        else if (bodies[1]) dynamicBody = 1;

        // when larger, collisions near seams between trimesh triangles causes 2 contacts (with both triangles) to be created.
        // this causes bumpy movement on seams.
        // max 1 contact probably won't work with non-convex objects, so they are not supported (yet)
        int MAX_CONTACTS = 12;
        dContact contactsArray[MAX_CONTACTS];
        std::vector<dContact> contacts;

        // colliding two non-space geoms, so generate contact points between o1 and o2
        int num_contact = dCollide(o1, o2, MAX_CONTACTS, &contactsArray[0].geom, sizeof(dContact));

        // put into vector for simpler merging
        for (int i = 0; i < num_contact; i++) {
          dContact contact = contactsArray[i];
          contacts.push_back(contact);
        }


        // contact point merging strategy
        // more info: http://www.ode.org/old_list_archives/2007-June/022137.html

        float mergeThreshold = 0.4; // contact points less than 40 centimeters apart? merge em
        bool firstTime = true;
        bool possibleNeighbours = false;

        int startContacts = contacts.size();
        //if (startContacts > 1) printf("before: %i points\n", contacts.size());

        while (possibleNeighbours || firstTime) {
          firstTime = false;
          possibleNeighbours = false;

          std::vector<dContact>::iterator subjectIter = contacts.begin();
          while (subjectIter != contacts.end()) {

            // for each contact, find all neighbours

            Vector3 pos((*subjectIter).geom.pos[0], (*subjectIter).geom.pos[1], (*subjectIter).geom.pos[2]);
            Vector3 normal((*subjectIter).geom.normal[0], (*subjectIter).geom.normal[1], (*subjectIter).geom.normal[2]);

            std::vector<dContact>::iterator checkIter = subjectIter + 1;
            while (checkIter != contacts.end()) {

              Vector3 pos2((*checkIter).geom.pos[0], (*checkIter).geom.pos[1], (*checkIter).geom.pos[2]);
              Vector3 normal2((*checkIter).geom.normal[0], (*checkIter).geom.normal[1], (*checkIter).geom.normal[2]);

              if (pos.GetDistance(pos2) < mergeThreshold) {

                // merge
                Vector3 newPos = (pos + pos2) * 0.5;
                Vector3 newNormal = (normal + normal2) * 0.5;
                if (newNormal.GetLength() == 0) {
                  newNormal.Set(0, 0, 1);
                  Log(e_Warning, "ode_physics.cpp", "OdePhysicsNearCallback", "Unable to normalize Vector3");
                }
                newNormal.Normalize();

                // replace subject with new contact pos/normal
                // edit: just ignoring the second contact seems to give better results
                /*
                (*subjectIter).geom.pos[0] = newPos.coords[0];
                (*subjectIter).geom.pos[1] = newPos.coords[1];
                (*subjectIter).geom.pos[2] = newPos.coords[2];
                (*subjectIter).geom.normal[0] = newNormal.coords[0];
                (*subjectIter).geom.normal[1] = newNormal.coords[1];
                (*subjectIter).geom.normal[2] = newNormal.coords[2];
                (*subjectIter).geom.depth += (*checkIter).geom.depth;
                (*subjectIter).geom.depth *= 0.5;
                */

                //newNormal.Print();
                //newPos.Print();
                //printf("depth: %f\n", (*subjectIter).geom.depth);
                //printf("sub %f\n", (*subjectIter).geom.depth);
                //printf("chk %f\n", (*checkIter).geom.depth);

                // remove mergee
                checkIter = contacts.erase(checkIter);

                // iterate the 'main loop' once more after this, there might be more neighbours
                possibleNeighbours = true;

              } else {

                // leave alone
                checkIter++;

              }

            }  // check iter

            subjectIter++;

          } // subject iter

        } // possible neighbors

        //if (startContacts > 1) printf("after: %i points", contacts.size());
        //if (contacts.size() >= 2) printf(" <-- !!!");
        //if (startContacts > 1) printf("\n");


        // add these contact points to the simulation ...

        std::vector<dContact>::iterator subjectIter = contacts.begin();
        //printf("%f - %f\n", properties1->GetReal("friction"), properties2->GetReal("friction"));
        while (subjectIter != contacts.end()) {
         	(*subjectIter).surface.mode = dContactMu2 | dContactSoftCFM | dContactSoftERP | dContactApprox1;
      		(*subjectIter).surface.mu = properties[0]->GetReal("friction_1", 1) * properties[1]->GetReal("friction_1", 1);
      		(*subjectIter).surface.mu2 = properties[0]->GetReal("friction_2", 1) * properties[1]->GetReal("friction_2", 1);
      		(*subjectIter).surface.soft_cfm = 1e-5f;
  		    (*subjectIter).surface.soft_erp = 0.8;
  		    if (properties[dynamicBody]->Exists("bounce")) {
            (*subjectIter).surface.mode |= dContactBounce;
        		(*subjectIter).surface.bounce = properties[dynamicBody]->GetReal("bounce", 0.1);
        		(*subjectIter).surface.bounce_vel = properties[dynamicBody]->GetReal("bounce_minimum_velocity", 0.1);
          }

  		    if (properties[dynamicBody]->Exists("friction_direction_1")) {
            (*subjectIter).surface.mode |= dContactFDir1;

            /* disabled, calculate using the body's linear velocity
            // get user fdir1
            Vector3 fdir1 = properties[dynamicBody]->GetVector3("friction_direction_1");
            */

            // get object's rotation
            const dReal *R;
            R = dBodyGetLinearVel(bodies[dynamicBody]);
            Vector3 fdir1((float)R[0], (float)R[1], (float)R[2]);
            if (fdir1.GetLength() == 0) {
              Log(e_Warning, "ode_physics", "OdePhysicsCollisionCallback", "Unable to normalize fdir1");
              fdir1.Set(1, 0, 0);
            }
            fdir1.Normalize();

            // rotate fdir by object's rotation
            //fdir1 = rot * fdir1;

    		    (*subjectIter).fdir1[0] = fdir1.coords[0];
    		    (*subjectIter).fdir1[1] = fdir1.coords[1];
    		    (*subjectIter).fdir1[2] = fdir1.coords[2];

     		    if (properties[dynamicBody]->Exists("slip")) {
              (*subjectIter).surface.mode |= dContactSlip2;

              // get rolling velocity
              R = dBodyGetAngularVel(bodies[dynamicBody]);
              Vector3 vel((float)R[0], (float)R[1], (float)R[2]);
              float velocity = vel.GetLength();
              (*subjectIter).surface.slip2 = properties[dynamicBody]->GetReal("slip") * velocity;
            }

          }

          dJointID c = dJointCreateContact(static_cast<dWorldID>(data), collisionGroup, &*subjectIter);
          dJointAttach(c, bodies[0], bodies[1]);
          //printf("%f %f %f\n", contact[i].geom.normal[0], contact[i].geom.normal[1], contact[i].geom.normal[2]);

          subjectIter++;
        }
      }
    }
  }

  int OdePhysics::StepTime(int timediff_ms, int resolution_ms) {
    mutex.lock();

    // how many steps to undertake?

    // modulus version
    int numSteps = timediff_ms / resolution_ms;
    int remainder_ms = timediff_ms % resolution_ms;
    //printf("%i %i %i %i\n", numSteps, timediff_ms, resolution_ms, remainder_ms);

    //printf("%i timesteps * %i resolution_ms\n", numSteps, resolution_ms);

    // for each world..
    // check for collisions and add contact joints
    // do timestep

    // quick hax: link world and space id's, since ODE is kinda silly
    // todo: combine world and space into one map
    std::map <int, dWorldID>::iterator worldIter = worldMap.begin();
    std::map <int, dSpaceID>::iterator spaceIter = spaceMap.begin();
    while (worldIter != worldMap.end()) {

      for (int i = 0; i < numSteps; i++) {
        dSpaceCollide((*spaceIter).second, (*worldIter).second, &OdePhysicsNearCallback);
        dWorldQuickStep((*worldIter).second, resolution_ms * 0.001);
        dJointGroupEmpty(collisionGroup);
      }

      spaceIter++;
      worldIter++;
    }

    mutex.unlock();

    return remainder_ms;
  }

  int OdePhysics::CreateWorld() {
    mutex.lock();

    dWorldID worldID = dWorldCreate();

    //dWorldSetContactSurfaceLayer(worldID, 0.001);

    collisionGroup = dJointGroupCreate(0);

    int entry = worldMap.size();

    std::pair <std::map <int, dWorldID>::iterator, bool> result = worldMap.insert(std::map <int, dWorldID>::value_type(entry, worldID));
    if (result.second == false) {
      Log(e_FatalError, "OdePhysics", "CreateWorld", "Could not insert worldMap entry");
      return 0;
    }

    mutex.unlock();

    return entry;
  }

  void OdePhysics::DeleteWorld(int worldID) {
    mutex.lock();

    dJointGroupDestroy(collisionGroup);

    dWorldDestroy(GetOdeWorldID(worldID));
    worldMap.erase(worldMap.find(worldID));

    mutex.unlock();
  }

  int OdePhysics::CreateSpace() {
    mutex.lock();

    dSpaceID spaceID = dHashSpaceCreate(0);
    dHashSpaceSetLevels(spaceID, 10, 100);
    dSpaceSetCleanup(spaceID, 1); // delete geoms on space destroy

    int entry = spaceMap.size();

    std::pair <std::map <int, dSpaceID>::iterator, bool> result = spaceMap.insert(std::map <int, dSpaceID>::value_type(entry, spaceID));
    if (result.second == false) {
      Log(e_FatalError, "OdePhysics", "CreateSpace", "Could not insert spaceMap entry");
      return 0;
    }

    mutex.unlock();

    return entry;
  }

  void OdePhysics::DeleteSpace(int spaceID) {
    mutex.lock();
    dSpaceDestroy(GetOdeSpaceID(spaceID));
    spaceMap.erase(spaceMap.find(spaceID));
    mutex.unlock();
  }

  void OdePhysics::SetGravity(int worldID, const Vector3 &gravity) {
    mutex.lock();
    dWorldSetGravity(GetOdeWorldID(worldID), gravity.coords[0], gravity.coords[1], gravity.coords[2]);
    mutex.unlock();
  }

  void OdePhysics::SetErrorCorrection(int worldID, float value) {
    mutex.lock();
    dWorldSetERP(GetOdeWorldID(worldID), value);
    mutex.unlock();
  }

  void OdePhysics::SetConstraintForceMixing(int worldID, float value) {
    mutex.lock();
    dWorldSetCFM(GetOdeWorldID(worldID), value);
    mutex.unlock();
  }


  // actor functions

  IPhysicsActor *OdePhysics::CreateActor(int worldID) {
    mutex.lock();

    dBodyID id = dBodyCreate(GetOdeWorldID(worldID));

    OdePhysicsActor *actor = new OdePhysicsActor();
    actor->id = id;

    mutex.unlock();

    return actor;
  }

  void OdePhysics::DeleteActor(IPhysicsActor *actor) {
    mutex.lock();

    dBodyDestroy(static_cast<OdePhysicsActor*>(actor)->id);
    delete actor;
    actor = 0;

    mutex.unlock();
  }

  void OdePhysics::ActorSetMassSphere(IPhysicsActor *actor, float radius, float weight) {
    mutex.lock();

    dMass mass;
    dMassSetZero(&mass);

    dMassSetSphereTotal(&mass, weight, radius);
    dBodySetMass(static_cast<OdePhysicsActor*>(actor)->id, &mass);

    mutex.unlock();
  }

  void OdePhysics::ActorSetMassBox(IPhysicsActor *actor, const Vector3 &sides, float weight) {
    mutex.lock();

    dMass mass;
    dMassSetZero(&mass);

    dMassSetBoxTotal(&mass, weight, sides.coords[0], sides.coords[1], sides.coords[2]);
    dBodySetMass(static_cast<OdePhysicsActor*>(actor)->id, &mass);

    mutex.unlock();
  }

  void OdePhysics::ActorSetMassPosition(IPhysicsActor *actor, const Vector3 &pos) {
    mutex.lock();

    const dReal *bla = dBodyGetPosition(static_cast<OdePhysicsActor*>(actor)->id);

    dGeomID geom = dBodyGetFirstGeom(static_cast<OdePhysicsActor*>(actor)->id);

    dBodySetPosition(static_cast<OdePhysicsActor*>(actor)->id, bla[0] + pos.coords[0], bla[1] + pos.coords[1], bla[2] + pos.coords[2]);
    dGeomSetOffsetPosition(geom, -pos.coords[0], -pos.coords[1], -pos.coords[2]);

    mutex.unlock();
  }

  void OdePhysics::ActorSetPosition(IPhysicsActor *actor, const Vector3 &pos) {
    mutex.lock();
    dBodySetPosition(static_cast<OdePhysicsActor*>(actor)->id, pos.coords[0], pos.coords[1], pos.coords[2]);
    mutex.unlock();
  }

  void OdePhysics::ActorSetRotation(IPhysicsActor *actor, const Quaternion &rot) {
    mutex.lock();
    dReal R[4] = { rot.elements[3], rot.elements[0], rot.elements[1], rot.elements[2] };
    dBodySetQuaternion(static_cast<OdePhysicsActor*>(actor)->id, R);
    mutex.unlock();
  }

  Vector3 OdePhysics::ActorGetPosition(IPhysicsActor *actor) {
    mutex.lock();
    const dReal *P;
    P = dBodyGetPosition(static_cast<OdePhysicsActor*>(actor)->id);
    Vector3 pos = Vector3((float)P[0], (float)P[1], (float)P[2]);
    mutex.unlock();

    return pos;
  }

  Quaternion OdePhysics::ActorGetRotation(IPhysicsActor *actor) {
    mutex.lock();
    const dReal *R;
    R = dBodyGetQuaternion(static_cast<OdePhysicsActor*>(actor)->id);
    Quaternion rot((float)R[1], (float)R[2], (float)R[3], (float)R[0]);
    mutex.unlock();

    return rot;
  }

  float OdePhysics::ActorGetVelocity(IPhysicsActor *actor) {
    mutex.lock();
    const dReal *P;
    P = dBodyGetLinearVel(static_cast<OdePhysicsActor*>(actor)->id);
    Vector3 velocity = Vector3((float)P[0], (float)P[1], (float)P[2]);
    mutex.unlock();

    return velocity.GetLength();
  }

  Vector3 OdePhysics::ActorGetMovement(IPhysicsActor *actor) {
    mutex.lock();
    const dReal *P;
    P = dBodyGetLinearVel(static_cast<OdePhysicsActor*>(actor)->id);
    Vector3 velocity = Vector3((float)P[0], (float)P[1], (float)P[2]);
    mutex.unlock();

    return velocity;
  }

  void OdePhysics::ActorApplyForceAtRelativePosition(IPhysicsActor *actor, float force, const Vector3 &direction, const Vector3 &position) {
    mutex.lock();
    dBodyAddForceAtRelPos(static_cast<OdePhysicsActor*>(actor)->id,
                          direction.coords[0] * force, direction.coords[1] * force, direction.coords[2] * force,
                          position.coords[0], position.coords[1], position.coords[2]);
    mutex.unlock();
  }


  // collision functions

  IPhysicsCollisionMesh *OdePhysics::CreateCollisionPlane(int spaceID, const Vector3 &normal, float d) {
    mutex.lock();

    dGeomID id = dCreatePlane(GetOdeSpaceID(spaceID), normal.coords[0], normal.coords[1], normal.coords[2], d);

    OdePhysicsCollisionMesh *coll = new OdePhysicsCollisionMesh();
    coll->id = id;

    mutex.unlock();

    return coll;
  }

  IPhysicsCollisionMesh *OdePhysics::CreateCollisionSphere(int spaceID, float radius) {
    mutex.lock();

    dGeomID id = dCreateSphere(GetOdeSpaceID(spaceID), radius);

    OdePhysicsCollisionMesh *coll = new OdePhysicsCollisionMesh();
    coll->id = id;

    mutex.unlock();

    return coll;
  }

  IPhysicsCollisionMesh *OdePhysics::CreateCollisionBox(int spaceID, const Vector3 &sides) {
    mutex.lock();

    dGeomID id = dCreateBox(GetOdeSpaceID(spaceID), sides.coords[0], sides.coords[1], sides.coords[2]);

    OdePhysicsCollisionMesh *coll = new OdePhysicsCollisionMesh();
    coll->id = id;

    mutex.unlock();

    return coll;
  }

  IPhysicsCollisionMesh *OdePhysics::CreateCollisionTriMesh(int spaceID, const float *vertices, int vertexCount) {
    mutex.lock();

    dTriMeshDataID trimesh_id = dGeomTriMeshDataCreate();

    // unique vertices version
    dReal *dvertices = new dReal[vertexCount * 3];
    int *indices = new int[vertexCount];
    // put all meshes' vertices in ode vertex buffer
    for (int i = 0; i < vertexCount; i++) {
      for (int v = 0; v < 3; v++) {
        dvertices[i * 3 + v] = vertices[i * 3 + v];
      }
      indices[i] = i;
    }

    dGeomTriMeshDataBuildSingle(trimesh_id, dvertices, sizeof(dReal) * 3, vertexCount,
                                indices, vertexCount, sizeof(unsigned int) * 3);



    // shared vertices version
    /*
    // find duplicate vertices and index them
    std::vector<Vector3> uniqueVertices;
    //std::map <int, int> newVertexIndices;

    int *indices = new int[vertexCount];

    //printf("size before: %i\n", vertexCount);

    for (int i = 0; i < vertexCount; i++) {
      Vector3 pos(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
      std::vector<Vector3>::iterator found = std::find(uniqueVertices.begin(), uniqueVertices.end(), pos);
      if (found == uniqueVertices.end()) {
        uniqueVertices.push_back(pos);
        indices[i] = uniqueVertices.size() - 1;
      } else {
        // map vertex to new vertex id
        indices[i] = std::distance(uniqueVertices.begin(), found);
      }
    }

    dReal *dvertices = new dReal[uniqueVertices.size() * 3];

    for (int i = 0; i < uniqueVertices.size(); i++) {
      dvertices[i * 3 + 0] = uniqueVertices.at(i).coords[0];
      dvertices[i * 3 + 1] = uniqueVertices.at(i).coords[1];
      dvertices[i * 3 + 2] = uniqueVertices.at(i).coords[2];
    }

    //printf("size after: %i\n", uniqueVertices.size());

    dGeomTriMeshDataBuildSingle(trimesh_id, dvertices, sizeof(dReal) * 3, uniqueVertices.size(),
                                indices, vertexCount, sizeof(unsigned int) * 3);
    */

    dGeomID id = dCreateTriMesh(GetOdeSpaceID(spaceID), trimesh_id, NULL, NULL, NULL);

    OdePhysicsCollisionMesh *coll = new OdePhysicsCollisionMesh();
    coll->id = id;
    coll->vertices = dvertices;
    coll->indices = indices;

    mutex.unlock();

    return coll;
  }

  void OdePhysics::DeleteCollisionMesh(IPhysicsCollisionMesh *mesh) {
    mutex.lock();

    // unlink body first
    dGeomSetBody(static_cast<OdePhysicsCollisionMesh*>(mesh)->id, 0);
    dGeomDestroy(static_cast<OdePhysicsCollisionMesh*>(mesh)->id);
    delete mesh;
    mesh = 0;

    mutex.unlock();
  }

  void OdePhysics::CollisionMeshSetActor(IPhysicsCollisionMesh *mesh, IPhysicsActor *actor) {
    mutex.lock();
    dGeomSetBody(static_cast<OdePhysicsCollisionMesh*>(mesh)->id, static_cast<OdePhysicsActor*>(actor)->id);
    mutex.unlock();
  }

  void OdePhysics::CollisionMeshSetPosition(IPhysicsCollisionMesh *mesh, const Vector3 &pos) {
    mutex.lock();
    dGeomSetPosition(static_cast<OdePhysicsCollisionMesh*>(mesh)->id, pos.coords[0], pos.coords[1], pos.coords[2]);
    mutex.unlock();
  }

  void OdePhysics::CollisionMeshSetRotation(IPhysicsCollisionMesh *mesh, const Quaternion &rot) {
    mutex.lock();
    dReal R[4] = { rot.elements[3], rot.elements[0], rot.elements[1], rot.elements[2] };
    dGeomSetQuaternion(static_cast<OdePhysicsCollisionMesh*>(mesh)->id, R);
    mutex.unlock();
  }

  Vector3 OdePhysics::CollisionMeshGetPosition(IPhysicsCollisionMesh *mesh) {
    mutex.lock();
    const dReal *P;
    P = dGeomGetPosition(static_cast<OdePhysicsCollisionMesh*>(mesh)->id);
    Vector3 pos = Vector3((float)P[0], (float)P[1], (float)P[2]);
    mutex.unlock();

    return pos;
  }

  Quaternion OdePhysics::CollisionMeshGetRotation(IPhysicsCollisionMesh *mesh) {
    mutex.lock();
    dReal *R = 0;
    dGeomGetQuaternion(static_cast<OdePhysicsCollisionMesh*>(mesh)->id, R);
    Quaternion rot((float)R[1], (float)R[2], (float)R[3], (float)R[0]);
    mutex.unlock();

    return rot;
  }

  void OdePhysics::CollisionMeshSetData(IPhysicsCollisionMesh *mesh, const Properties *data) {
    mutex.lock();
    // need to const_cast, cause ODE needs a non-const.
    // just don't alter it in the collision callback function!
    dGeomSetData(static_cast<OdePhysicsCollisionMesh*>(mesh)->id, const_cast<Properties*>(data));
    mutex.unlock();
  }


  // joint functions

  IPhysicsJoint *OdePhysics::CreateJoint(int worldID, e_JointType jointType, IPhysicsActor *actor1, IPhysicsActor *actor2, const Vector3 &anchor, const Vector3 &axis1, const Vector3 &axis2) {
    mutex.lock();

    dWorldID odeWorldID = GetOdeWorldID(worldID);

    // hmm create a nice joint! smokin'
    OdePhysicsJoint *joint = new OdePhysicsJoint();

    switch (jointType) {

      case e_JointType_Hinge:
        joint->id = dJointCreateHinge(odeWorldID, 0);
        dJointAttach(joint->id, static_cast<OdePhysicsActor*>(actor1)->id, static_cast<OdePhysicsActor*>(actor2)->id);
        dJointSetHingeAnchor(joint->id, anchor.coords[0], anchor.coords[1], anchor.coords[2]);
        dJointSetHingeAxis(joint->id, axis1.coords[0], axis1.coords[1], axis1.coords[2]);
        break;

      case e_JointType_Hinge2:
        joint->id = dJointCreateHinge2(odeWorldID, 0);
        dJointAttach(joint->id, static_cast<OdePhysicsActor*>(actor1)->id, static_cast<OdePhysicsActor*>(actor2)->id);
        dJointSetHinge2Anchor(joint->id, anchor.coords[0], anchor.coords[1], anchor.coords[2]);
        dJointSetHinge2Axis1(joint->id, axis1.coords[0], axis1.coords[1], axis1.coords[2]);
        dJointSetHinge2Axis2(joint->id, axis2.coords[0], axis2.coords[1], axis2.coords[2]);
        break;

      default:
        Log(e_FatalError, "OdePhysics", "CreateJoint", "Joint type " + int_to_str(jointType) + " not yet supported ;)");
        break;
    }

    joint->jointType = jointType;

    mutex.unlock();

    return joint;
  }

  void OdePhysics::DeleteJoint(IPhysicsJoint *joint) {
    mutex.lock();
    dJointDestroy(static_cast<OdePhysicsJoint*>(joint)->id);
    mutex.unlock();
  }

  void OdePhysics::JointSetStops(IPhysicsJoint *joint, radian lowStop, radian highStop, int paramNum) {
    mutex.lock();
    // hack for ode 'flaw': http://opende.sourceforge.net/wiki/index.php/FAQ#My_wheels_are_sloppy.3B_they_don.27t_stay_straight_when_I_turn_them.
    JointSetParameter(joint, dParamHiStop, highStop, paramNum);
    JointSetParameter(joint, dParamLoStop, lowStop, paramNum);
    JointSetParameter(joint, dParamHiStop, highStop, paramNum);
    mutex.unlock();
  }

  void OdePhysics::JointSetVelocity(IPhysicsJoint *joint, float velocity, int paramNum) { // desired velocity
    mutex.lock();
    JointSetParameter(joint, dParamVel, velocity, paramNum);
    mutex.unlock();
  }

  void OdePhysics::JointSetMaxForce(IPhysicsJoint *joint, float force, int paramNum) { // maximum force to reach the desired velocity
    mutex.lock();
    JointSetParameter(joint, dParamFMax, force, paramNum);
    mutex.unlock();
  }

  void OdePhysics::JointSetConstraintForceMixing(IPhysicsJoint *joint, float value, int paramNum) {
    mutex.lock();
    JointSetParameter(joint, dParamCFM, value, paramNum);
    mutex.unlock();
  }

  void OdePhysics::JointSetErrorCorrection(IPhysicsJoint *joint, float value, int paramNum) {
    mutex.lock();
    JointSetParameter(joint, dParamERP, value, paramNum);
    mutex.unlock();
  }

  void OdePhysics::JointSetSuspensionConstraintForceMixing(IPhysicsJoint *joint, float value) { // only on hinge2 joints
    mutex.lock();
    JointSetParameter(joint, dParamSuspensionCFM, value);
    mutex.unlock();
  }

  void OdePhysics::JointSetSuspensionErrorReduction(IPhysicsJoint *joint, float value) { // only on hinge2 joints
    mutex.lock();
    dJointID id = static_cast<OdePhysicsJoint*>(joint)->id;
    JointSetParameter(joint, dParamSuspensionERP, value);
    mutex.unlock();
  }

  float OdePhysics::JointGetAngleRate(IPhysicsJoint *joint, int paramNum) {
    float value = 0;

    mutex.lock();

    dJointID id = static_cast<OdePhysicsJoint*>(joint)->id;
    switch (static_cast<OdePhysicsJoint*>(joint)->jointType) {

      case e_JointType_Hinge:
        value = dJointGetHingeAngleRate(id);
        break;

      case e_JointType_Hinge2:
        if (paramNum == 1) value = dJointGetHinge2Angle1Rate(id);
        if (paramNum == 2) value = dJointGetHinge2Angle2Rate(id);
        break;

      default:
        break;

    }

    mutex.unlock();

    return value;
  }


  // utility

  void OdePhysics::JointSetParameter(IPhysicsJoint *joint, int parameter, float value, int paramNum) {
    OdePhysicsJoint *odeJoint = static_cast<OdePhysicsJoint*>(joint);

    /*
    e_JointType_Ball,
    e_JointType_Hinge,
    e_JointType_Slider,
    e_JointType_Contact,
    e_JointType_Universal,
    e_JointType_Hinge2,
    e_JointType_PR,
    e_JointType_PU,
    e_JointType_Piston,
    e_JointType_Fixed,
    e_JointType_AMotor,
    e_JointType_LMotor,
    e_JointType_Plane2D
    */

    switch (odeJoint->jointType) {

      case e_JointType_Hinge:
        dJointSetHingeParam(odeJoint->id, parameter, value);
        break;

      case e_JointType_Hinge2:
        dJointSetHinge2Param(odeJoint->id, parameter + dParamGroup * (paramNum - 1), value);
        break;

      default:
        break;

    }
  }


  dSpaceID OdePhysics::GetOdeSpaceID(int spaceID) {
    if (spaceMap.find(spaceID) == spaceMap.end()) Log(e_FatalError, "OdePhysics", "GetOdeSpaceID", "spaceID non-existant");
    return spaceMap.find(spaceID)->second;
  }

  dWorldID OdePhysics::GetOdeWorldID(int worldID) {
    if (worldMap.find(worldID) == worldMap.end()) Log(e_FatalError, "OdePhysics", "GetOdeWorldID", "worldID non-existant");
    return worldMap.find(worldID)->second;
  }

}
