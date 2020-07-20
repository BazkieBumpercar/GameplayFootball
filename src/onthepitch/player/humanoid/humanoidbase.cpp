// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include <cmath>
#include "humanoid.hpp"

#include "humanoid_utils.hpp"

#include "../playerbase.hpp"
#include "../../match.hpp"

#include "../../../main.hpp"

#include "../../AIsupport/AIfunctions.hpp"

#include "utils/animationextensions/footballanimationextension.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "scene/objectfactory.hpp"

const float bodyRotationSmoothingFactor = 1.0f;
const float bodyRotationSmoothingMaxAngle = 0.25f * pi;
const float initialReQueueDelayFrames = 32;

void FillTemporalHumanoidNodes(boost::intrusive_ptr<Node> targetNode, std::vector<TemporalHumanoidNode> &temporalHumanoidNodes) {
  //printf("%s\n", targetNode->GetName().c_str());
  TemporalHumanoidNode temporalHumanoidNode;
  temporalHumanoidNode.actualNode = targetNode;
  temporalHumanoidNode.cachedPosition = targetNode->GetPosition();
  temporalHumanoidNode.cachedOrientation = targetNode->GetRotation();
  // initial values, not sure if really needed
  temporalHumanoidNode.position.SetValue(targetNode->GetPosition(), EnvironmentManager::GetInstance().GetTime_ms());
  temporalHumanoidNode.orientation.SetValue(targetNode->GetRotation(), EnvironmentManager::GetInstance().GetTime_ms());
  temporalHumanoidNodes.push_back(temporalHumanoidNode);

  std::vector < boost::intrusive_ptr<Node> > gatherNodes;
  targetNode->GetNodes(gatherNodes);
  for (unsigned int i = 0; i < gatherNodes.size(); i++) {
    FillTemporalHumanoidNodes(gatherNodes.at(i), temporalHumanoidNodes);
  }

}

HumanoidBase::HumanoidBase(PlayerBase *player, Match *match, boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::shared_ptr<AnimCollection> animCollection, boost::intrusive_ptr<Node> fullbodyTargetNode, boost::intrusive_ptr < Resource<Surface> > kit, int bodyUpdatePhaseOffset) : fullbodyTargetNode(fullbodyTargetNode), match(match), player(player), anims(animCollection), buf_bodyUpdatePhaseOffset(bodyUpdatePhaseOffset) {

  interruptAnim = e_InterruptAnim_None;
  reQueueDelayFrames = 0;

  buf_LowDetailMode = false;
  fetchedbuf_previousSnapshotTime_ms = 0;

  currentAnim = new Anim();
  previousAnim = new Anim();

  decayingPositionOffset = Vector3(0);
  decayingDifficultyFactor = 0.0f;

  _cache_AgilityFactor = GetConfiguration()->GetReal("gameplay_agilityfactor", _default_AgilityFactor);
  _cache_AccelerationFactor = GetConfiguration()->GetReal("gameplay_accelerationfactor", _default_AccelerationFactor);

  allowedBodyDirVecs.push_back(Vector3(0, -1, 0));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.25 * pi));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.25 * pi));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.75 * pi));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.75 * pi));

  allowedBodyDirAngles.push_back(0 * pi);
  allowedBodyDirAngles.push_back(0.25 * pi);
  allowedBodyDirAngles.push_back(-0.25 * pi);
  allowedBodyDirAngles.push_back(0.75 * pi);
  allowedBodyDirAngles.push_back(-0.75 * pi);

  preferredDirectionVecs.push_back(Vector3(0, -1, 0));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.111 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.111 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.25 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.25 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.5 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.5 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.75 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.75 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.999 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.999 * pi));

  preferredDirectionAngles.push_back(0 * pi);
  preferredDirectionAngles.push_back(0.111 * pi); // 20
  preferredDirectionAngles.push_back(-0.111 * pi);
  preferredDirectionAngles.push_back(0.25 * pi); // 45
  preferredDirectionAngles.push_back(-0.25 * pi);
  preferredDirectionAngles.push_back(0.5 * pi); // 90
  preferredDirectionAngles.push_back(-0.5 * pi);
  preferredDirectionAngles.push_back(0.75 * pi); // 135
  preferredDirectionAngles.push_back(-0.75 * pi);
  preferredDirectionAngles.push_back(0.999 * pi); // 180
  preferredDirectionAngles.push_back(-0.999 * pi);

  assert(match);

  float playerHeight = player->GetPlayerData()->GetHeight();
  zMultiplier = (1.0f / defaultPlayerHeight) * playerHeight;

  boost::intrusive_ptr<Node> bla(new Node(*humanoidSourceNode.get(), "", GetScene3D()));
  humanoidNode = bla;
  humanoidNode->SetLocalMode(e_LocalMode_Absolute);

  boost::intrusive_ptr < Resource<Surface> > skin;
  skin = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("media/objects/players/textures/skin0" + int_to_str(player->GetPlayerData()->GetSkinColor()) + ".png", true, true);

  boost::intrusive_ptr<Node> bla2(new Node(*fullbodySourceNode.get(), int_to_str(player->GetID()), GetScene3D()));
  fullbodyNode = bla2;
  fullbodyNode->SetLocalMode(e_LocalMode_Absolute);
  fullbodyTargetNode->AddNode(fullbodyNode);

  boost::intrusive_ptr< Resource<GeometryData> > bodyGeom = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  bodyGeom->resourceMutex.lock();
  std::vector < MaterializedTriangleMesh > &tmesh = bodyGeom->GetResource()->GetTriangleMeshesRef();
  for (unsigned int i = 0; i < tmesh.size(); i++) {
    if (tmesh.at(i).material.diffuseTexture != boost::intrusive_ptr< Resource<Surface> >()) {
      if (tmesh.at(i).material.diffuseTexture->GetIdentString() == "skin.jpg") {
        tmesh.at(i).material.diffuseTexture = skin;
        tmesh.at(i).material.specular_amount = 0.002f;
        tmesh.at(i).material.shininess = 0.2f;
      }
    }
  }
  bodyGeom->resourceMutex.unlock();

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();

  kitDiffuseTextureIdentString = "kit_template.png";
  SetKit(kit);


  scene3D = GetScene3D();

  FillNodeMap(humanoidNode, nodeMap);
  FillTemporalHumanoidNodes(humanoidNode, buf_TemporalHumanoidNodes);

  PrepareFullbodyModel(colorCoords);
  buf_bodyUpdatePhase = 0;


  // hairstyle

  boost::intrusive_ptr < Resource<GeometryData> > geometry = ResourceManagerPool::GetInstance().GetManager<GeometryData>(e_ResourceType_GeometryData)->Fetch("media/objects/players/hairstyles/" + player->GetPlayerData()->GetHairStyle() + ".ase", true, true);
  hairStyle = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("hairstyle", e_ObjectType_Geometry));

  scene3D->CreateSystemObjects(hairStyle);
  hairStyle->SetLocalMode(e_LocalMode_Absolute);
  hairStyle->SetGeometryData(geometry);
  fullbodyTargetNode->AddObject(hairStyle);

  boost::intrusive_ptr < Resource<Surface> > hairTexture;
  hairTexture = ResourceManagerPool::GetInstance().GetManager<Surface>(e_ResourceType_Surface)->Fetch("media/objects/players/textures/hair/" + player->GetPlayerData()->GetHairColor() + ".png", true, true);

  std::vector < MaterializedTriangleMesh > &hairtmesh = hairStyle->GetGeometryData()->GetResource()->GetTriangleMeshesRef();

  for (unsigned int i = 0; i < hairtmesh.size(); i++) {
    if (hairtmesh.at(i).material.diffuseTexture != boost::intrusive_ptr<Resource <Surface> >()) {
      hairtmesh.at(i).material.diffuseTexture = hairTexture;
      hairtmesh.at(i).material.specular_amount = 0.01f;
      hairtmesh.at(i).material.shininess = 0.05f;
    }
  }
  hairStyle->OnUpdateGeometryData();


  ResetPosition(Vector3(0), Vector3(0));

  currentMentalImage = 0;
}

HumanoidBase::~HumanoidBase() {
  if (Verbose()) printf("exiting humanoidbase.. ");
  // printf("\n\nfullbodytargetnode:\n");
  // fullbodyTargetNode->PrintTree();
  // printf("\nhumanoidnode:\n");
  // humanoidNode->PrintTree();
  // printf("\n\n");
  humanoidNode->Exit();
  humanoidNode.reset();
  fullbodyTargetNode->DeleteNode(fullbodyNode);
  fullbodyTargetNode->DeleteObject(hairStyle);

  buf_TemporalHumanoidNodes.clear();

  for (unsigned int i = 0; i < uniqueFullbodyMesh.size(); i++) {
    delete [] uniqueFullbodyMesh.at(i).data;
  }

  for (unsigned int i = 0; i < uniqueIndicesVec.size(); i++) {
    delete [] uniqueIndicesVec.at(i);
  }

  // the other full body mesh ref is connected to a geometry object which has taken over ownership and will clean it up

  delete currentAnim;
  delete previousAnim;
  if (Verbose()) printf("done\n");
}

void HumanoidBase::PrepareFullbodyModel(std::map<Vector3, Vector3> &colorCoords) {

  if (Verbose()) printf("prepare full body model.. ");

  // base anim with default angles - all anims' joints will be inversely rotated by the joints in this anim. this way, the fullbody mesh doesn't need to have 0 degree angles
  Animation *baseAnim = new Animation();
  baseAnim->Load("media/animations/base.anim.util");
  AnimApplyBuffer animApplyBuffer;
  animApplyBuffer.anim = baseAnim;
  animApplyBuffer.frameNum = 0;
  animApplyBuffer.smooth = false;
  animApplyBuffer.smoothFactor = 0.0f;
  animApplyBuffer.position = Vector3(0);
  animApplyBuffer.orientation = 0;
  animApplyBuffer.offsets.clear();
  animApplyBuffer.anim->Apply(nodeMap, animApplyBuffer.frameNum, 0, animApplyBuffer.smooth, animApplyBuffer.smoothFactor, animApplyBuffer.position, animApplyBuffer.orientation, animApplyBuffer.offsets, 0, false, true);
  std::vector < boost::intrusive_ptr<Node> > jointsVec;
  humanoidNode->GetNodes(jointsVec, true);


  // joints

  for (unsigned int i = 0; i < jointsVec.size(); i++) {
    Joint joint;
    joint.node = jointsVec[i];
    joint.origPos = jointsVec[i]->GetDerivedPosition();
    joints.push_back(joint);
  }

  boost::intrusive_ptr < Resource<GeometryData> > fullbodyGeometryData = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  fullbodyGeometryData->resourceMutex.lock();
  std::vector < MaterializedTriangleMesh > &materializedTriangleMeshes = fullbodyGeometryData->GetResource()->GetTriangleMeshesRef();

  fullbodySubgeomCount = materializedTriangleMeshes.size();

  for (unsigned int subgeom = 0; subgeom < fullbodySubgeomCount; subgeom++) {

    std::vector<WeightedVertex> weightedVertexVector;
    weightedVerticesVec.push_back(weightedVertexVector);

    FloatArray meshRef;
    meshRef.data = materializedTriangleMeshes.at(subgeom).vertices;
    meshRef.size = materializedTriangleMeshes.at(subgeom).verticesDataSize;

    FloatArray uniqueMesh;

    int elementOffset = meshRef.size / GetTriangleMeshElementCount(); // was: fullbodyMeshSize

    // map is used for duplicate 'search'
    std::vector < std::vector<Vector3> > uniqueVertices;

    // generate list of unique vertices and an array linking vertexIDs with uniqueVertexIDs
    int *uniqueIndices = new int[elementOffset / 3];
    for (int v = 0; v < elementOffset; v += 3) {
      std::vector<Vector3> elementalVertex;
      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        elementalVertex.push_back(Vector3(meshRef.data[v + e * elementOffset], meshRef.data[v + e * elementOffset + 1], meshRef.data[v + e * elementOffset + 2]));
        // test: if (e == 2) elementalVertex.at(elementalVertex.size() - 1) += 0.2f;
      }

      // see if this one already exists; if not, add
      bool duplicate = false;
      int index = 0;
      for (unsigned int i = 0; i < uniqueVertices.size(); i++) {
        if (uniqueVertices[i][0] == elementalVertex[0] &&
            uniqueVertices[i][2] == elementalVertex[2]) { // texcoord also needs to be shared
          duplicate = true;
          index = i;
          break;
        }
      }
      if (!duplicate) {
        uniqueVertices.push_back(elementalVertex);
        index = uniqueVertices.size() - 1;
      }
      uniqueIndices[v / 3] = index;
    }

    //printf("vertices: %i, unique vertices: %i\n", elementOffset / 3, uniqueVertices.size());
    //printf("meshRef.size: %i, uniqueIndices.size: %i\n", meshRef.size, elementOffset / 3);

    uniqueMesh.size = uniqueVertices.size() * 3 * GetTriangleMeshElementCount();
    uniqueMesh.data = new float[uniqueMesh.size];

    int uniqueElementOffset = uniqueMesh.size / GetTriangleMeshElementCount();

    assert((unsigned int)uniqueMesh.size == uniqueVertices.size() * GetTriangleMeshElementCount() * 3);

    for (unsigned int v = 0; v < uniqueVertices.size(); v++) {
      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 0] = uniqueVertices.at(v).at(e).coords[0];
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 1] = uniqueVertices.at(v).at(e).coords[1];
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 2] = uniqueVertices.at(v).at(e).coords[2];
      }
    }

    for (int v = 0; v < uniqueElementOffset; v += 3) {

      Vector3 vertexPos(uniqueMesh.data[v], uniqueMesh.data[v + 1], uniqueMesh.data[v + 2]);

      WeightedVertex weightedVertex;
      weightedVertex.vertexID = v / 3;

      if (colorCoords.find(vertexPos) == colorCoords.end()) {
        printf("color coord not found: %f, %f, %f\n", vertexPos.coords[0], vertexPos.coords[1], vertexPos.coords[2]);
      }
      assert(colorCoords.find(vertexPos) != colorCoords.end());
      const Vector3 &color = colorCoords.find(vertexPos)->second;
      uniqueMesh.data[v + 0] *= zMultiplier;
      uniqueMesh.data[v + 1] *= zMultiplier;
      uniqueMesh.data[v + 2] *= zMultiplier;

      float totalWeight = 0.0;
      WeightedBone weightedBones[3];
      for (int c = 0; c < 3; c++) {
        int jointID = floor(color.coords[c] * 0.1);
        float weight = (color.coords[c] - jointID * 10.0) / 9.0;

        weightedBones[c].jointID = jointID;
        weightedBones[c].weight = weight;

        totalWeight += weight;
      }

      // total weight has to be 1.0;
      for (int c = 0; c < 3; c++) {
        if (c == 0) {
          if (weightedBones[c].weight == 0.f) printf("offending jointID: %i (coord %i) (vertexpos %f, %f, %f)\n", weightedBones[c].jointID, c, vertexPos.coords[0], vertexPos.coords[1], vertexPos.coords[2]);
          assert(weightedBones[c].weight != 0.f);
        }
        if (weightedBones[c].weight > 0.01f) {
          weightedBones[c].weight /= totalWeight;
          weightedVertex.bones.push_back(weightedBones[c]);
        }
      }

      weightedVerticesVec.at(subgeom).push_back(weightedVertex);
    }

    uniqueFullbodyMesh.push_back(uniqueMesh);
    uniqueIndicesVec.push_back(uniqueIndices);


    // update geometry object so that it uses indices & shared vertices

    assert(materializedTriangleMeshes.at(subgeom).verticesDataSize / GetTriangleMeshElementCount() == elementOffset);
    assert(uniqueMesh.size == uniqueElementOffset * GetTriangleMeshElementCount());
    delete [] materializedTriangleMeshes.at(subgeom).vertices;
    materializedTriangleMeshes.at(subgeom).vertices = new float[uniqueMesh.size];
    memcpy(materializedTriangleMeshes.at(subgeom).vertices, uniqueMesh.data, uniqueMesh.size * sizeof(float));
    materializedTriangleMeshes.at(subgeom).verticesDataSize = uniqueMesh.size;
    materializedTriangleMeshes.at(subgeom).indices.clear();
    for (int v = 0; v < elementOffset; v += 3) {
      materializedTriangleMeshes.at(subgeom).indices.push_back(uniqueIndices[v / 3]);
    }

