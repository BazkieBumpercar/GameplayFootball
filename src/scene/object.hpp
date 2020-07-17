// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_OBJECT
#define _HPP_OBJECT

#include "defines.hpp"

#include "types/subject.hpp"
#include "types/spatial.hpp"
#include "base/properties.hpp"

namespace blunted {

  class ISystemObject;

  enum e_ObjectType {
    e_ObjectType_Camera = 1,
    e_ObjectType_Image2D = 2,
    e_ObjectType_Geometry = 3,
    e_ObjectType_Skybox = 4,
    e_ObjectType_Light = 5,
    e_ObjectType_Joint = 6,
    e_ObjectType_AudioReceiver = 7,
    e_ObjectType_Sound = 8,
    e_ObjectType_UserStart = 9
  };

  struct MustUpdateSpatialData {
    bool haveTo;
    e_SystemType excludeSystem;
  };

  // ATOMICITY: this class is responsible for doing about everything concurrently without crashing.
  // this implicitly accounts for atomicity in observers.
  class Object : public Subject<Interpreter>, public Spatial {

    public:
      Object(std::string name, e_ObjectType objectType);
      virtual ~Object();

      Object(const Object &src);

      virtual void Exit(); // ATOMIC

      virtual e_ObjectType GetObjectType();

      virtual bool IsEnabled() { return enabled; }
      virtual void Enable() { enabled = true; }
      virtual void Disable() { enabled = false; }

      virtual const Properties &GetProperties() const;
      virtual bool PropertyExists(const char *property) const;
      virtual const std::string &GetProperty(const char *property) const;

      virtual void SetProperties(Properties properties);
      virtual void SetProperty(const char *name, const char *value);

      virtual bool RequestPropertyExists(const char *property) const;
      virtual std::string GetRequestProperty(const char *property) const;
      virtual void AddRequestProperty(const char *property);
      virtual void SetRequestProperty(const char *property, const char *value);

      virtual void Synchronize();
      virtual void Poke(e_SystemType targetSystemType);

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

      Lockable<MustUpdateSpatialData> updateSpatialDataAfterPoke;

      virtual boost::intrusive_ptr<Interpreter> GetInterpreter(e_SystemType targetSystemType);
      virtual void LockSubject();
      virtual void UnlockSubject();

      virtual void SetPokePriority(int prio) { pokePriority.SetData(prio); }
      virtual int GetPokePriority() const { return pokePriority.GetData(); }

      // set these before creating system objects
      // todo: create 2 types of properties: one for pre-creating system objects (for example; what system objects should be created?)
      // and one for Init() stuff
      Properties properties;

      // todo: virtual OnChange method?

    protected:
      e_ObjectType objectType;

      mutable Lockable<int> pokePriority;

      // request these to be set by observing objects
      mutable Lockable<Properties> requestProperties;

      bool enabled;

  };

}

#endif
