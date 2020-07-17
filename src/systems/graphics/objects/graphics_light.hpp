// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_GRAPHICSSYSTEM_OBJECT_LIGHT
#define _HPP_GRAPHICSSYSTEM_OBJECT_LIGHT

#include "base/math/vector3.hpp"
#include "scene/objects/light.hpp"

#include "../graphics_object.hpp"

#include "../resources/texture.hpp"

#include "../rendering/interface_renderer3d.hpp"

namespace blunted {

  class GraphicsLight_LightInterpreter;

  class GraphicsLight : public GraphicsObject {

    public:
      GraphicsLight(GraphicsScene *graphicsScene);
      virtual ~GraphicsLight();

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_ObjectType objectType);

      virtual void SetPosition(const Vector3 &newPosition);
      virtual Vector3 GetPosition() const;

      virtual void SetColor(const Vector3 &newColor);
      virtual Vector3 GetColor() const;

      virtual void SetRadius(float radius);
      virtual float GetRadius() const;

      virtual void SetType(e_LightType lightType);
      virtual e_LightType GetType() const;

      virtual void SetShadow(bool shadow);
      virtual bool GetShadow() const;

      std::vector<ShadowMap> shadowMaps;

    protected:
      Vector3 position;
      Vector3 color;
      float radius;
      e_LightType lightType;
      bool shadow;

  };

  class GraphicsLight_LightInterpreter : public ILightInterpreter {

    public:
      GraphicsLight_LightInterpreter(GraphicsLight *caller);

      virtual e_SystemType GetSystemType() const { return e_SystemType_Graphics; }
      virtual void OnUnload();
      virtual void SetValues(const Vector3 &color, float radius);
      virtual void SetType(e_LightType lightType);
      virtual void SetShadow(bool shadow);
      virtual bool GetShadow();
      virtual void OnSpatialChange(const Vector3 &position, const Quaternion &rotation);
      virtual void EnqueueShadowMap(boost::intrusive_ptr<Camera> camera, std::deque < boost::intrusive_ptr<Geometry> > visibleGeometry);
      virtual ShadowMap GetShadowMap(const std::string &camName);
      virtual void OnPoke();

    protected:
      GraphicsLight *caller;

  };
}

#endif