/*
    printf("\n");
    printf("optimized: ");
    for (int i = 0; i < elementOffset; i += 3) {
      printf("%f, ", materializedTriangleMeshes.at(subgeom).vertices[uniqueIndices[i / 3] * 3 + uniqueElementOffset * 2]);
    }
    printf("\n");
    printf("\n");
*/

  } // subgeom

  fullbodyGeometryData->resourceMutex.unlock();
  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints.at(i).orientation = jointsVec[i]->GetDerivedRotation().GetInverse().GetNormalized();
  }

  Animation *straightAnim = new Animation();
  straightAnim->Load("media/animations/straight.anim.util");
  animApplyBuffer.anim = straightAnim;
  animApplyBuffer.anim->Apply(nodeMap, animApplyBuffer.frameNum, 0, animApplyBuffer.smooth, animApplyBuffer.smoothFactor, animApplyBuffer.position, animApplyBuffer.orientation, animApplyBuffer.offsets, 0, false, true);

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints.at(i).position = jointsVec[i]->GetDerivedPosition();// * zMultiplier;
  }

  if (Verbose()) printf("6\n");

  UpdateFullbodyModel(true);
  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData(false);

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints.at(i).origPos = jointsVec[i]->GetDerivedPosition();
  }

  delete straightAnim;
  delete baseAnim;

  //printf("vertexcount: %i, unique vertexcount: %i\n", elementOffset / 3, uniqueElementOffset / 3);
}

void HumanoidBase::UpdateFullbodyNodes() {

  Vector3 previousFullbodyOffset = fullbodyOffset;
  fullbodyOffset = humanoidNode->GetPosition().Get2D();
  fullbodyNode->SetPosition(fullbodyOffset);

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints[i].orientation = joints[i].node->GetDerivedRotation();
    joints[i].position = joints[i].node->GetDerivedPosition() - fullbodyOffset;
  }

  // todo: something is wrong with the hairdo update logic, so just always update now
  /*
  if (NeedsModelUpdate()) {
    hairStyle->SetRotation(joints[2].orientation, false);
    hairStyle->SetPosition(joints[2].position * zMultiplier + fullbodyOffset, false);
    hairStyle->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  } else {
    hairStyle->SetPosition(hairStyle->GetPosition() + (fullbodyOffset - previousFullbodyOffset), false);
    hairStyle->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }
  */
  hairStyle->SetRotation(joints[2].orientation, false);
  hairStyle->SetPosition(joints[2].position * zMultiplier + fullbodyOffset, false);
  hairStyle->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

  //hairStyle->SetRotation(nodeMap.find("neck")->second->GetDerivedRotation());
  //hairStyle->SetPosition(nodeMap.find("neck")->second->GetDerivedPosition() + joints[2].orientation * Vector3(0, 0, player->GetPlayerData()->GetHeight() - defaultPlayerHeight));
}

bool HumanoidBase::NeedsModelUpdate() {
  if (buf_LowDetailMode && buf_bodyUpdatePhase != 1 - buf_bodyUpdatePhaseOffset) return false; else return true;
}

void HumanoidBase::UpdateFullbodyModel(bool updateSrc) {

  int uploadVertices = true;
  int uploadNormals = true;
  int uploadTangents = true;
  int uploadBitangents = true;

  boost::intrusive_ptr < Resource<GeometryData> > fullbodyGeometryData = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  fullbodyGeometryData->resourceMutex.lock();
  std::vector < MaterializedTriangleMesh > &materializedTriangleMeshes = fullbodyGeometryData->GetResource()->GetTriangleMeshesRef();

  for (unsigned int subgeom = 0; subgeom < fullbodySubgeomCount; subgeom++) {

    FloatArray &uniqueMesh = uniqueFullbodyMesh.at(subgeom);

    const std::vector<WeightedVertex> &weightedVertices = weightedVerticesVec.at(subgeom);

    int uniqueVertexCount = weightedVertices.size();

    int uniqueElementOffset = uniqueMesh.size / GetTriangleMeshElementCount();

    Vector3 origVertex;
    Vector3 origNormal;
    Vector3 origTangent;
    Vector3 origBitangent;
    Vector3 resultVertex;
    Vector3 resultNormal;
    Vector3 resultTangent;
    Vector3 resultBitangent;
    Vector3 adaptedVertex;
    Vector3 adaptedNormal;
    Vector3 adaptedTangent;
    Vector3 adaptedBitangent;

    for (int v = 0; v < uniqueVertexCount; v++) {
      if (uploadVertices)   memcpy(origVertex.coords,    &uniqueMesh.data[weightedVertices[v].vertexID * 3],                           3 * sizeof(float)); // was: uniqueFullbodyMeshSrc
      if (uploadNormals)    memcpy(origNormal.coords,    &uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset],     3 * sizeof(float));
      if (uploadTangents)   memcpy(origTangent.coords,   &uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 3], 3 * sizeof(float));
      if (uploadBitangents) memcpy(origBitangent.coords, &uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 4], 3 * sizeof(float));

      if (weightedVertices[v].bones.size() == 1) {

        if (uploadVertices) {
          resultVertex = origVertex;
          resultVertex -= joints[weightedVertices[v].bones[0].jointID].origPos * zMultiplier;
          resultVertex.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);
          resultVertex += joints[weightedVertices[v].bones[0].jointID].position * zMultiplier;
        }

        if (uploadNormals) {
          resultNormal = origNormal;
          resultNormal.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);
        }

        if (uploadTangents) {
          resultTangent = origTangent;
          resultTangent.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);
        }

        if (uploadBitangents) {
          resultBitangent = origBitangent;
          resultBitangent.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);
        }

      } else {

        if (uploadVertices)   resultVertex.Set(0);
        if (uploadNormals)    resultNormal.Set(0);
        if (uploadTangents)   resultTangent.Set(0);
        if (uploadBitangents) resultBitangent.Set(0);

        for (unsigned int b = 0; b < weightedVertices[v].bones.size(); b++) {

          if (uploadVertices) {
            adaptedVertex = origVertex;
            adaptedVertex -= joints[weightedVertices[v].bones[b].jointID].origPos * zMultiplier;
            adaptedVertex.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
            adaptedVertex += joints[weightedVertices[v].bones[b].jointID].position * zMultiplier;
            resultVertex += adaptedVertex * weightedVertices[v].bones[b].weight;
          }

          if (uploadNormals) {
            adaptedNormal = origNormal;
            adaptedNormal.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
            resultNormal += adaptedNormal * weightedVertices[v].bones[b].weight;
          }

          if (uploadTangents) {
            adaptedTangent = origTangent;
            adaptedTangent.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
            resultTangent += adaptedTangent * weightedVertices[v].bones[b].weight;
          }

          if (uploadBitangents) {
            adaptedBitangent = origBitangent;
            adaptedBitangent.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
            resultBitangent += adaptedBitangent * weightedVertices[v].bones[b].weight;
          }

        }

        if (uploadNormals)    resultNormal.FastNormalize();
        if (uploadTangents)   resultTangent.FastNormalize();
        if (uploadBitangents) resultBitangent.FastNormalize();

      }

      if (updateSrc) {
        if (uploadVertices)   memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3],                           resultVertex.coords,    3 * sizeof(float));
        if (uploadNormals)    memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset],     resultNormal.coords,    3 * sizeof(float));
        if (uploadTangents)   memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 3], resultTangent.coords,   3 * sizeof(float));
        if (uploadBitangents) memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 4], resultBitangent.coords, 3 * sizeof(float));
      }

      if (uploadVertices)   memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3],                           resultVertex.coords,    3 * sizeof(float));
      if (uploadNormals)    memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3 + uniqueElementOffset],     resultNormal.coords,    3 * sizeof(float));
      if (uploadTangents)   memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 3], resultTangent.coords,   3 * sizeof(float));
      if (uploadBitangents) memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 4], resultBitangent.coords, 3 * sizeof(float));
    }

  } // subgeom

  fullbodyGeometryData->resourceMutex.unlock();

}

/* moved this to gametask upload thread, so it can be multithreaded, whilst assuring its lifetime
void HumanoidBase::UploadFullbodyModel() {
  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData(false);
}
*/

void HumanoidBase::Process() {

  _cache_AgilityFactor = GetConfiguration()->GetReal("gameplay_agilityfactor", _default_AgilityFactor);
  _cache_AccelerationFactor = GetConfiguration()->GetReal("gameplay_accelerationfactor", _default_AccelerationFactor);

  decayingPositionOffset *= 0.95f;
  if (decayingPositionOffset.GetLength() < 0.005) decayingPositionOffset.Set(0);
  decayingDifficultyFactor = clamp(decayingDifficultyFactor - 0.002f, 0.0f, 1.0f);


  assert(match);

  if (!currentMentalImage) {
    currentMentalImage = match->GetMentalImage(0);
  }

  CalculateSpatialState();
  spatialState.positionOffsetMovement = Vector3(0);

  currentAnim->frameNum++;
  previousAnim->frameNum++;


  if (currentAnim->frameNum == currentAnim->anim->GetFrameCount() - 1 && interruptAnim == e_InterruptAnim_None) {
    interruptAnim = e_InterruptAnim_Switch;
  }


  bool mayReQueue = false;


  // already some anim interrupt waiting?

  if (mayReQueue) {
    if (interruptAnim != e_InterruptAnim_None) {
      mayReQueue = false;
    }
  }


  // okay, see if we need to requeue

  if (mayReQueue) {
    interruptAnim = e_InterruptAnim_ReQueue;
  }

  if (interruptAnim != e_InterruptAnim_None) {

    PlayerCommandQueue commandQueue;

    if (interruptAnim == e_InterruptAnim_Trip && tripType != 0) {
      AddTripCommandToQueue(commandQueue, tripDirection, tripType);
      tripType = 0;
      commandQueue.push_back(GetBasicMovementCommand(tripDirection, spatialState.floatVelocity)); // backup, if there's no applicable trip anim
    } else {
      player->RequestCommand(commandQueue);
    }


    // iterate through the command queue and pick the first that is applicable

    bool found = false;
    for (unsigned int i = 0; i < commandQueue.size(); i++) {

      const PlayerCommand &command = commandQueue.at(i);

      found = SelectAnim(command, interruptAnim);
      if (found) break;
    }

    if (interruptAnim != e_InterruptAnim_ReQueue && !found) {
      printf("RED ALERT! NO APPLICABLE ANIM FOUND FOR HUMANOIDBASE! NOOOO!\n");
      printf("currentanimtype: %s\n", currentAnim->anim->GetVariable("type").c_str());
      for (unsigned int i = 0; i < commandQueue.size(); i++) {
        printf("desiredanimtype: %i\n", commandQueue.at(i).desiredFunctionType);
      }
    }

    if (found) {
      startPos = spatialState.position;
      startAngle = spatialState.angle;

      CalculatePredictedSituation(nextStartPos, nextStartAngle);

      animApplyBuffer.anim = currentAnim->anim;
      animApplyBuffer.smooth = true;
      animApplyBuffer.smoothFactor = (interruptAnim == e_InterruptAnim_Switch) ? 0.6f : 1.0f;

      // decaying difficulty
      float animDiff = atof(currentAnim->anim->GetVariable("animdifficultyfactor").c_str());
      if (animDiff > decayingDifficultyFactor) decayingDifficultyFactor = animDiff;
      //if (player->GetDebug()) printf("difficulty: %f\n", decayingDifficultyFactor);

      // if we just requeued, for example, from movement to ballcontrol, there's no reason we can not immediately requeue to another ballcontrol again (next time). only apply the initial requeue delay on subsequent anims of the same type
      // (so we can have a fast ballcontrol -> ballcontrol requeue, but after that, use the initial delay)
      if (interruptAnim == e_InterruptAnim_ReQueue && previousAnim->functionType == currentAnim->functionType) {
        reQueueDelayFrames = initialReQueueDelayFrames; // don't try requeueing (some types of anims, see selectanim()) too often
      }

    }

  }
  reQueueDelayFrames = clamp(reQueueDelayFrames - 1, 0, 10000);


  interruptAnim = e_InterruptAnim_None;

  if (startPos.coords[2] != 0.f) {
    // the z coordinate not being 0 denotes something went horribly wrong :P
    Log(e_FatalError, "HumanoidBase", "Process", "BWAAAAAH FLYING PLAYERS!! height: " + real_to_str(startPos.coords[2]));
  }


  // movement/rotation smuggle

  // start with +1, because we want to influence the first frame as well
  // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
  // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
  float frameBias = (currentAnim->frameNum + 1) / (float)(currentAnim->anim->GetEffectiveFrameCount() + 1);

  // not sure if this is correct!
  // radian beginAngle = currentAnim->rotationSmuggle.begin;// * (1.0f - frameBias); // more influence in the beginning; act more like it was 0 later on. (yes, next one is a bias within a bias) *edit: disabled, looks better without
  // currentAnim->rotationSmuggleOffset = beginAngle * (1.0f - frameBias) +
  //                                      currentAnim->rotationSmuggle.end * frameBias;

  currentAnim->rotationSmuggleOffset = currentAnim->rotationSmuggle.begin * (1.0f - frameBias) +
                                       currentAnim->rotationSmuggle.end * frameBias;


  // next frame

  animApplyBuffer.frameNum = currentAnim->frameNum;

  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    animApplyBuffer.position = startPos + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset + currentAnim->positions.at(currentAnim->frameNum);
    animApplyBuffer.orientation = startAngle + currentAnim->rotationSmuggleOffset;
    animApplyBuffer.noPos = true;
  } else {
    if (player->GetDebug()) printf("ERROR: %u, %u (%s)\n", (unsigned int)currentAnim->positions.size(), (unsigned int)currentAnim->frameNum, currentAnim->anim->GetName().c_str());
    animApplyBuffer.position = startPos + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
    animApplyBuffer.orientation = startAngle;
    animApplyBuffer.noPos = false;
  }

  animApplyBuffer.offsets = offsets;
}

void HumanoidBase::PreparePutBuffers(unsigned long snapshotTime_ms) {

  // offsets
  CalculateGeomOffsets(); // todo: in a perfect world, we don't want to do cpu intensive and/or stuff that uses a lot of mutex locking in this here function

  buf_animApplyBuffer = animApplyBuffer;
  buf_animApplyBuffer.snapshotTime_ms = snapshotTime_ms;
  /*
  // some temporal buffers fail when switching anims - can't interpolate between these values from previous and new anim
  if (currentAnim->frameNum == 0) {
    buf_animApplyBuffer_TemporalOrientationOffset.Clear();
    buf_animApplyBuffer_FrameNum.Clear();
  }
  buf_animApplyBuffer_TemporalPosition.SetValue(animApplyBuffer.position);
  buf_animApplyBuffer_TemporalOrientationOffset.SetValue(animApplyBuffer.orientationOffset);
  buf_animApplyBuffer_FrameNum.SetValue(animApplyBuffer.frameNum);
  */

  // display humanoids farther away from action at half FPS
  buf_LowDetailMode = false;
  if (!player->GetExternalController() && !match->GetPause()) {
    Vector3 focusPos = match->GetBall()->Predict(100).Get2D();
    if (match->GetDesignatedPossessionPlayer()) {
      focusPos = focusPos * 0.5f + match->GetDesignatedPossessionPlayer()->GetPosition() * 0.5f;
    }

    if ((spatialState.position - focusPos).GetLength() > 14.0f) buf_LowDetailMode = true;
  }

}

