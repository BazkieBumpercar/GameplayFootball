// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "blunted.hpp"

#include "framework/scheduler.hpp"

#include "scene/objects/camera.hpp"
#include "scene/objects/image2d.hpp"
#include "scene/objects/geometry.hpp"
#include "scene/objects/skybox.hpp"
#include "scene/objects/light.hpp"
#include "scene/objects/joint.hpp"
#include "scene/objects/audioreceiver.hpp"
#include "scene/objects/sound.hpp"

#include "scene/objectfactory.hpp"

#include "scene/resources/surface.hpp"
#include "scene/resources/geometrydata.hpp"

#include "framework/scheduler.hpp"

#include "managers/systemmanager.hpp"
#include "managers/taskmanager.hpp"
#include "managers/usereventmanager.hpp"
#include "managers/environmentmanager.hpp"
#include "managers/scenemanager.hpp"
#include "managers/resourcemanager.hpp"
#include "managers/resourcemanagerpool.hpp"

#include "systems/isystem.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"
#include "base/properties.hpp"

#include "loaders/aseloader.hpp"
#include "loaders/imageloader.hpp"
#include "loaders/wavloader.hpp"

#include "utils/console.hpp"

#include "SDL/SDL_ttf.h"

namespace boost {
  void assertion_failed(char const * expr, char const * function, char const * file, long line) {
    char errorString[256];
    sprintf(errorString, "%s @ line %li: %s %s\n", file, line, function, expr);
    blunted::Log(blunted::e_FatalError, "boost", "assertion_failed", errorString);
  }
}

namespace blunted {

  ASELoader *aseLoader;
  ImageLoader *imageLoader;
  WAVLoader *wavLoader;

  Scheduler *scheduler;

  SystemManager *systemManager;
  TaskManager *taskManager;
  SceneManager *sceneManager;
  UserEventManager *userEventManager;
  ResourceManagerPool *resourceManagerPool;

  ObjectFactory *objectFactory;

  void RegisterObjectTypes(ObjectFactory* objectFactory);

  void Initialize(Properties &config) {

    printf("INIT\n");

    LogOpen();


    // initialize managers

    new EnvironmentManager();

    new SystemManager();
    systemManager = SystemManager::GetInstancePtr();

    new TaskManager();
    taskManager = TaskManager::GetInstancePtr();
    taskManager->Initialize();

    new SceneManager();
    sceneManager = SceneManager::GetInstancePtr();

    new ObjectFactory();
    objectFactory = ObjectFactory::GetInstancePtr();

    new UserEventManager();
    userEventManager = UserEventManager::GetInstancePtr();

    new ResourceManagerPool();
    resourceManagerPool = ResourceManagerPool::GetInstancePtr();


    // initialize resource managers

    boost::shared_ptr < ResourceManager<GeometryData> > geometryDataResourceManager(new ResourceManager<GeometryData>("geometrydata"));
    boost::shared_ptr < ResourceManager<Surface> > surfaceResourceManager(new ResourceManager<Surface>("surface"));
    boost::shared_ptr < ResourceManager<SoundBuffer> > soundBufferResourceManager(new ResourceManager<SoundBuffer>("soundbuffer"));

    ResourceManagerPool::GetInstance().RegisterManager(e_ResourceType_GeometryData, geometryDataResourceManager);
    ResourceManagerPool::GetInstance().RegisterManager(e_ResourceType_Surface, surfaceResourceManager);
    ResourceManagerPool::GetInstance().RegisterManager(e_ResourceType_SoundBuffer, soundBufferResourceManager);

    aseLoader = new ASELoader();
    geometryDataResourceManager->RegisterLoader("ase", aseLoader);
    imageLoader = new ImageLoader();
    surfaceResourceManager->RegisterLoader("jpg", imageLoader);
    surfaceResourceManager->RegisterLoader("png", imageLoader);
    wavLoader = new WAVLoader();
    soundBufferResourceManager->RegisterLoader("wav", wavLoader);

    TTF_Init();


    // initialize scheduler

    scheduler = new Scheduler(taskManager);

    RegisterObjectTypes(objectFactory);

  }

  void Run() {
    printf("RUN\n");

    scheduler->Run();

    // stop signaling systems
    // also clears links to user tasks
    scheduler->Exit();
    delete scheduler;
    scheduler = 0;
  }

  Scheduler *GetScheduler() {
    assert(scheduler);
    return scheduler;
  }

  void Exit() {
    printf("EXIT\n");

    Log(e_Notice, "blunted", "Exit", "exiting scenemanager");
    SceneManager::GetInstance().Exit();

    objectFactory->Exit();
    objectFactory->Destroy();

    Log(e_Notice, "blunted", "Exit", "exiting resourcemanagerpool");
    ResourceManagerPool::GetInstance().Exit();
    Log(e_Notice, "blunted", "Exit", "deleting resourcemanagerpool");
    ResourceManagerPool::GetInstance().Destroy();

    TaskManager::GetInstance().EmptyQueue();

    delete aseLoader;
    delete imageLoader;
    delete wavLoader;
    aseLoader = 0;
    imageLoader = 0;
    wavLoader = 0;

    Log(e_Notice, "blunted", "Exit", "exiting systemmanager");
    SystemManager::GetInstance().Exit();
    Log(e_Notice, "blunted", "Exit", "destroying systemmanager");
    SystemManager::GetInstance().Destroy();

    Log(e_Notice, "blunted", "Exit", "exiting taskmanager");
    TaskManager::GetInstance().Exit();
    Log(e_Notice, "blunted", "Exit", "destroying taskmanager");
    TaskManager::GetInstance().Destroy();

    Log(e_Notice, "blunted", "Exit", "destroying scenemanager");
    SceneManager::GetInstance().Destroy();

    Log(e_Notice, "blunted", "Exit", "exiting usereventmanager");
    UserEventManager::GetInstance().Exit();
    Log(e_Notice, "blunted", "Exit", "destroying usereventmanager");
    UserEventManager::GetInstance().Destroy();

    printf("READY\n");

    EnvironmentManager::GetInstance().Pause_ms(1000);
    EnvironmentManager::GetInstance().Destroy();

    TTF_Quit();
    SDL_Quit();

    LogClose();
  }


  // additional functions

  void RegisterObjectTypes(ObjectFactory *objectFactory) {
    boost::intrusive_ptr<Camera> camera(new Camera("Camera prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Camera, camera);
    boost::intrusive_ptr<Image2D> image2D(new Image2D("Image2D prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Image2D, image2D);
    boost::intrusive_ptr<Geometry> geometry(new Geometry("Geometry prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Geometry, geometry);
    boost::intrusive_ptr<Skybox> skybox(new Skybox("Skybox prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Skybox, skybox);
    boost::intrusive_ptr<Light> light(new Light("Light prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Light, light);
    boost::intrusive_ptr<Joint> joint(new Joint("Joint prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Joint, joint);
    boost::intrusive_ptr<AudioReceiver> audioReceiver(new AudioReceiver("AudioReceiver prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_AudioReceiver, audioReceiver);
    boost::intrusive_ptr<Sound> sound(new Sound("Sound prototype"));
    objectFactory->RegisterPrototype(e_ObjectType_Sound, sound);
  }

}