void HumanoidBase::FetchPutBuffers(unsigned long putTime_ms) {

  fetchedbuf_animApplyBuffer = buf_animApplyBuffer;
  assert(fetchedbuf_animApplyBuffer.anim == buf_animApplyBuffer.anim);

  fetchedbuf_LowDetailMode = buf_LowDetailMode;
  buf_bodyUpdatePhase++;
  if (buf_bodyUpdatePhase == 2) buf_bodyUpdatePhase = 0;
  fetchedbuf_bodyUpdatePhase = buf_bodyUpdatePhase;
  fetchedbuf_bodyUpdatePhaseOffset = buf_bodyUpdatePhaseOffset;
}

void HumanoidBase::Put() {

  //unsigned long timeDiff_ms = match->GetTimeSincePreviousPut_ms();//EnvironmentManager::GetInstance().GetTime_ms() - match->GetPreviousTime_ms();
  // the apply function doesn't know better than that it is displaying snapshot times, so continue this hoax into the timeDiff_ms value. then,
  // the temporalsmoother will convert it to 'realtime' once again
  unsigned long timeDiff_ms = fetchedbuf_animApplyBuffer.snapshotTime_ms - fetchedbuf_previousSnapshotTime_ms;
  //printf("diff: %lu\n", timeDiff_ms);
  timeDiff_ms = clamp(timeDiff_ms, 10, 50);
  fetchedbuf_previousSnapshotTime_ms = fetchedbuf_animApplyBuffer.snapshotTime_ms;

  for (unsigned int i = 0; i < buf_TemporalHumanoidNodes.size(); i++) {
    // first, restore the previous non-temporal-smoothed values, so the Apply() function can use them for smoothing
    buf_TemporalHumanoidNodes.at(i).actualNode->SetPosition(buf_TemporalHumanoidNodes.at(i).cachedPosition, false);
    buf_TemporalHumanoidNodes.at(i).actualNode->SetRotation(buf_TemporalHumanoidNodes.at(i).cachedOrientation, false);
  }

  humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

  //printf("anim ptr: %i\n", fetchedbuf_animApplyBuffer.anim);
  //printf("nodemap size: %i\n", nodeMap.size());
  fetchedbuf_animApplyBuffer.anim->Apply(nodeMap, fetchedbuf_animApplyBuffer.frameNum, -1, fetchedbuf_animApplyBuffer.smooth, fetchedbuf_animApplyBuffer.smoothFactor, fetchedbuf_animApplyBuffer.position, fetchedbuf_animApplyBuffer.orientation, fetchedbuf_animApplyBuffer.offsets, &movementHistory, timeDiff_ms, fetchedbuf_animApplyBuffer.noPos, false);

  humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

  // we've just set the humanoid positions for time fetchedbuf_animApplyBuffer.snapshotTime_ms. however, it's eventually going to be displayed in a historic position, for temporal smoothing.
  // thus; read out the current values we've just set, insert them in the temporal smoother, and get the historic spatial data instead (GetValue).
  for (unsigned int i = 0; i < buf_TemporalHumanoidNodes.size(); i++) {
    // save the non-historic version in cachedNode
    buf_TemporalHumanoidNodes.at(i).cachedPosition = buf_TemporalHumanoidNodes.at(i).actualNode->GetPosition();
    buf_TemporalHumanoidNodes.at(i).cachedOrientation = buf_TemporalHumanoidNodes.at(i).actualNode->GetRotation();

    // sudden realisation: by only SetValue'ing here instead of in prepareputbuffers, aren't we missing out on valuable interpolateable data?
    buf_TemporalHumanoidNodes.at(i).position.SetValue(buf_TemporalHumanoidNodes.at(i).actualNode->GetPosition(), fetchedbuf_animApplyBuffer.snapshotTime_ms);
    buf_TemporalHumanoidNodes.at(i).orientation.SetValue(buf_TemporalHumanoidNodes.at(i).actualNode->GetRotation(), fetchedbuf_animApplyBuffer.snapshotTime_ms);

    buf_TemporalHumanoidNodes.at(i).actualNode->SetPosition(buf_TemporalHumanoidNodes.at(i).position.GetValue(match->GetPreviousPutTime_ms()), false);
    buf_TemporalHumanoidNodes.at(i).actualNode->SetRotation(buf_TemporalHumanoidNodes.at(i).orientation.GetValue(match->GetPreviousPutTime_ms()), false);
  }

  humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

  UpdateFullbodyNodes();
}

void HumanoidBase::CalculateGeomOffsets() {
  /* todo
  SetOffset("middle", 0.0, QUATERNION_IDENTITY);
  SetOffset("neck", 0.0, QUATERNION_IDENTITY);
  SetOffset("left_shoulder", 0.0, QUATERNION_IDENTITY);
  SetOffset("right_shoulder", 0.0, QUATERNION_IDENTITY);
  SetOffset("left_elbow", 0.0, QUATERNION_IDENTITY);
  SetOffset("right_elbow", 0.0, QUATERNION_IDENTITY);
  SetOffset("body", 0.0, QUATERNION_IDENTITY);

  if (currentAnim->functionType == e_FunctionType_Movement && match->IsInPlay()) {

    // aim a bit towards ball

    Vector3 toBall = (currentMentalImage->GetBallPrediction(10) - spatialState.position).GetNormalized(spatialState.directionVec);

    // middle
    radian bx, by, bz;
    nodeMap.find("body")->second->GetDerivedRotation().GetAngles(bx, by, bz);

    // head
    radian mx, my, mz;
    nodeMap.find("middle")->second->GetDerivedRotation().GetAngles(mx, my, mz);

    // middle
    radian angle = FixAngle(toBall.GetAngle2D() - bz);
    if (fabs(angle) < 0.8 * pi) {
      angle = clamp(angle, -0.5 * pi, 0.5 * pi);
      Quaternion middleOrientation;
      middleOrientation.SetAngleAxis(angle, Vector3(0, 0, 1));
      Quaternion rotX;
      rotX.SetAngleAxis(0.05f * pi, Vector3(1, 0, 0));
      middleOrientation = rotX * middleOrientation;
      float bias = clamp(0.25f * pi - (fabs(mx) + fabs(my)) - 0.15f, 0, 0.25f * pi) * 0.4f;
      SetOffset("middle", bias, middleOrientation.GetNormalized());
    }

    // head
    angle = FixAngle(toBall.GetAngle2D() - mz);
    if (fabs(angle) < 0.8 * pi) {
      angle = clamp(angle, -0.5 * pi, 0.5 * pi);
      Quaternion neckOrientation;
      neckOrientation.SetAngleAxis(angle, Vector3(0, 0, 1));
      SetOffset("neck", 0.6, neckOrientation.GetNormalized());
    }
  }
  */
}

void HumanoidBase::SetOffset(const std::string &nodeName, float bias, const Quaternion &orientation, bool isRelative) {
  std::map < std::string, BiasedOffset >::iterator iter = offsets.find(nodeName);
  if (iter == offsets.end()) {
    if (bias != 0) {
      BiasedOffset biasedOffset;
      biasedOffset.bias = bias;
      biasedOffset.orientation = orientation;
      biasedOffset.isRelative = isRelative;
      offsets.insert(std::pair<std::string, BiasedOffset>(nodeName, biasedOffset));
    }
  } else {
    if (bias != 0) {
      BiasedOffset biasedOffset;
      biasedOffset.bias = bias;
      biasedOffset.orientation = orientation;
      biasedOffset.isRelative = isRelative;
      iter->second = biasedOffset;
    } else {
      offsets.erase(iter);
    }
  }
}

int HumanoidBase::GetIdleMovementAnimID() {
  CrudeSelectionQuery query;
  query.byFunctionType = true;
  query.functionType = e_FunctionType_Movement;
  query.byIncomingVelocity = true;
  query.incomingVelocity = e_Velocity_Idle;
  query.byOutgoingVelocity = true;
  query.outgoingVelocity = e_Velocity_Idle;

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);
  if (Verbose()) if (dataSet.size() == 0) printf("no animations to begin with\n");

  int desiredIdleLevel = 1;
  SetNumericVariableSimilarityPredicate("idlelevel", desiredIdleLevel);
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&Humanoid::CompareNumericVariable, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareNumericVariable, this, _1, _2));
  #endif

  SetIncomingBodyDirectionSimilarityPredicate(Vector3(0, -1, 0));
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&Humanoid::CompareIncomingBodyDirectionSimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIncomingBodyDirectionSimilarity, this, _1, _2));
  #endif

  SetIncomingVelocitySimilarityPredicate(e_Velocity_Idle);
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&Humanoid::CompareIncomingVelocitySimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIncomingVelocitySimilarity, this, _1, _2));
  #endif

  SetMovementSimilarityPredicate(Vector3(0, -1, 0), e_Velocity_Idle);
  SetBodyDirectionSimilarityPredicate(spatialState.position + Vector3(0, -10, 0).GetRotated2D(spatialState.angle)); // lookat
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&HumanoidBase::CompareBodyDirectionSimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareBodyDirectionSimilarity, this, _1, _2));
  #endif

  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
  #endif

  //printf("%s\n", anims->GetAnim(*dataSet.begin())->GetName().c_str());

  return *dataSet.begin();
}

void HumanoidBase::ResetPosition(const Vector3 &newPos, const Vector3 &focusPos) {

  startPos = newPos;
  startAngle = FixAngle((focusPos - newPos).GetNormalized(Vector3(0, -1, 0)).GetAngle2D());
  nextStartPos = startPos;
  nextStartAngle = startAngle;
  previousPosition2D = startPos;

  spatialState.position = startPos;
  spatialState.angle = startAngle;
  spatialState.directionVec = Vector3(0, -1, 0).GetRotated2D(startAngle);
  spatialState.floatVelocity = 0;
  spatialState.enumVelocity = e_Velocity_Idle;
  spatialState.movement = Vector3(0);
  spatialState.relBodyDirectionVec = Vector3(0, -1, 0);
  spatialState.relBodyAngle = 0;
  spatialState.bodyDirectionVec = Vector3(0, -1, 0);
  spatialState.bodyAngle = 0;
  spatialState.foot = e_Foot_Right;

  int idleAnimID = GetIdleMovementAnimID();
  currentAnim->id = idleAnimID;
  currentAnim->anim = anims->GetAnim(currentAnim->id);
  currentAnim->positions.clear();
  currentAnim->positions = match->GetAnimPositionCache(currentAnim->anim);
  currentAnim->frameNum = random(0, currentAnim->anim->GetEffectiveFrameCount() - 1);
  currentAnim->radiusOffset = 0.0;
  currentAnim->touchFrame = -1;
  currentAnim->originatingInterrupt = e_InterruptAnim_None;
  currentAnim->fullActionSmuggle = Vector3(0);
  currentAnim->actionSmuggle = Vector3(0);
  currentAnim->actionSmuggleOffset = Vector3(0);
  currentAnim->actionSmuggleSustain = Vector3(0);
  currentAnim->actionSmuggleSustainOffset = Vector3(0);
  currentAnim->movementSmuggle = Vector3(0);
  currentAnim->movementSmuggleOffset = Vector3(0);
  currentAnim->rotationSmuggle.begin = 0;
  currentAnim->rotationSmuggle.end = 0;
  currentAnim->rotationSmuggleOffset = 0;
  currentAnim->functionType = e_FunctionType_Movement;
  currentAnim->incomingMovement = Vector3(0);
  currentAnim->outgoingMovement = Vector3(0);
  currentAnim->positionOffset = Vector3(0);

  previousAnim->id = idleAnimID;
  previousAnim->anim = currentAnim->anim;
  previousAnim->positions.clear();
  previousAnim->positions = match->GetAnimPositionCache(previousAnim->anim);
  previousAnim->frameNum = 0;
  previousAnim->radiusOffset = 0.0;
  previousAnim->touchFrame = -1;
  previousAnim->originatingInterrupt = e_InterruptAnim_None;
  previousAnim->fullActionSmuggle = Vector3(0);
  previousAnim->actionSmuggle = Vector3(0);
  previousAnim->actionSmuggleOffset = Vector3(0);
  previousAnim->actionSmuggleSustain = Vector3(0);
  previousAnim->actionSmuggleSustainOffset = Vector3(0);
  previousAnim->movementSmuggle = Vector3(0);
  previousAnim->movementSmuggleOffset = Vector3(0);
  previousAnim->rotationSmuggle.begin = 0;
  previousAnim->rotationSmuggle.end = 0;
  previousAnim->rotationSmuggleOffset = 0;
  previousAnim->functionType = e_FunctionType_Movement;
  previousAnim->incomingMovement = Vector3(0);
  previousAnim->outgoingMovement = Vector3(0);
  previousAnim->positionOffset = Vector3(0);

  humanoidNode->SetPosition(startPos, false);

  animApplyBuffer.anim = currentAnim->anim;
  animApplyBuffer.smooth = false;
  animApplyBuffer.smoothFactor = 0.0f;
  animApplyBuffer.position = startPos;
  animApplyBuffer.orientation = startAngle;
  animApplyBuffer.offsets.clear();
  buf_animApplyBuffer = animApplyBuffer;

  interruptAnim = e_InterruptAnim_None;
  tripType = 0;

  decayingPositionOffset = Vector3(0);
  decayingDifficultyFactor = 0.0f;

  movementHistory.clear();

  // clear temporalsmoother vars
  // todo: not sure if we may access buf_ vars here
/*
  for (unsigned int i = 0; i < buf_TemporalHumanoidNodes.size(); i++) {
    buf_TemporalHumanoidNodes.at(i).position.Clear();
    buf_TemporalHumanoidNodes.at(i).orientation.Clear();
  }
  buf_animApplyBuffer_TemporalPosition.Clear();
  buf_animApplyBuffer_TemporalOrientationOffset.Clear();
  buf_animApplyBuffer_FrameNum.Clear();
*/
}

void HumanoidBase::OffsetPosition(const Vector3 &offset) {
  // todo: move this to team process or disable it altogether
  // ponder on the consequences and decide!
  // update: seems to function well as is atm

  assert(offset.coords[2] == 0.0f);

  float cheat = 1.0f;

  nextStartPos += offset * cheat;
  startPos += offset * cheat;
  spatialState.position += offset * cheat;
  spatialState.positionOffsetMovement += offset * 100.0f * cheat;
  decayingPositionOffset += offset * cheat;
  if (decayingPositionOffset.GetLength() > 0.1f) decayingPositionOffset = decayingPositionOffset.GetNormalized() * 0.1f;
  currentAnim->positionOffset += offset * cheat;
}

void HumanoidBase::TripMe(const Vector3 &tripVector, int tripType) {
  if (match->GetBallRetainer() == player) return;
  if (currentAnim->anim->GetVariable("incoming_special_state").compare("") == 0 && currentAnim->anim->GetVariable("outgoing_special_state").compare("") == 0) {
    if (this->interruptAnim == e_InterruptAnim_None && (currentAnim->functionType != e_FunctionType_Trip || (currentAnim->anim->GetVariable("triptype").compare("1") == 0 && tripType > 1))
                                                    && currentAnim->functionType != e_FunctionType_Sliding) {
      this->interruptAnim = e_InterruptAnim_Trip;
      this->tripDirection = tripVector;
      this->tripType = tripType;
    }
  }
}

void HumanoidBase::SetKit(boost::intrusive_ptr < Resource<Surface> > newKit) {
  if (Verbose()) printf("setting new kit: %s\n", newKit->GetIdentString().c_str());
  boost::intrusive_ptr< Resource<GeometryData> > bodyGeom = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();

  bodyGeom->resourceMutex.lock();

  std::vector < MaterializedTriangleMesh > &tmesh = bodyGeom->GetResource()->GetTriangleMeshesRef();

  if (newKit != boost::intrusive_ptr< Resource<Surface> >()) {
    for (unsigned int i = 0; i < tmesh.size(); i++) {
      if (tmesh.at(i).material.diffuseTexture != boost::intrusive_ptr< Resource<Surface> >()) {
        if (tmesh.at(i).material.diffuseTexture->GetIdentString() == kitDiffuseTextureIdentString) {
          tmesh.at(i).material.diffuseTexture = newKit;
          tmesh.at(i).material.specular_amount = 0.01f;//0.02f;//0.033f;//0.01f;
          tmesh.at(i).material.shininess = 0.01f;//0.005f;
        }
      } else if (Verbose()) printf("no texture!\n");
    }
    kitDiffuseTextureIdentString = newKit->GetIdentString();
  }

  bodyGeom->resourceMutex.unlock();

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();
}

void HumanoidBase::ResetSituation(const Vector3 &focusPos) {
  currentMentalImage = 0;

  ResetPosition(spatialState.position, focusPos);
}

bool HumanoidBase::_HighOrBouncyBall() const {
  float ballHeight1 = match->GetBall()->Predict(10).coords[2];
  float ballHeight2 = match->GetBall()->Predict(defaultTouchOffset_ms).coords[2];
  float ballBounce = fabs(match->GetBall()->GetMovement().coords[2]);
  bool highBall = false;
  if (ballHeight1 > 0.3f || ballHeight2 > 0.3f) {
    highBall = true;
  } else if (ballBounce > 1.0f) { // low balls are also treated as 'high ball' when there's a lot of bounce going on (hard to control)
    highBall = true;
  }
  return highBall;
}

// ALERT: set sorting predicates before calling this function
void HumanoidBase::_KeepBestDirectionAnims(DataSet &dataSet, const PlayerCommand &command, bool strict, radian allowedAngle, int allowedVelocitySteps, int forcedQuadrantID) {

  assert(dataSet.size() != 0);

  int bestQuadrantID = forcedQuadrantID;
  if (bestQuadrantID == -1) {

    #ifdef dataSetSortable
    dataSet.sort(boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
    #else
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
    #endif

    // we want the best anim to be a baseanim, and compare other anims to it
    if (strict) {
      if (command.desiredFunctionType != e_FunctionType_Movement) {
        #ifdef dataSetSortable
        dataSet.sort(boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
        #else
        std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
        #endif
      }
    }

    Animation *bestAnim = anims->GetAnim(*dataSet.begin());

    bestQuadrantID = atoi(bestAnim->GetVariable("quadrant_id").c_str());
  }

  const Quadrant &quadrant = anims->GetQuadrant(bestQuadrantID);

  DataSet::iterator iter = dataSet.begin();
  iter++;
  while (iter != dataSet.end()) {
    Animation *anim = anims->GetAnim(*iter);

    if (strict) {
      if (atoi(anim->GetVariable("quadrant_id").c_str()) == bestQuadrantID) {
        //if (player->GetDebug() && bestAnim->GetAnimType().compare("movement") == 0) printf("keeping %s\n", anim->GetName().c_str());
        iter++;
      } else {
        //if (player->GetDebug() && bestAnim->GetAnimType().compare("movement") == 0) printf("deleting %s\n", anim->GetName().c_str());
        iter = dataSet.erase(iter);
      }
    } else {
      int quadrantID = atoi(anim->GetVariable("quadrant_id").c_str());
      const Quadrant &bestQuadrant = anims->GetQuadrant(bestQuadrantID);
      const Quadrant &quadrant = anims->GetQuadrant(quadrantID);

      bool predicate = true;

      if (anim->GetVariable("lastditch").compare("true") != 0) { // last ditch anims may always change velo
        if (abs(GetVelocityID(quadrant.velocity, true) - GetVelocityID(bestQuadrant.velocity, true)) > allowedVelocitySteps) predicate = false;
      }
      if (fabs(quadrant.angle - bestQuadrant.angle) > allowedAngle) predicate = false;

      if (predicate) {
        //if (player->GetDebug() && bestAnim->GetAnimType().compare("movement") == 0) printf("keeping %s\n", anim->GetName().c_str());
        iter++;
      } else {
        //if (player->GetDebug() && bestAnim->GetAnimType().compare("movement") == 0) printf("deleting %s (quadrant.angle %f - bestQuadrant.angle %f)\n", anim->GetName().c_str(), quadrant.angle, bestQuadrant.angle);
        iter = dataSet.erase(iter);
      }
    }

  }

  //if (player->GetDebug() && bestAnim->GetAnimType().compare("movement") == 0) printf("\n");
}

// ALERT: set sorting predicates before calling this function
void HumanoidBase::_KeepBestBodyDirectionAnims(DataSet &dataSet, const PlayerCommand &command, bool strict, radian allowedAngle) {

  // delete nonqualified bodydir quadrants

  assert(dataSet.size() != 0);

  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&HumanoidBase::CompareBodyDirectionSimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareBodyDirectionSimilarity, this, _1, _2));
  #endif

  // we want the best anim to be a baseanim, and compare other anims to it
  if (strict) {
    if (command.desiredFunctionType != e_FunctionType_Movement) {
      #ifdef dataSetSortable
      dataSet.sort(boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
      #else
      std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
      #endif
    }
  }

  Animation *bestAnim = anims->GetAnim(*dataSet.begin());

  radian bestOutgoingBodyAngle = ForceIntoAllowedBodyDirectionAngle(bestAnim->GetOutgoingBodyAngle());
  radian bestOutgoingAngle = ForceIntoPreferredDirectionAngle(bestAnim->GetOutgoingAngle());
  radian bestLookAngle = bestOutgoingBodyAngle + bestOutgoingAngle;


  DataSet::iterator iter = dataSet.begin();
  iter++;
  while (iter != dataSet.end()) {

    Animation *anim = anims->GetAnim(*iter);

    radian animOutgoingBodyAngle = ForceIntoAllowedBodyDirectionAngle(anim->GetOutgoingBodyAngle());
    radian animOutgoingAngle = ForceIntoPreferredDirectionAngle(anim->GetOutgoingAngle());
    radian animLookAngle = animOutgoingBodyAngle + animOutgoingAngle;

    float adaptedAllowedAngle = 0.06f * pi; // between 0 and 20 deg
    if (!strict) {
      adaptedAllowedAngle = allowedAngle;
    }
    if (fabs(animLookAngle - bestLookAngle) <= adaptedAllowedAngle) {
      //if (player->GetDebug() && bestAnim->GetAnimType().compare("ballcontrol") == 0) printf("keeping %s (%f, %f (%f), %f)\n", anim->GetName().c_str(), animOutgoingBodyAngle, animOutgoingAngle, anim->GetOutgoingAngle(), animLookAngle);
      iter++;
    } else {
      //if (player->GetDebug() && bestAnim->GetAnimType().compare("ballcontrol") == 0) printf("deleting %s (%f, %f (%f), %f)\n", anim->GetName().c_str(), animOutgoingBodyAngle, animOutgoingAngle, anim->GetOutgoingAngle(), animLookAngle);
      iter = dataSet.erase(iter);
    }

  }

  //if (player->GetDebug() && bestAnim->GetAnimType().compare("ballcontrol") == 0) printf("\n");
}

/* deprecated
void HumanoidBase::GetRatedDataSet(RatedDataSet &ratedDataSet, const DataSet &dataSet, const IdealAnimDescription &idealAnimDescription, float weight_foot, float weight_incomingVelocity, float weight_outgoingMovement, float weight_incomingBodyDirection, float weight_outgoingBodyDirection, float weight_baseAnim, float weight_side, float penalty_nonLinearVelocity, float penalty_nonLinearBodyDirection) {
  DataSet::const_iterator iter = dataSet.begin();

  while (iter != dataSet.end()) {

    Animation *anim = anims->GetAnim(*iter);


    // --- FOOT RATING

    float rating_foot = 0.0f;
    if (weight_foot > 0.0f) {
      if (anim->GetCurrentFoot() == idealAnimDescription.foot) rating_foot = 1.0f;
      if (FloatToEnumVelocity(anim->GetIncomingVelocity()) == e_Velocity_Idle) rating_foot = 1.0f; // actually: disable weighting?
    }


    // --- INCOMING VELOCITY

    float rating_incomingVelocity = 0.0f;
    float rating_nonLinearVelocity = 0.0f;
    if (weight_incomingVelocity > 0.0f) {
      int currentVelocityID = GetVelocityID(FloatToEnumVelocity(idealAnimDescription.incomingVelocityFloat));

      // rate difference anim incoming / actual incoming
      int anim_incomingVelocityID = GetVelocityID(FloatToEnumVelocity(anim->GetIncomingVelocity()));
      rating_incomingVelocity = 1.0f - pow(NormalizedClamp(fabs(anim_incomingVelocityID - currentVelocityID), 0, 3), 0.5f);

      // also add a penalty for anim incoming velocities which aren't between actual incoming and anim outgoing
      // todo: also make variable ratings per amount of excess?
      int anim_outgoingVelocityID = GetVelocityID(FloatToEnumVelocity(anim->GetOutgoingVelocity()));
      if (anim_incomingVelocityID > std::max(currentVelocityID, anim_outgoingVelocityID)) rating_nonLinearVelocity -= penalty_nonLinearVelocity;
      if (anim_incomingVelocityID < std::min(currentVelocityID, anim_outgoingVelocityID)) rating_nonLinearVelocity -= penalty_nonLinearVelocity;
    }


    // --- OUTGOING MOVEMENT

    float rating_outgoingMovement = 0.0f;
    if (weight_outgoingMovement > 0.0f) {
      //Vector3 animOutgoingMovement = anim->GetRangedOutgoingMovement();
      int quadrantID = atoi(anim->GetVariable("quadrant_id").c_str());
      const Quadrant &quadrant = anims->GetQuadrant(quadrantID);
      Vector3 animOutgoingMovement = quadrant.position;
      rating_outgoingMovement = 1.0f - pow(NormalizedClamp((idealAnimDescription.outgoingMovementRel - animOutgoingMovement).GetLength(), 0.0f, sprintVelocity), 0.5f);

      // accel/decel anims are kinda chaotic so make them less likeable (HAX!)
      // if (FloatToEnumVelocity(anim->GetIncomingVelocity()) != FloatToEnumVelocity(anim->GetOutgoingVelocity())) rating_outgoingMovement *= 0.1f;
    }


    // --- INCOMING BODY DIRECTION

    float rating_incomingBodyDirection = 0.0f;
    float rating_nonLinearBodyDirection = 0.0f;
    if (weight_incomingBodyDirection > 0.0f) {
      rating_incomingBodyDirection = 1.0f - pow(fabs(ForceIntoAllowedBodyDirectionVec(anim->GetIncomingBodyDirection()).GetAngle2D(ForceIntoAllowedBodyDirectionVec(idealAnimDescription.incomingBodyDirectionRel))) / pi, 0.5f);
      if (FloatToEnumVelocity(anim->GetIncomingVelocity()) == e_Velocity_Idle) rating_incomingBodyDirection = 1.0f;

      bool nonLinearCondition = false;
      // if anim incoming body dir != between (including) query incoming and anim outgoing dir, then this anim may look annoying
      // how do we check this?
      // 1. if we look at the smallest angles between (anim incoming -> query incoming) and (anim incoming -> anim outgoing), one has to be positive, the other negative.
      // 2. the (absolute) angles added up have to be < pi radians. else, we could be on the 'other side' of the 'virtual half circle' and still have the former condition met
      radian marginRadians = 0.06f * pi; // anims can deviate a few degrees from the desired (quantized) directions
      Vector3 incomingBodyDirRel = anim->GetIncomingBodyDirection();
      Vector3 outgoingBodyDirRel = Vector3(0, -1, 0).GetRotated2D(anim->GetOutgoingBodyAngle() + anim->GetOutgoingAngle());
      radian shortestAngle1 = incomingBodyDirRel.GetAngle2D(outgoingBodyDirRel);
      radian shortestAngle2 = incomingBodyDirRel.GetAngle2D(idealAnimDescription.incomingBodyDirectionRel);
      if ((shortestAngle1 >  marginRadians && shortestAngle2 >  marginRadians) ||
          (shortestAngle1 < -marginRadians && shortestAngle2 < -marginRadians)) {
        nonLinearCondition = true;
      }
      if (nonLinearCondition) {
        if (fabs(shortestAngle1) + fabs(shortestAngle2) > pi + marginRadians) nonLinearCondition = true; else nonLinearCondition = false;
      }

      if (nonLinearCondition) rating_nonLinearBodyDirection -= penalty_nonLinearBodyDirection;
    }


    // -- OUTGOING BODY DIRECTION (LOOKAT)

    float rating_outgoingBodyDirection = 0.0f;
    Vector3 relDesiredBodyDirection; // possibly used later in rotational side weighting
    if (weight_outgoingBodyDirection > 0.0f) {
      float translationFactor = 1.0f;
      relDesiredBodyDirection = ((idealAnimDescription.desiredLookAtAbs - spatialState.position).Get2D().GetRotated2D(-spatialState.angle) - anim->GetTranslation() * translationFactor).GetNormalized(Vector3(0, -1, 0));
      //relDesiredBodyDirection = ForceIntoAllowedBodyDirectionVec(relDesiredBodyDirection);
      //rating_outgoingBodyDirection = 1.0f - pow(fabs(anim->GetOutgoingBodyDirection().GetRotated2D(anim->GetOutgoingAngle()).GetAngle2D(relDesiredBodyDirection)) / pi, 0.5f);
      rating_outgoingBodyDirection = 1.0f - pow(fabs(ForceIntoAllowedBodyDirectionVec(anim->GetOutgoingBodyDirection()).GetRotated2D(ForceIntoPreferredDirectionAngle(anim->GetOutgoingAngle())).GetAngle2D(relDesiredBodyDirection)) / pi, 0.5f);
    }


    // --- BASE ANIM?

    float rating_baseAnim = 0.0f;
    if (weight_baseAnim > 0.0f) {
      if (anim->GetVariable("baseanim").compare("true") == 0) rating_baseAnim = 1.0f;
    }


    // --- ROTATION SIDE

    float rating_side = 0.0f;
    if (weight_side > 0.0f && weight_outgoingBodyDirection > 0.0f) {
      // anim direction
      Vector3 animIncomingBodyDir = anim->GetIncomingBodyDirection();
      Vector3 animOutgoingBodyDir = anim->GetOutgoingDirection().GetRotated2D(anim->GetOutgoingBodyAngle());
      radian animRotationAngle = animOutgoingBodyDir.GetAngle2D(animIncomingBodyDir);
      // desired direction
      radian desiredRotationAngle = relDesiredBodyDirection.GetAngle2D(animIncomingBodyDir);

      //rating_side = 1.0f - (fabs(animRotationAngle - desiredRotationAngle) / (2.0f * pi));
      if (fabs(animRotationAngle) > 0.3f * pi && fabs(desiredRotationAngle) > 0.3f * pi && fabs(animRotationAngle - desiredRotationAngle) > 0.06f) {
        rating_side = (signSide(animRotationAngle) == signSide(desiredRotationAngle)) ? 1.0f : 0.0f;
      } else rating_side = 1.0f;
    }


    // weight the shit out of that shit

    float rating = rating_foot                  * weight_foot +
                   rating_incomingVelocity      * weight_incomingVelocity +
                   rating_outgoingMovement      * weight_outgoingMovement +
                   rating_incomingBodyDirection * weight_incomingBodyDirection +
                   rating_outgoingBodyDirection * weight_outgoingBodyDirection +
                   rating_baseAnim              * weight_baseAnim +
                   rating_side                  * weight_side +
                   rating_nonLinearVelocity +
                   rating_nonLinearBodyDirection;

    RatedAnim ratedAnim;
    ratedAnim.ID = *iter;
    ratedAnim.rating = rating;
    ratedDataSet.push_back(ratedAnim);

    iter++;
  }
}
*/

bool HumanoidBase::SelectAnim(const PlayerCommand &command, e_InterruptAnim localInterruptAnim, bool preferPassAndShot) { // returns false on no applicable anim found
  assert(command.desiredDirection.coords[2] == 0.0f);

  if (localInterruptAnim != e_InterruptAnim_ReQueue || currentAnim->frameNum > 12) CalculateFactualSpatialState();


  // CREATE A CRUDE SET OF POTENTIAL ANIMATIONS

  //if (player->GetDebug()) printf("time: %lu\n", match->GetActualTime_ms());
  CrudeSelectionQuery query;
  query.byFunctionType = true;
  query.functionType = command.desiredFunctionType;

  query.byFoot = false;
  query.foot = (spatialState.foot == e_Foot_Left) ? e_Foot_Right : e_Foot_Left;

  query.byIncomingVelocity = true;
  query.incomingVelocity = spatialState.enumVelocity;
  query.incomingVelocity_Strict = true;
  query.byIncomingBodyDirection = true;
  query.incomingBodyDirection_Strict = true;
  query.incomingBodyDirection = spatialState.relBodyDirectionVec;
  query.incomingVelocity_ForceLinearity = true;
  query.incomingBodyDirection_ForceLinearity = true;

  if (command.desiredFunctionType == e_FunctionType_Trip) {
    query.byTripType = true;
    query.tripType = command.tripType;
  }
  query.properties.Set("incoming_special_state", currentAnim->anim->GetVariable("outgoing_special_state"));
  if (match->GetBallRetainer() == player) query.properties.Set("incoming_retain_state", currentAnim->anim->GetVariable("outgoing_retain_state"));
  if (command.useSpecialVar1) query.properties.Set("specialvar1", command.specialVar1);
  if (command.useSpecialVar2) query.properties.Set("specialvar2", command.specialVar2);

  if (currentAnim->anim->GetVariable("outgoing_special_state") != "") query.incomingVelocity = e_Velocity_Idle; // standing up anims always start out idle

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);
  if (dataSet.size() == 0) {
    if (command.desiredFunctionType == e_FunctionType_Movement) {
      //if (player->GetDebug()) printf("selected player: ");
      if (Verbose()) printf("no movement animations to begin with (humanoidbase)\n");
      dataSet.push_back(GetIdleMovementAnimID()); // do with idle anim (should not happen too often, only after weird bumps when there's for example a need for a sprint anim at an impossible body angle, after a trip of whatever)
    } else return false;
  }

  //printf("dataset size after crude selection: %i\n", dataSet.size());


  // NOW SORT OUT THE RESULTING SET

  float adaptedDesiredVelocityFloat = command.desiredVelocityFloat;

  if (command.useDesiredMovement) {

    Vector3 relDesiredDirection = command.desiredDirection.GetRotated2D(-spatialState.angle);
    SetMovementSimilarityPredicate(relDesiredDirection, FloatToEnumVelocity(adaptedDesiredVelocityFloat));
    SetBodyDirectionSimilarityPredicate(command.desiredLookAt);
    //if (player->GetDebug()) printf("%f\n", adaptedDesiredVelocityFloat);

    if (command.desiredFunctionType == e_FunctionType_Movement) {
      _KeepBestDirectionAnims(dataSet, command);
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command);
    }

    else { // undefined animtype
      #ifdef dataSetSortable
      dataSet.sort(boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
      #else
      std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
      #endif
    }

  }

  int desiredIdleLevel = 1;
  SetNumericVariableSimilarityPredicate("idlelevel", desiredIdleLevel);
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&Humanoid::CompareNumericVariable, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareNumericVariable, this, _1, _2));
  #endif

  SetFootSimilarityPredicate(spatialState.foot);
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&HumanoidBase::CompareFootSimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareFootSimilarity, this, _1, _2));
  #endif

  SetIncomingBodyDirectionSimilarityPredicate(spatialState.relBodyDirectionVec);
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&HumanoidBase::CompareIncomingBodyDirectionSimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareIncomingBodyDirectionSimilarity, this, _1, _2));
  #endif

  SetIncomingVelocitySimilarityPredicate(spatialState.enumVelocity);
  #ifdef dataSetSortable
  dataSet.sort(boost::bind(&HumanoidBase::CompareIncomingVelocitySimilarity, this, _1, _2));
  #else
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareIncomingVelocitySimilarity, this, _1, _2));
  #endif

  if (command.useDesiredTripDirection) {
    Vector3 relDesiredTripDirection = command.desiredTripDirection.GetRotated2D(-spatialState.angle);
    SetTripDirectionSimilarityPredicate(relDesiredTripDirection);
    #ifdef dataSetSortable
    dataSet.sort(boost::bind(&HumanoidBase::CompareTripDirectionSimilarity, this, _1, _2));
    #else
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareTripDirectionSimilarity, this, _1, _2));
    #endif

    //if (player->GetDebug() && command.desiredFunctionType == e_FunctionType_Trip) SetRedDebugPilon(player->GetPosition() + command.desiredTripDirection * 5);
  }

  if (command.desiredFunctionType != e_FunctionType_Movement) {
    #ifdef dataSetSortable
    dataSet.sort(boost::bind(&HumanoidBase::CompareBaseanimSimilarity, this, _1, _2));
    #else
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareBaseanimSimilarity, this, _1, _2));
    #endif
  }


  // process result

  int selectedAnimID = -1;
  std::vector<Vector3> positions_tmp;
  int touchFrame_tmp = -1;
  float radiusOffset_tmp = 0.0f;
  Vector3 touchPos_tmp;
  Vector3 fullActionSmuggle_tmp;
  Vector3 actionSmuggle_tmp;
  radian rotationSmuggle_tmp = 0;

  if (dataSet.size() == 0) {
    //if (Verbose()) if (player->GetDebug()) printf("selected player: ");
    if (Verbose()) printf("no animations left\n");
    return false;
  }
  //if (player->GetDebug()) printf("chosen: %s\n", anims->GetAnim(ratedDataSet.begin()->ID)->GetName().c_str());

  if (command.desiredFunctionType == e_FunctionType_Movement ||
      command.desiredFunctionType == e_FunctionType_Trip ||
      command.desiredFunctionType == e_FunctionType_Special) {
    //if (player->GetDebug()) printf("chosen: %s\n", anims->GetAnim(ratedDataSet.begin()->ID)->GetName().c_str());
    selectedAnimID = *dataSet.begin();
    Animation *nextAnim = anims->GetAnim(selectedAnimID);
    Vector3 desiredMovement = command.desiredDirection * command.desiredVelocityFloat;
    if (command.desiredDirection.coords[2] != 0.0f) {
      command.desiredDirection.Print();
      printf("functiontype %i\n", command.desiredFunctionType);
    }
    assert(desiredMovement.coords[2] == 0.0f);
    Vector3 desiredBodyDirectionRel = Vector3(0, -1, 0);
    if (command.useDesiredLookAt) desiredBodyDirectionRel = ((command.desiredLookAt - spatialState.position).Get2D().GetRotated2D(-spatialState.angle) - nextAnim->GetTranslation()).GetNormalized(Vector3(0, -1, 0));
    Vector3 physicsVector = CalculatePhysicsVector(nextAnim, command.useDesiredMovement, desiredMovement, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, rotationSmuggle_tmp);
  }


  // check if we really want to requeue - only requeue movement to movement, for example, when we want to go a different direction

  if (localInterruptAnim == e_InterruptAnim_ReQueue && selectedAnimID != -1 && currentAnim->positions.size() > 1 && positions_tmp.size() > 1) {

    // don't requeue to same quadrant
    if (currentAnim->functionType == command.desiredFunctionType &&

        ((FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) != e_Velocity_Idle &&
          currentAnim->anim->GetVariable("quadrant_id").compare(anims->GetAnim(selectedAnimID)->GetVariable("quadrant_id")) == 0)
          ||
         (FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) == e_Velocity_Idle &&
          fabs((ForceIntoPreferredDirectionAngle(currentAnim->anim->GetOutgoingAngle()) - ForceIntoPreferredDirectionAngle(anims->GetAnim(selectedAnimID)->GetOutgoingAngle()))) < 0.20f * pi))
       ) {

      selectedAnimID = -1;
      if (player->GetDebug()) printf("rejecting requeue anim for leading into the same quadrant\n");

    }
  }


  // make it so

  if (selectedAnimID != -1) {

    /*
    if (command.desiredFunctionType != e_FunctionType_Movement) {
      if (player->GetDebug()) {
        SetGreenDebugPilon(Vector3(0, 0, -1));
        SetYellowDebugPilon(Vector3(0, 0, -1));
        SetRedDebugPilon(Vector3(0, 0, -1));
      }
    }
    */

    *previousAnim = *currentAnim;

    currentAnim->anim = anims->GetAnim(selectedAnimID);
    currentAnim->id = selectedAnimID;
    currentAnim->functionType = command.desiredFunctionType;
    currentAnim->frameNum = 0;
    currentAnim->touchFrame = touchFrame_tmp;
    currentAnim->originatingInterrupt = localInterruptAnim;
    currentAnim->radiusOffset = radiusOffset_tmp;
    currentAnim->touchPos = touchPos_tmp;
    currentAnim->rotationSmuggle.begin = clamp(ModulateIntoRange(-pi, pi, spatialState.relBodyAngleNonquantized - currentAnim->anim->GetIncomingBodyAngle()) * bodyRotationSmoothingFactor, -bodyRotationSmoothingMaxAngle, bodyRotationSmoothingMaxAngle);
    currentAnim->rotationSmuggle.end = rotationSmuggle_tmp;
    currentAnim->rotationSmuggleOffset = 0;
    currentAnim->fullActionSmuggle = fullActionSmuggle_tmp;
    currentAnim->actionSmuggle = actionSmuggle_tmp;
    currentAnim->actionSmuggleOffset = Vector3(0);
    currentAnim->actionSmuggleSustain = Vector3(0);
    currentAnim->actionSmuggleSustainOffset = Vector3(0);
    currentAnim->movementSmuggle = Vector3(0);
    currentAnim->movementSmuggleOffset = Vector3(0);
    currentAnim->incomingMovement = spatialState.movement;
    currentAnim->outgoingMovement = CalculateOutgoingMovement(positions_tmp);
    currentAnim->positions.clear();
    currentAnim->positions.assign(positions_tmp.begin(), positions_tmp.end());
    currentAnim->positionOffset = 0.0;
    currentAnim->originatingCommand = command;

    return true;
  }

  return false;
}

void HumanoidBase::CalculatePredictedSituation(Vector3 &predictedPos, radian &predictedAngle) {

  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    assert(currentAnim->positions.size() > (unsigned int)currentAnim->anim->GetEffectiveFrameCount());
    predictedPos = spatialState.position + currentAnim->positions.at(currentAnim->anim->GetEffectiveFrameCount()) + currentAnim->actionSmuggle + currentAnim->actionSmuggleSustain + currentAnim->movementSmuggle;
  } else {
    predictedPos = spatialState.position + currentAnim->anim->GetTranslation().Get2D().GetRotated2D(spatialState.angle) + currentAnim->actionSmuggle + currentAnim->actionSmuggleSustain + currentAnim->movementSmuggle;
  }

  predictedAngle = spatialState.angle + currentAnim->anim->GetOutgoingAngle() + currentAnim->rotationSmuggle.end;
  predictedAngle = ModulateIntoRange(-pi, pi, predictedAngle);
  assert(predictedPos.coords[2] == 0.0f);
}

Vector3 HumanoidBase::CalculateOutgoingMovement(const std::vector<Vector3> &positions) const {
  if (positions.size() < 2) return 0;
  return (positions.at(positions.size() - 1) - positions.at(positions.size() - 2)) * 100.0f;
}

void HumanoidBase::CalculateSpatialState() {
  Vector3 position;
  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    position = startPos + currentAnim->positions.at(currentAnim->frameNum) + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
  } else {
    Quaternion orientation;
    currentAnim->anim->GetKeyFrame("player", currentAnim->frameNum, orientation, position);
    position.coords[2] = 0.0f;
    position = startPos + position.GetRotated2D(startAngle) + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
  }

  if (currentAnim->frameNum > 12) {
    spatialState.foot = currentAnim->anim->GetOutgoingFoot();
  }

  assert(startPos.coords[2] == 0.0f);
  assert(currentAnim->actionSmuggleOffset.coords[2] == 0.0f);
  assert(currentAnim->movementSmuggleOffset.coords[2] == 0.0f);
  assert(position.coords[2] == 0.0f);

  spatialState.actualMovement = (position - previousPosition2D) * 100.0f;
  float positionOffsetMovementIgnoreFactor = 0.5f;
  spatialState.physicsMovement = spatialState.actualMovement - (spatialState.actionSmuggleMovement) - (spatialState.movementSmuggleMovement) - (spatialState.positionOffsetMovement * positionOffsetMovementIgnoreFactor);
  spatialState.animMovement = spatialState.physicsMovement;
  if (currentAnim->positions.size() > 0) {
    // this way, action cheating is being omitted from the current movement, making for better requeues. however, keep in mind that
    // movementoffsets, from bumping into other players, for example, will also be ignored this way.
    const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(currentAnim->anim);
    spatialState.animMovement = CalculateMovementAtFrame(origPositionCache, currentAnim->frameNum, 1).GetRotated2D(startAngle);
  }
  //if (player->GetDebug()) SetRedDebugPilon(position + spatialState.physicsMovement);
  //if (player->GetDebug()) SetGreenDebugPilon(position + spatialState.animMovement);

  spatialState.movement = spatialState.physicsMovement; // PICK DEFAULT

  //if (player->GetDebug()) SetRedDebugPilon(spatialState.actualMovement);


  Vector3 bodyPosition;
  Quaternion bodyOrientation;
  currentAnim->anim->GetKeyFrame("body", currentAnim->frameNum, bodyOrientation, bodyPosition);
  radian x, y, z;
  bodyOrientation.GetAngles(x, y, z);

  Vector3 quatDirection; quatDirection = bodyOrientation;

  Vector3 bodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(z + startAngle + currentAnim->rotationSmuggleOffset);

  spatialState.floatVelocity = spatialState.movement.GetLength();
  spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);

  if (spatialState.enumVelocity != e_Velocity_Idle) {
    spatialState.directionVec = spatialState.movement.GetNormalized();
  } else {
    // too slow for comfort, use body direction as global direction
    spatialState.directionVec = bodyDirectionVec;
  }

  spatialState.position = position;
  spatialState.angle = ModulateIntoRange(-pi, pi, FixAngle(spatialState.directionVec.GetAngle2D()));

  if (spatialState.enumVelocity != e_Velocity_Idle) {
    Vector3 adaptedBodyDirectionVec = bodyDirectionVec.GetRotated2D(-spatialState.angle);
    // prefer straight forward, so lie about the actual direction a bit
    // this may fix bugs of body dir being non-0 somewhere during 0 anims
    // but it may also cause other bugs (going 0 to 45 all over again each time)
    //adaptedBodyDirectionVec = (adaptedBodyDirectionVec * 0.95f + Vector3(0, -1, 0) * 0.05f).GetNormalized(0);
    //ForceIntoAllowedBodyDirectionVec(Vector3(0, -1, 0)).Print();

    bool preferCorrectVeloOverCorrectAngle = true; // todo: for false, should also alter .movement, right?
    radian bodyAngleRel = adaptedBodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));
    if (spatialState.enumVelocity == e_Velocity_Sprint && fabs(bodyAngleRel) >= 0.125f * pi) {
      if (preferCorrectVeloOverCorrectAngle) {
        // on impossible combinations of velocity and body angle, decrease body angle
        adaptedBodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(0.12f * pi * signSide(bodyAngleRel));
      } else {
        // on impossible combinations of velocity and body angle, decrease velocity
        spatialState.floatVelocity = walkSprintSwitch - 0.1f;
        spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);
      }
    }
    else if (spatialState.enumVelocity == e_Velocity_Walk && fabs(bodyAngleRel) >= 0.5f * pi) {
      if (preferCorrectVeloOverCorrectAngle) {
        // on impossible combinations of velocity and body angle, decrease body angle
        adaptedBodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(0.495f * pi * signSide(bodyAngleRel));
      } else {
        // on impossible combinations of velocity and body angle, decrease velocity
        spatialState.floatVelocity = dribbleWalkSwitch - 0.1f;
        spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);
      }
    }

    spatialState.relBodyDirectionVecNonquantized = adaptedBodyDirectionVec;
    spatialState.relBodyDirectionVec = ForceIntoAllowedBodyDirectionVec(adaptedBodyDirectionVec);
  } else {
    spatialState.relBodyDirectionVecNonquantized = Vector3(0, -1, 0);
    spatialState.relBodyDirectionVec = Vector3(0, -1, 0);
  }
  spatialState.relBodyAngle = spatialState.relBodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));
  spatialState.relBodyAngleNonquantized = spatialState.relBodyDirectionVecNonquantized.GetAngle2D(Vector3(0, -1, 0));
  spatialState.bodyDirectionVec = spatialState.relBodyDirectionVec.GetRotated2D(spatialState.angle); // rotate back, we now have it forced into allowed angle
  spatialState.bodyAngle = spatialState.bodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));

  previousPosition2D = position;
}

void HumanoidBase::CalculateFactualSpatialState() {

  spatialState.foot = currentAnim->anim->GetOutgoingFoot();

  if (currentAnim->anim->GetVariable("outgoing_special_state").compare("") != 0) {
    spatialState.floatVelocity = 0;
    spatialState.enumVelocity = e_Velocity_Idle;
    spatialState.movement = Vector3(0);
  }
}

void HumanoidBase::AddTripCommandToQueue(PlayerCommandQueue &commandQueue, const Vector3 &tripVector, int tripType) {
  if (tripType == 1) {
    commandQueue.push_back(GetTripCommand(tripDirection, tripType));
  } else {
    // allow both types 2 and 3, but prefer the right one
    int otherTripType = 3;
    if (tripType == 3) otherTripType = 2;
    commandQueue.push_back(GetTripCommand(tripDirection, tripType));
    commandQueue.push_back(GetTripCommand(tripDirection, otherTripType));
    commandQueue.push_back(GetTripCommand(tripDirection, 1));
  }
}

PlayerCommand HumanoidBase::GetTripCommand(const Vector3 &tripVector, int tripType) {
  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Trip;
  command.useDesiredMovement = false;
  command.useDesiredTripDirection = true;
  command.desiredTripDirection = tripVector;
  command.desiredVelocityFloat = spatialState.floatVelocity;//e_Velocity_Sprint;
  command.useTripType = true;
  command.tripType = tripType;
  return command;
}

PlayerCommand HumanoidBase::GetBasicMovementCommand(const Vector3 &desiredDirection, float velocityFloat) {
  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Movement;
  command.useDesiredMovement = true;
  command.useDesiredLookAt = true;
  command.desiredDirection = spatialState.directionVec;
  command.desiredVelocityFloat = velocityFloat;
  command.desiredLookAt = spatialState.position + command.desiredDirection * 10.0f;
  return command;
}

void HumanoidBase::SetFootSimilarityPredicate(e_Foot desiredFoot) const {
  predicate_DesiredFoot = desiredFoot;
}

bool HumanoidBase::CompareFootSimilarity(int animIndex1, int animIndex2) const {
  int one = 1;
  int two = 1;
  if (anims->GetAnim(animIndex1)->GetCurrentFoot() == predicate_DesiredFoot) one = 0;
  if (anims->GetAnim(animIndex2)->GetCurrentFoot() == predicate_DesiredFoot) two = 0;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()) == e_Velocity_Idle) one = 0;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()) == e_Velocity_Idle) two = 0;
  return one < two;
}

void HumanoidBase::SetIncomingVelocitySimilarityPredicate(e_Velocity velocity) const {
  predicate_IncomingVelocity = velocity;
}

bool HumanoidBase::CompareIncomingVelocitySimilarity(int animIndex1, int animIndex2) const {
  /* old version
  float rating1 = fabs(clamp(RangeVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()) - EnumToFloatVelocity(predicate_IncomingVelocity), -sprintVelocity, sprintVelocity));
  float rating2 = fabs(clamp(RangeVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()) - EnumToFloatVelocity(predicate_IncomingVelocity), -sprintVelocity, sprintVelocity));
  */

  int currentVelocityID = GetVelocityID(predicate_IncomingVelocity);

  // rate difference anim incoming / actual incoming
  int anim1_incomingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()));
  int anim2_incomingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()));
  float rating1 = fabs(clamp(anim1_incomingVelocityID - currentVelocityID, -3, 3));
  float rating2 = fabs(clamp(anim2_incomingVelocityID - currentVelocityID, -3, 3));

  // also add a penalty for anim incoming velocities which aren't between actual incoming and anim outgoing
  int anim1_outgoingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetOutgoingVelocity()));
  int anim2_outgoingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetOutgoingVelocity()));
  if (anim1_incomingVelocityID > std::max(currentVelocityID, anim1_outgoingVelocityID)) rating1 += 0.5f;
  if (anim1_incomingVelocityID < std::min(currentVelocityID, anim1_outgoingVelocityID)) rating1 += 0.5f;
  if (anim2_incomingVelocityID > std::max(currentVelocityID, anim2_outgoingVelocityID)) rating2 += 0.5f;
  if (anim2_incomingVelocityID < std::min(currentVelocityID, anim2_outgoingVelocityID)) rating2 += 0.5f;

  return rating1 < rating2;
}

void HumanoidBase::SetMovementSimilarityPredicate(const Vector3 &relDesiredDirection, e_Velocity desiredVelocity) const {
  predicate_RelDesiredDirection = relDesiredDirection;
  predicate_DesiredVelocity = desiredVelocity;
  //if (relDesiredDirection.GetDotProduct(Vector3(0, -1, 0)) < 0) predicate_DesiredVelocity = e_Velocity_Idle;
  // this isn't working all too well: if targetmovement is set to 0 (aka corneringbias towards 1), both dribble @ 0 deg and dribble @ 90 deg will be the same distance (from 0), so still no preference for braking straight
  predicate_CorneringBias = CalculateBiasForFastCornering(Vector3(0, -1.0f * spatialState.floatVelocity, 0), relDesiredDirection * EnumToFloatVelocity(desiredVelocity), 1.0f, 0.9f); // anim space values!
}

float HumanoidBase::GetMovementSimilarity(int animIndex, const Vector3 &relDesiredDirection, e_Velocity desiredVelocity, float corneringBias) const {

  Vector3 desiredMovement = relDesiredDirection * EnumToFloatVelocity(desiredVelocity);

  Vector3 outgoingDirection = ForceIntoPreferredDirectionVec(anims->GetAnim(animIndex)->GetOutgoingDirection());
  float outgoingVelocity = RangeVelocity(anims->GetAnim(animIndex)->GetOutgoingVelocity());
  Vector3 outgoingMovement = outgoingDirection * outgoingVelocity;


  // anims that end at lower velocities have an advantage: they don't get dragged into the currentmovement that much
  // thus: have a bias that is higher at higher outgoing velocities, which means anim outgoingmovement gets more % of current movement and less % of their own
  // disabled for now - the new physics system disregards most of the anims movement anyway :)
  // *enabled again: altered the physics system so that it will regard anim movement more, so this became useful again for proper hard cornering

  // alternative to this system (drags back movement)
  //outgoingMovement += -desiredMovement.GetNormalized(0) * outgoingMovement.GetLength() * 0.5f;
  //desiredMovement = desiredMovement.GetNormalized(0) * clamp(desiredMovement.GetLength() - 0.8f, idleVelocity, sprintVelocity);

  desiredMovement = desiredMovement * (1.0f - corneringBias);

  float value = (desiredMovement - outgoingMovement).GetLength();

  value -= fabs(relDesiredDirection.GetDotProduct(outgoingDirection)) * 4.0f; // prefer straight lines (towards/away from desired outgoing)

/* disabled quantization:
  // maximum quantization (20 degree angle @ dribble velocity, smallest distance to be measured)
  // faulty calculation (cornering distance instead of straight line) for optimization, makes little difference anyway for 20 degrees
  float maxQuant = (dribbleVelocity * 2.0f * pi) / (360.0f / 20.0f);// * (1.0 - velocityBias * 0.5) // == ~1.22 at the moment of writing
  //float maxQuant = sqrt(2.0f * pow(dribbleVelocity, 2.0f) * (1.0f - cos(pi / 180.0f * 20.0f))); // the actual correct calculation (straight line)

  // add safety
  maxQuant *= 0.8f;

  // quantize
  value /= maxQuant;
  value = round(value);
  value *= maxQuant;
*/

  // never turn the wrong way around ** BUGGY, causes weird acceleration when we want idle velo **
  //if (fabs(outgoingDirection.GetAngle2D() - relDesiredDirection.GetAngle2D()) > pi) value += 100000;

  return value;
}

bool HumanoidBase::CompareMovementSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = GetMovementSimilarity(animIndex1, predicate_RelDesiredDirection, predicate_DesiredVelocity, predicate_CorneringBias);
  float rating2 = GetMovementSimilarity(animIndex2, predicate_RelDesiredDirection, predicate_DesiredVelocity, predicate_CorneringBias);
  return rating1 < rating2;
}

bool HumanoidBase::CompareDirectionSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = fabs(predicate_RelDesiredDirection.GetAngle2D(anims->GetAnim(animIndex1)->GetOutgoingDirection()));
  float rating2 = fabs(predicate_RelDesiredDirection.GetAngle2D(anims->GetAnim(animIndex2)->GetOutgoingDirection()));
  return rating1 < rating2;
}

bool HumanoidBase::CompareOutgoingVelocitySimilarity(int animIndex1, int animIndex2) const {
  float rating1 = fabs(clamp(RangeVelocity(anims->GetAnim(animIndex1)->GetOutgoingVelocity()) - EnumToFloatVelocity(predicate_DesiredVelocity), -sprintVelocity, sprintVelocity));
  float rating2 = fabs(clamp(RangeVelocity(anims->GetAnim(animIndex2)->GetOutgoingVelocity()) - EnumToFloatVelocity(predicate_DesiredVelocity), -sprintVelocity, sprintVelocity));
  return rating1 < rating2;
}

void HumanoidBase::SetIncomingBodyDirectionSimilarityPredicate(const Vector3 &relIncomingBodyDirection) const {
  predicate_RelIncomingBodyDirection = relIncomingBodyDirection;
}

bool HumanoidBase::CompareIncomingBodyDirectionSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = fabs(ForceIntoAllowedBodyDirectionVec(anims->GetAnim(animIndex1)->GetIncomingBodyDirection()).GetAngle2D(ForceIntoAllowedBodyDirectionVec(predicate_RelIncomingBodyDirection))) / pi;
  float rating2 = fabs(ForceIntoAllowedBodyDirectionVec(anims->GetAnim(animIndex2)->GetIncomingBodyDirection()).GetAngle2D(ForceIntoAllowedBodyDirectionVec(predicate_RelIncomingBodyDirection))) / pi;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()) == e_Velocity_Idle) rating1 = 0;//-1;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()) == e_Velocity_Idle) rating2 = 0;//-1;

  return rating1 < rating2;
}

void HumanoidBase::SetBodyDirectionSimilarityPredicate(const Vector3 &lookAt) const {
  predicate_LookAt = lookAt;
}

bool HumanoidBase::CompareBodyDirectionSimilarity(int animIndex1, int animIndex2) const {

  Animation *a1 = anims->GetAnim(animIndex1);
  Animation *a2 = anims->GetAnim(animIndex2);

  float translationFactor = 1.0f;
  Vector3 relDesiredBodyDirection1 = ((predicate_LookAt - spatialState.position).GetRotated2D(-spatialState.angle) - a1->GetTranslation() * translationFactor).GetNormalized(Vector3(0, -1, 0));
  Vector3 relDesiredBodyDirection2 = ((predicate_LookAt - spatialState.position).GetRotated2D(-spatialState.angle) - a2->GetTranslation() * translationFactor).GetNormalized(Vector3(0, -1, 0));

  /* this is quirky somehow; when accelerating, sometimes the humanoid will actually go 135 when it shouldn't. could debug it, but then again, maybe KISS is a better idea;
     this whole setup seems a bit overly complicated. probably should look for solution elsewhere

  // prefer 'lower angled' anim (so from 0 to 45, use 0, from 45 to 135, use 45)
  // doesn't work when ending idle
    if (anims->GetAnim(animIndex1)->GetAnimType().compare("movement") == 0) {
    if (anims->GetAnim(animIndex1)->GetOutgoingVelocity() > idleDribbleSwitch &&
        anims->GetAnim(animIndex2)->GetOutgoingVelocity() > idleDribbleSwitch) {

      float origAngleBias = 0.5f;

      radian a1 = relDesiredBodyDirection1.GetAngle2D(Vector3(0, -1, 0));
      radian d1 = 0;
      if (fabs(a1) > 0.75f * pi) d1 = 0.75f * pi * signSide(a1);
      else if (fabs(a1) > 0.25f * pi) d1 = 0.25f * pi * signSide(a1);
      relDesiredBodyDirection1 = Vector3(0, -1, 0).GetRotated2D(d1 * (1.0f - origAngleBias) + a1 * origAngleBias);

      radian a2 = relDesiredBodyDirection2.GetAngle2D(Vector3(0, -1, 0));
      radian d2 = 0;
      if (fabs(a2) > 0.75f * pi) d2 = 0.75f * pi * signSide(a2);
      else if (fabs(a2) > 0.25f * pi) d2 = 0.25f * pi * signSide(a2);
      relDesiredBodyDirection2 = Vector3(0, -1, 0).GetRotated2D(d2 * (1.0f - origAngleBias) + a2 * origAngleBias);
    }
  }
  */

  // this version corrects for rotation smuggle; we will probably end up walking in a direction a bit rotated towards the user desired direction, instead of pure anim direction.
  // we need some heuristic to use that fact, because else, we may end up looking in the wrong direction somewhat
  // radian outgoingAngle1 = FixAngle(((a1->GetOutgoingDirection() + predicate_RelDesiredDirection * 0.2f).GetNormalized()).GetAngle2D());
  // radian outgoingAngle2 = FixAngle(((a2->GetOutgoingDirection() + predicate_RelDesiredDirection * 0.2f).GetNormalized()).GetAngle2D());
  radian maxAngleSmuggle = 0.1f * pi; // mind you, for anims ending idle, this shouldn't be >= 0.125f * pi, because then we end up allowing multiple outgoing angles.
                                      // after all, outgoing direction will be skewed by this value as max, which will make outgoing body direction follow in the same dir
                                      // but since idle anims don't have outgoing body directions, it will just change their outgoing directions and they will be pointing in exactly the same direction
  radian outgoingAngle1 = a1->GetOutgoingDirection().GetRotated2D( clamp(predicate_RelDesiredDirection.GetAngle2D(a1->GetOutgoingDirection()), -maxAngleSmuggle, maxAngleSmuggle) ).GetAngle2D(Vector3(0, -1, 0));
  radian outgoingAngle2 = a2->GetOutgoingDirection().GetRotated2D( clamp(predicate_RelDesiredDirection.GetAngle2D(a2->GetOutgoingDirection()), -maxAngleSmuggle, maxAngleSmuggle) ).GetAngle2D(Vector3(0, -1, 0));
  Vector3 predictedOutgoingBodyDirection1 = a1->GetOutgoingBodyDirection().GetRotated2D(outgoingAngle1);
  Vector3 predictedOutgoingBodyDirection2 = a2->GetOutgoingBodyDirection().GetRotated2D(outgoingAngle2);
  radian rating1 = fabs(predictedOutgoingBodyDirection1.GetAngle2D(relDesiredBodyDirection1));
  radian rating2 = fabs(predictedOutgoingBodyDirection2.GetAngle2D(relDesiredBodyDirection2));

  // penalty for body angles (as opposed to straight forward), to get a slight preference for forward angles
  rating1 += fabs(a1->GetOutgoingBodyAngle()) * 0.05f;
  rating2 += fabs(a2->GetOutgoingBodyAngle()) * 0.05f;

  // quantize
  //rating1 = round(rating1 * 4.0);
  //rating2 = round(rating2 * 4.0);
  // rating1 = round((rating1 / pi) * 4.0);
  // rating2 = round((rating2 / pi) * 4.0);

  return rating1 < rating2;
}

void HumanoidBase::SetTripDirectionSimilarityPredicate(const Vector3 &relDesiredTripDirection) const {
  predicate_RelDesiredTripDirection = relDesiredTripDirection;
}

bool HumanoidBase::CompareTripDirectionSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = -GetVectorFromString(anims->GetAnim(animIndex1)->GetVariable("bumpdirection")).GetDotProduct(predicate_RelDesiredTripDirection);
  float rating2 = -GetVectorFromString(anims->GetAnim(animIndex2)->GetVariable("bumpdirection")).GetDotProduct(predicate_RelDesiredTripDirection);
  return rating1 < rating2;
}

void HumanoidBase::SetBallDirectionSimilarityPredicate(const Vector3 &relDesiredBallDirection) const {
  predicate_RelDesiredBallDirection = relDesiredBallDirection;
}

bool HumanoidBase::CompareBallDirectionSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = -GetVectorFromString(anims->GetAnim(animIndex1)->GetVariable("balldirection")).Get2D().GetDotProduct(predicate_RelDesiredBallDirection);
  float rating2 = -GetVectorFromString(anims->GetAnim(animIndex2)->GetVariable("balldirection")).Get2D().GetDotProduct(predicate_RelDesiredBallDirection);
  return rating1 < rating2;
}

bool HumanoidBase::CompareBaseanimSimilarity(int animIndex1, int animIndex2) const {
  bool isBase1 = (anims->GetAnim(animIndex1)->GetVariable("baseanim").compare("true") == 0);
  bool isBase2 = (anims->GetAnim(animIndex2)->GetVariable("baseanim").compare("true") == 0);

  if (isBase1 == true && isBase2 == false) return true;
  return false;
}

bool HumanoidBase::CompareCatchOrDeflect(int animIndex1, int animIndex2) const {
  bool catch1 = (anims->GetAnim(animIndex1)->GetVariable("outgoing_retain_state").compare("") != 0);
  bool catch2 = (anims->GetAnim(animIndex2)->GetVariable("outgoing_retain_state").compare("") != 0);

  if (catch1 == true && catch2 == false) return true;
  return false;
}

void HumanoidBase::SetNumericVariableSimilarityPredicate(const std::string &varName, float desiredValue) const {
  predicate_NumericVariableName = varName;
  predicate_NumericVariableValue = desiredValue;
}

bool HumanoidBase::CompareNumericVariable(int animIndex1, int animIndex2) const {
  return fabs(atof(anims->GetAnim(animIndex1)->GetVariable(predicate_NumericVariableName.c_str()).c_str()) - predicate_NumericVariableValue) <
         fabs(atof(anims->GetAnim(animIndex2)->GetVariable(predicate_NumericVariableName.c_str()).c_str()) - predicate_NumericVariableValue);
}

Vector3 HumanoidBase::CalculatePhysicsVector(Animation *anim, bool useDesiredMovement, const Vector3 &desiredMovement, bool useDesiredBodyDirection, const Vector3 &desiredBodyDirectionRel, std::vector<Vector3> &positions_ret, radian &rotationOffset_ret) const {

  positions_ret.clear();

  int animTouchFrame = atoi(anim->GetVariable("touchframe").c_str());
  bool touch = (animTouchFrame > 0);

  float stat_agility = player->GetStat("physical_agility");
  float stat_acceleration = player->GetStat("physical_acceleration");
  float stat_velocity = player->GetStat("physical_velocity");
  float stat_dribble = player->GetStat("technical_dribble");

  float incomingSwitchBias = 0.0f; // anything other than 0.0 may result in unpuristic behavior
  float outgoingSwitchBias = 0.0f;

  const std::string &animType = anim->GetAnimType();

  if (animType.compare("ballcontrol") == 0) {
    outgoingSwitchBias = 0.0f;
  } else if (animType.compare("trap") == 0) {
    outgoingSwitchBias = 0.0f;
  } else if (animType.compare("interfere") == 0) {
    outgoingSwitchBias = 0.0f;
  } else if (animType.compare("deflect") == 0) {
    outgoingSwitchBias = 1.0f;
  } else if (animType.compare("sliding") == 0) {
    outgoingSwitchBias = 0.0f;
  } else if (animType.compare("special") == 0) {
    outgoingSwitchBias = 1.0f;
  } else if (animType.compare("trip") == 0) {
    outgoingSwitchBias = 0.5f; // direction partly predecided by collision function in match class
  } else if (touch) {
    outgoingSwitchBias = 1.0f;
  }

  if (anim->GetVariable("incoming_special_state").compare("") != 0 ||
      anim->GetVariable("outgoing_special_state").compare("") != 0) outgoingSwitchBias = 1.0f;

  Vector3 animIncomingMovement = Vector3(0, -1, 0).GetRotated2D(spatialState.angle) * RangeVelocity(anim->GetIncomingVelocity());
  Vector3 adaptedCurrentMovement = animIncomingMovement * incomingSwitchBias + spatialState.movement * (1.0f - incomingSwitchBias);

  Vector3 predictedOutgoingMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle);
  Vector3 velocifiedDesiredMovement = (useDesiredMovement) ? desiredMovement : predictedOutgoingMovement;

  assert(desiredMovement.coords[2] == 0.0f);

  Vector3 adaptedDesiredMovement = predictedOutgoingMovement * outgoingSwitchBias + velocifiedDesiredMovement * (1.0f - outgoingSwitchBias);
  assert(predictedOutgoingMovement.coords[2] == 0.0f);
  assert(velocifiedDesiredMovement.coords[2] == 0.0f);
  assert(adaptedDesiredMovement.coords[2] == 0.0f);

  float maxVelocity = player->GetMaxVelocity();
  if (touch) maxVelocity *= 0.92f;

  Vector3 resultingMovement;

  const int timeStep_ms = 10;

  bool isBaseAnim = (anim->GetVariable("baseanim").compare("true") == 0);

  float difficultyFactor = atof(anim->GetVariable("animdifficultyfactor").c_str());
  float difficultyPenaltyFactor = std::pow(
      clamp((difficultyFactor - 0.0f) *
                (1.0f - (stat_agility * 0.2f + stat_acceleration * 0.2f)) *
                2.0f,
            0.0f, 1.0f),
      0.7f);

  float powerFactor = 1.0f - clamp(std::pow(player->GetLastTouchBias(1000), 0.8f) * (0.8f - stat_dribble * 0.3f), 0.0f, 0.4f); // todo: put lasttouchbias thing in loop, so it'll change over time (in that loop)
  // moved to per ms timeloop penalty
  powerFactor *= 1.0f - clamp(decayingPositionOffset.GetLength() * (10.0f - player->GetStat("physical_balance") * 5.0f) - 0.1f, 0.0f, 0.3f);

  //if (player->GetDebug()) printf("physicsvector factors: %f, %f\n", difficultyPenaltyFactor, powerFactor);

  Vector3 temporalMovement = adaptedCurrentMovement;

  assert(adaptedCurrentMovement.coords[2] == 0.0f);
  assert(adaptedDesiredMovement.coords[2] == 0.0f);


  // orig anim positions

  const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(anim);

  Vector3 currentPosition;

  // amount of pure physics that 'shines through' pure anim
  float physicsBias = 1.0f;
  // angle deviation away from anim
  radian maxAngleMod_underAnimAngle = 0.125f * pi;
  radian maxAngleMod_overAnimAngle = 0.125f * pi;
  radian maxAngleMod_straightAnimAngle = 0.125f * pi;
  if (touch) {
    float bonus = 1.0f - std::pow(NormalizedClamp((adaptedCurrentMovement + predictedOutgoingMovement).GetLength() * 0.5f, 0, sprintVelocity), 0.8f) * 0.8f;
    bonus *= 0.6f + 0.4f * player->GetStat("technical_ballcontrol"); // todo: shouldn't this be agility?
    maxAngleMod_underAnimAngle = 0.2f * pi * bonus;
    maxAngleMod_overAnimAngle = 0;
    maxAngleMod_straightAnimAngle = 0.1f * pi * bonus;
  }
  if (animType.compare("sliding") == 0) {
    maxAngleMod_underAnimAngle = 0.5f * pi;
    maxAngleMod_overAnimAngle = 0.5f * pi;
    maxAngleMod_straightAnimAngle = 0.5f * pi;
  }

  if (animType.compare("movement") == 0)    { physicsBias *= 1.0f; }

  if (animType.compare("ballcontrol") == 0) { physicsBias *= 1.0f; }
  if (animType.compare("trap") == 0)        { physicsBias *= 1.0f; }

  if (animType.compare("shortpass") == 0)   { physicsBias *= 0.0f; }
  if (animType.compare("highpass") == 0)    { physicsBias *= 0.0f; }
  if (animType.compare("shot") == 0)        { physicsBias *= 0.0f; }

  if (animType.compare("interfere") == 0)   { physicsBias *= 0.5f; }
  if (animType.compare("deflect") == 0)     { physicsBias *= 0.0f; }

  if (animType.compare("sliding") == 0)     { physicsBias *= 1.0f; }
  if (animType.compare("trip") == 0)        { if (anim->GetVariable("triptype").compare("1") == 0) physicsBias *= 0.5f; else physicsBias *= 0.0f; }

  if (animType.compare("special") == 0)     { physicsBias *= 0.0f; }
  if (anim->GetVariable("incoming_special_state").compare("") != 0)
                                            { physicsBias *= 0.0f; }

  bool mod_AllowRotation = true;
  bool mod_CorneringBraking = false;
  bool mod_PointinessCurve = true;
  bool mod_MaximumAccelDecel = false;
  bool mod_BrakeOnTouch = false; // may be too pointy for anims near 90 degree
  bool mod_MaxCornering = true;
  bool mod_MaxChange = true;
  bool mod_AirResistance = true;
  bool mod_CheatBodyDirection = false;

  float accelerationMultiplier = 0.5f + _cache_AccelerationFactor;


  // rotate anim towards desired angle

  radian toDesiredAngle_capped = 0;
  if (mod_AllowRotation && physicsBias > 0.0f) {
    Vector3 animOutgoingVector = predictedOutgoingMovement.GetNormalized(0);
    if (FloatToEnumVelocity(predictedOutgoingMovement.GetLength()) == e_Velocity_Idle) animOutgoingVector = anim->GetOutgoingDirection().GetRotated2D(spatialState.angle);
    Vector3 desiredVector = adaptedDesiredMovement.GetNormalized(0);
    if (FloatToEnumVelocity(adaptedDesiredMovement.GetLength()) == e_Velocity_Idle) desiredVector = desiredBodyDirectionRel.GetRotated2D(spatialState.angle);
    radian toDesiredAngle = desiredVector.GetAngle2D(animOutgoingVector);
    if (fabs(toDesiredAngle) <= 0.5f * pi || animType.compare("sliding") == 0) { // if we want > x degrees, just skip it to next anim, it'll only look weird otherwise

      radian animChange = animOutgoingVector.GetAngle2D(spatialState.directionVec);
      if (fabs(animChange) > 0.06f * pi) {
        int sign = signSide(animChange);
        if (signSide(toDesiredAngle) == sign) {
          toDesiredAngle_capped = clamp(toDesiredAngle, -maxAngleMod_overAnimAngle, maxAngleMod_overAnimAngle);
        } else {
          toDesiredAngle_capped = clamp(toDesiredAngle, -maxAngleMod_underAnimAngle, maxAngleMod_underAnimAngle);
        }
      } else {
        // straight ahead anim, has no specific side.
        toDesiredAngle_capped = clamp(toDesiredAngle, -maxAngleMod_straightAnimAngle, maxAngleMod_straightAnimAngle);
      }
      //printf("animChange: %f radians, to desired angle: %f radians, sign: %i, sign side of to desired angle: %i, resulting capped angle: %f radians\n", animChange, toDesiredAngle, sign, signSide(toDesiredAngle), toDesiredAngle_capped);
    }
  }


  float maximumOutgoingVelocity = sprintVelocity;
  // brake on cornering
  if (mod_CorneringBraking) {

    float brakeBias = 0.8f;
    brakeBias *= (touch) ? 1.0f : 0.8f;
    brakeBias *= (1.0f - stat_agility * 0.2f);

    Vector3 animOutgoingMovement = anim->GetOutgoingMovement();
    animOutgoingMovement.Rotate2D(toDesiredAngle_capped);
    brakeBias *= std::pow(NormalizedClamp(spatialState.floatVelocity,
                                          idleVelocity, sprintVelocity - 0.5f),
                          0.8f);  // 0.5f);
    float maxVelo =
        sprintVelocity *
        ((1.0f - brakeBias) +
         ((1.0f -
           std::pow(fabs(animOutgoingMovement.GetNormalized(0).GetAngle2D(
                             Vector3(0, -1, 0)) /
                         pi),
                    0.5f)) *
          brakeBias));
    maximumOutgoingVelocity = maxVelo;
  }


  // --- loop da loop ------------------------------------------------------------------------------------------------------------------------------------------
  for (int time_ms = 0; time_ms < anim->GetFrameCount() * 10; time_ms += timeStep_ms) {

    // start with +1, because we want to influence the first frame as well
    // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
    // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
    float frameBias = (time_ms + 10) / (float)((anim->GetEffectiveFrameCount() + 1) * 10);

    float lagExp = 1.0f;
    if (mod_PointinessCurve && physicsBias > 0.0f && (animType.compare("ballcontrol") == 0 || animType.compare("movement") == 0)) {
      lagExp = 1.4f - _cache_AgilityFactor * 0.8f;
      lagExp *= 1.2f - stat_agility * 0.4f;
      if (touch) {
        lagExp += -0.1f + clamp(difficultyFactor * 0.4f, 0.0f, 0.5f);
      } else {
        lagExp += -0.2f + clamp(difficultyFactor * 0.2f, 0.0f, 0.2f);
      }

      lagExp = clamp(lagExp, 0.25f, 4.0f);
      if (touch && time_ms < animTouchFrame * 10) lagExp = std::max(lagExp, 0.7f); // else, we could 'miss' the ball because we're already turned around too much

      lagExp = lagExp * physicsBias + 1.0f * (1.0f - physicsBias);
    }
    float adaptedFrameBias = pow(frameBias, lagExp);
    Vector3 animMovement = CalculateMovementAtFrame(origPositionCache, anim->GetEffectiveFrameCount() * adaptedFrameBias, 1).GetRotated2D(spatialState.angle);

    float animVelo = animMovement.GetLength();
    Vector3 adaptedAnimMovement = animMovement;
    float adaptedAnimVelo = animVelo;


    // adapt sprint velocity to player's max velocity stat

    if (animVelo > walkSprintSwitch && (animType.compare("movement") == 0 || animType.compare("ballcontrol") == 0 || animType.compare("trap") == 0)) {

      if (maxVelocity > animVelo) { // only speed up, don't slow down. may be faster parts (jumps and such) within anim, allow this
        adaptedAnimVelo = StretchSprintTo(animVelo, animSprintVelocity, maxVelocity);
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }

    }


    float maxSlower = 1.6f; // rationale: don't want to end up below dribbleVelocity - idleDribbleSwitch (= change velocity)
    if (touch) maxSlower = 1.2f;
    float maxFaster = 0.0f;
    if (touch) maxFaster = 0.0f;
    if (temporalMovement.GetLength() > adaptedAnimVelo) maxFaster = std::min(0.0f + 1.0f * (1.0f - frameBias), std::max(maxFaster, temporalMovement.GetLength() - adaptedAnimVelo)); // ..already going faster.. well okay, allow this
    if (maxFaster > 0) maxFaster *= std::max(0.0f, adaptedAnimMovement.GetNormalizedMax(1.0f).GetDotProduct(adaptedDesiredMovement.GetNormalized(0))); // only go faster if it's in the right direction
    if (animType.compare("sliding") == 0) maxFaster = 100;
    float desiredVelocity = adaptedDesiredMovement.GetLength();
    adaptedAnimVelo = clamp(desiredVelocity, adaptedAnimVelo - maxSlower, adaptedAnimVelo + maxFaster);
    adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;

    if (mod_CorneringBraking) {
      float frameBiasedMaximumOutgoingVelocity = sprintVelocity * (1.0f - frameBias) + maximumOutgoingVelocity * frameBias;
      if (adaptedAnimVelo > frameBiasedMaximumOutgoingVelocity) {
        adaptedAnimVelo = frameBiasedMaximumOutgoingVelocity;
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }
    }

    if (mod_MaximumAccelDecel) {
      // this is basically meant to enforce transitions to be smoother, disallowing bizarre steps. however, with low enough max values, it can also serve as a physics slowness thing.
      // in that regard, the maxaccel part is somewhat similar to the air resistance mod below. they can live together; this one can serve as a constant maximum, and the air resistance as a velocity-based maximum.
      // update: decided to not let them live together, this should now purely be used for capping transition speed
      float maxAccelMPS = 20.0f;
      float maxDecelMPS = 20.0f;
      float currentVelo = temporalMovement.GetLength();
      float veloChangeMPS = (adaptedAnimVelo - currentVelo) / ((float)timeStep_ms * 0.001f);
      if (veloChangeMPS < -maxDecelMPS || veloChangeMPS > maxAccelMPS) {
        adaptedAnimVelo = currentVelo + clamp(veloChangeMPS, -maxDecelMPS, maxAccelMPS) * ((float)timeStep_ms * 0.001f);
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }
    }

    Vector3 resultingPhysicsMovement = adaptedAnimMovement;

    // angle
    resultingPhysicsMovement = resultingPhysicsMovement.GetRotated2D(toDesiredAngle_capped * frameBias);// * pow(frameBias, 0.6f));


    // --- stay true to anim? -----------------------------------------------------------------------------------------------------------------------

    resultingPhysicsMovement = resultingPhysicsMovement * physicsBias + animMovement * (1.0f - physicsBias);


    // that's it, we now know where we want to go in life

    Vector3 toDesired = (resultingPhysicsMovement - temporalMovement);


    // --- end --------------------------------------------------------------------------------------------------------------------------------------


    assert(toDesired.coords[2] == 0.0f);


    float penaltyBreakFactor = 0.0f;
    if (mod_BrakeOnTouch) {
      // slow down after touching ball
      // (precalc at touchframe, because temporalMovement will change because of this, so if we don't precalc then changing numBrakeFrames will change the amount of effect)
      int numBrakeFrames = 15;
      if (touch && time_ms >= animTouchFrame * 10 && time_ms < (animTouchFrame + numBrakeFrames) * 10) {
        int brakeFramesInto = (time_ms - (animTouchFrame * 10)) / 10;
        float brakeFrameFactor =
            std::pow(1.0f - (brakeFramesInto / (float)numBrakeFrames), 0.5f);

        float touchBrakeFactor = 0.3f;

        float touchDifficultyFactor = clamp((difficultyFactor + 0.7f) * (1.0f - stat_dribble * 0.4f), 0.0, 1.0f);
        touchDifficultyFactor *=
            1.0f -
            std::pow(
                fabs(anim->GetOutgoingAngle()) / pi,
                0.75f);  // don't help with braking (when going nearer 180 deg)

        float veloFactor = NormalizedClamp(temporalMovement.GetLength(), walkVelocity, sprintVelocity);

        penaltyBreakFactor = touchDifficultyFactor * veloFactor * brakeFrameFactor * touchBrakeFactor;
      }
    }

    /* part of new method, but needs some debugging/unittesting
    // increase over multiple frames for smoother effect, and so we get a proper effect even if maxChange isn't very high
    int numBrakeFrames = 5;
    if (touch && time_ms >= animTouchFrame * 10 && time_ms < (animTouchFrame + numBrakeFrames) * 10) {
      int framesInto = (time_ms - (animTouchFrame * 10)) / 10;
      toDesired += -temporalMovement.GetNormalized(0) * (ballTouchSlowdownAmount / (float)(numBrakeFrames - framesInto));
      // if (player->GetDebug()) {
      //   printf("divisor: %i, frame %i, time: %i, vector: ", numBrakeFrames, animTouchFrame, time_ms);
      //   ballTouchSlowdownVector.Print();
      // }
    }
    */

    if (mod_MaxCornering) {
      Vector3 predictedMovement = temporalMovement + toDesired;
      float startVelo = idleDribbleSwitch;
      if (temporalMovement.GetLength() > startVelo && predictedMovement.GetLength() > startVelo) {
        radian angle = predictedMovement.GetNormalized().GetAngle2D(temporalMovement.GetNormalized());
        float maxAngleFactor = 1.0f * (timeStep_ms / 1000.0f);
        maxAngleFactor *= (0.7f + 0.3f * stat_agility);
        if (!touch) maxAngleFactor *= 1.5f;
        radian maxAngle = maxAngleFactor * pi;
        float veloFactor = std::pow(
            NormalizedClamp(temporalMovement.GetLength(), 0, sprintVelocity),
            1.0f);
        maxAngle /= (veloFactor + 0.01f);

        if (fabs(angle) > maxAngle) {

          int mode = 1; // 0: restrict max angle, 1: restrict velocity

          if (mode == 0) {
            Vector3 restrictedPredictedMovement = predictedMovement.GetRotated2D((fabs(angle) - maxAngle) * -signSide(angle));
            Vector3 newToDesired = restrictedPredictedMovement - temporalMovement;
            toDesired = newToDesired;
          } else if (mode == 1) {
            radian overAngle = fabs(angle) - maxAngle; // > 0
            toDesired += -temporalMovement * clamp(overAngle / pi * 3.0f, 0.0f, 1.0f);// was: 3
          }

        }
      }
    }

    if (mod_MaxChange) {
      float maxChange = 0.03f;
      if (animType.compare("trip") == 0) maxChange *= 0.7f;
      if (animType.compare("sliding") == 0) maxChange = 0.1f;
      // no power first few frames, so transitions are smoother
      //maxChange *= 0.3f + 0.7f * curve(NormalizedClamp(time_ms, 0.0f, 80.0f), 1.0f);
      float veloFactor = std::pow(
          NormalizedClamp(temporalMovement.GetLength(), 0, sprintVelocity),
          1.5f);
      float firstStepFactor = veloFactor;
      if (animType.compare("movement") == 0) firstStepFactor *= 0.4f;
      maxChange *= (1.0f - firstStepFactor) + firstStepFactor * curve(NormalizedClamp(time_ms, 0.0f, 160.0f), 1.0f);

      maxChange *= 1.2f - veloFactor * 0.4f;

      maxChange *= 0.75f + _cache_AgilityFactor * 0.5f;

      // lose power 'around' touch

      // if (touch && time_ms >= animTouchFrame * 10) {
      //   int influenceFrames = 16; // number of frames to slow down on each 'side' of the balltouch
      //   //float frameBias = NormalizedClamp(fabs((animTouchFrame * 10) - time_ms), 0, influenceFrames * 10) * 0.7f + 0.3f;
      //   float frameBias = NormalizedClamp(time_ms - (animTouchFrame * 10), 0, influenceFrames * 10) * 1.0f;// + 0.1f;
      //   frameBias = curve(frameBias, 1.0f);
      //   maxChange *= clamp(frameBias, 0.1f, 1.0f);
      // }

      maxChange *= powerFactor;

      float desiredLength = toDesired.GetLength();
      float maxAddition = maxChange * timeStep_ms;

      toDesired.NormalizeMax(std::min(desiredLength, maxAddition));
    }


    // air resistance

    if (mod_AirResistance && animType.compare("sliding") != 0 && animType.compare("deflect") != 0) {
      float veloExp = 1.8f;
      float accelPower = 11.0f * accelerationMultiplier;
      float falloffStartVelo = idleDribbleSwitch;

      if ((temporalMovement + toDesired).GetLength() > falloffStartVelo) {

        // less accelpower on tough anims
        accelPower *= 1.0f - difficultyPenaltyFactor * 0.4f;

        // wolfram alpha to show difference in sin before/after exp order: 10 * (sin(((1.0 - x ^ 2.0) - 0.5) * pi) * 0.5 + 0.5), 10 * ((1.0 - (sin((x - 0.5) * pi) * 0.5 + 0.5)) ^ 2.0) | from x = 0.0 to 1.0

        float veloAirResistanceFactor = clamp(
            std::pow(clamp((temporalMovement.GetLength() - falloffStartVelo) /
                               (player->GetMaxVelocity() - falloffStartVelo),
                           0.0f, 1.0f),
                     veloExp),
            0.0f, 1.0f);

        // simulate footsteps - powered in the middle
        //veloAirResistanceFactor = 1.0f - ((1.0f - veloAirResistanceFactor) * pow(sin(frameBias * pi), 2.0f));

        // circular version
        Vector3 forwardVector;
        if ((temporalMovement + toDesired).GetLength() > temporalMovement.GetLength()) { // outside the 'velocity circle'
          Vector3 destination = temporalMovement + toDesired;
          float velo = temporalMovement.GetLength();
          float accel = destination.GetLength() - velo;
          forwardVector = destination.GetNormalized(0) * accel;
        }

        float accelerationAddition = forwardVector.GetLength();
        float maxAccelerationMPS = accelPower * (1.0f - veloAirResistanceFactor) * (stat_acceleration * 0.3f + 0.7f);
        float maxAccelerationAddition = maxAccelerationMPS * (timeStep_ms / 1000.0f);
        if (accelerationAddition > maxAccelerationAddition) {
          float remainingFactor = maxAccelerationAddition / accelerationAddition;
          toDesired = toDesired - forwardVector * (1.0f - remainingFactor);
        }

      }

    }


    // MAKE IT SEW! http://static.wixstatic.com/media/fc58ad_c0ef2d69d98f4f7ba8e7e488f0e28ece.jpg

    Vector3 tmpTemporalMovement = temporalMovement + toDesired;

    // make sure outgoing velocity is of the same idleness as the anim
    if (time_ms >= (anim->GetFrameCount() - 2) * 10) {

      bool hardQuantize = true;
      if (!hardQuantize && anim->GetVariable("outgoing_special_state").compare("") != 0) hardQuantize = true;

      if (!hardQuantize) {
        // soft version
        if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) == e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) != e_Velocity_Idle) tmpTemporalMovement.NormalizeTo(idleDribbleSwitch - 0.01f);
        else if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) == e_Velocity_Idle) tmpTemporalMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle).GetNormalizedTo(idleDribbleSwitch + 0.01f);
      } else {
        // hard version
        if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) == e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) != e_Velocity_Idle) tmpTemporalMovement = 0;
        else if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) == e_Velocity_Idle) tmpTemporalMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle).GetNormalizedTo(dribbleVelocity);
      }
    }

    assert(tmpTemporalMovement.coords[2] == 0.0f);
    temporalMovement = tmpTemporalMovement;

    if (time_ms >= (anim->GetFrameCount() - 2) * 10) penaltyBreakFactor = 0.0f;
    currentPosition += temporalMovement * (1.0f - penaltyBreakFactor) * (timeStep_ms / 1000.0f);
    assert(currentPosition.coords[2] == 0.0f);

    if (time_ms % 10 == 0) {
      positions_ret.push_back(currentPosition);
    }

    /*
    // dynamic timestep: more precision at high velocities
    if ((int)time_ms % 10 == 0) {
      //if (temporalMovement.GetLength() > walkVelocity) timeStep_ms = 5; else timeStep_ms = 10;
      timeStep_ms = 10;
    }
    */
  }

  assert(positions_ret.size() >= (unsigned int)anim->GetFrameCount());
  resultingMovement = temporalMovement;


  if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle && FloatToEnumVelocity(resultingMovement.GetLength()) != e_Velocity_Idle) {
    rotationOffset_ret = resultingMovement.GetRotated2D(-spatialState.angle).GetAngle2D(anim->GetOutgoingMovement());
  } else {
    rotationOffset_ret = toDesiredAngle_capped * physicsBias;
  }


  // body direction assist
  if (mod_CheatBodyDirection && useDesiredBodyDirection && animType.compare("movement") == 0) {

    float angleFactor = 0.5f;
    radian maxAngle = 0.25f * pi;

    radian predictedAngleRel = anim->GetOutgoingAngle() + anim->GetOutgoingBodyAngle() + rotationOffset_ret;
    radian desiredRotationOffset = desiredBodyDirectionRel.GetRotated2D(-predictedAngleRel).GetAngle2D(Vector3(0, -1, 0));

    if (fabs(desiredRotationOffset) < 0.5f * pi) { // else: too much

      float outgoingVelocityFactorInv = 1.0f - NormalizedClamp(resultingMovement.GetLength(), idleDribbleSwitch, sprintVelocity - 1.0f) * 1.0f;
      float animLengthFactor = NormalizedClamp(anim->GetFrameCount(), 0, 25);
      radian maximizedRotationOffset = clamp(desiredRotationOffset, outgoingVelocityFactorInv * animLengthFactor * angleFactor * -maxAngle,
                                                                    outgoingVelocityFactorInv * animLengthFactor * angleFactor *  maxAngle);

      // if (player->GetDebug() && animType.compare("movement") == 0) {
      //   //printf("rotationOffset: %f radians\n", rotationOffset);
      //   SetYellowDebugPilon(spatialState.position + positions_ret.back() + resultingMovement.GetNormalized(0) * 3.0f);
      //   SetGreenDebugPilon(spatialState.position + positions_ret.back() + Vector3(0, -1, 0).GetRotated2D(spatialState.angle + predictedAngleRel + rotationOffset) * 3.1f);
      //   SetRedDebugPilon(spatialState.position + positions_ret.back() + Vector3(0, -1, 0).GetRotated2D(spatialState.angle + predictedAngleRel + rotationOffset + maximizedRotationOffset) * 3.2f);
      // }
      rotationOffset_ret += maximizedRotationOffset;

      // if (player->GetDebug() && animType.compare("movement") == 0) {
      //   SetYellowDebugPilon(spatialState.position + Vector3(GetDirectionVec()) * 3.0f);
      //   SetGreenDebugPilon(spatialState.position + Vector3(GetDirectionVec().GetRotated2D(maximizedRotationOffset)) * 3.0f);
      // }

      // mee bezig. zie ook eerder in deze functie, moet ook aangepast, maximum angle shizzle, wil niet over 180 graden
      // if (fabs(predictedAngleRel + rotationOffset) > 0.75f * pi && SignSide(maximizedRotationOffset) == SignSide(predictedAngleRel)) {
      // }
    }
  }

  assert(resultingMovement.coords[2] == 0.0f);

  return resultingMovement;
}


Vector3 HumanoidBase::ForceIntoAllowedBodyDirectionVec(const Vector3 &src) const {

  // check what allowed dir this vector is closest to
  float bestDot = -1.0f;
  int bestIndex = 0;
  for (unsigned int i = 0; i < allowedBodyDirVecs.size(); i++) {
    float nDotL = allowedBodyDirVecs.at(i).GetDotProduct(src);
    if (nDotL > bestDot) {
      bestDot = nDotL;
      bestIndex = i;
    }
  }

  return allowedBodyDirVecs.at(bestIndex);
}

radian HumanoidBase::ForceIntoAllowedBodyDirectionAngle(radian angle) const {

  float bestAngleDiff = 10000.0;
  int bestIndex = 0;
  for (unsigned int i = 0; i < allowedBodyDirAngles.size(); i++) {
    float diff = fabs(allowedBodyDirAngles.at(i) - angle);
    if (diff < bestAngleDiff) {
      bestAngleDiff = diff;
      bestIndex = i;
    }
  }

  return allowedBodyDirAngles.at(bestIndex);
}

Vector3 HumanoidBase::ForceIntoPreferredDirectionVec(const Vector3 &src) const {

  float bestDot = -1.0f;
  int bestIndex = 0;
  for (unsigned int i = 0; i < preferredDirectionVecs.size(); i++) {
    float nDotL = preferredDirectionVecs.at(i).GetDotProduct(src);
    if (nDotL > bestDot) {
      bestDot = nDotL;
      bestIndex = i;
    }
  }

  return preferredDirectionVecs.at(bestIndex);
}

radian HumanoidBase::ForceIntoPreferredDirectionAngle(radian angle) const {

  float bestAngleDiff = 10000.0;
  int bestIndex = 0;
  for (unsigned int i = 0; i < preferredDirectionAngles.size(); i++) {
    float diff = fabs(preferredDirectionAngles.at(i) - angle);
    if (diff < bestAngleDiff) {
      bestAngleDiff = diff;
      bestIndex = i;
    }
  }

  return preferredDirectionAngles.at(bestIndex);
}
