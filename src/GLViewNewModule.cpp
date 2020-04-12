#include "GLViewNewModule.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "AftrGLRendererBase.h"
#include "irrKlang.h"

//If we want to use way points, we need to include this.
#include "NewModuleWayPoints.h"
#include "Module4Message.h"
#include "NetMessengerClient.h"
#include "PxPhysicsAPI.h"
#include "PxFoundation.h"
//#include "vehicle/PxVehicleSDK.h"
#include "extensions/PxDefaultAllocator.h"
#include "extensions/PxDefaultErrorCallback.h"
#include "Module5Physics.h""

using namespace Aftr;

GLViewNewModule* GLViewNewModule::New( const std::vector< std::string >& args )
{
   GLViewNewModule* glv = new GLViewNewModule( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}


GLViewNewModule::GLViewNewModule( const std::vector< std::string >& args ) : GLView( args )
{
   //Initialize any member variables that need to be used inside of LoadMap() here.
   //Note: At this point, the Managers are not yet initialized. The Engine initialization
   //occurs immediately after this method returns (see GLViewNewModule::New() for
   //reference). Then the engine invoke's GLView::loadMap() for this module.
   //After loadMap() returns, GLView::onCreate is finally invoked.

   //The order of execution of a module startup:
   //GLView::New() is invoked:
   //    calls GLView::init()
   //       calls GLView::loadMap() (as well as initializing the engine's Managers)
   //    calls GLView::onCreate()

   //GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
}


void GLViewNewModule::onCreate()
{
   //GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
   //At this point, all the managers are initialized. That is, the engine is fully initialized.

   if( this->pe != NULL )
   {
      //optionally, change gravity direction and magnitude here
      //The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
      this->pe->setGravityScalar( Aftr::GRAVITY );
   }
   this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
   //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
   //irrklang::ISoundEngine* sound = irrklang::createIrrKlangDevice();
   this->sound = irrklang::createIrrKlangDevice();

   //3D Audio
   this->sound->setRolloffFactor(0.25);
   
   //Defining look direction and position
   Aftr::Vector look = cam->getLookDirection();
   Aftr::Vector position = cam->getPosition();

   //Setting
   this->sound->setListenerPosition(irrklang::vec3d(position.x, position.y, position.z), irrklang::vec3d(look.x, look.y, look.z));
   this->sound->setDefault3DSoundMinDistance(5.0f);

   client = NetMessengerClient::New("127.0.0.1", "12683");
}


GLViewNewModule::~GLViewNewModule()
{
   //Implicitly calls GLView::~GLView()

    if(gscene != nullptr) {
        gscene->release();
    }
    if(f != nullptr) {
        f->release();
    }
    if(p != nullptr) {
        p->release();
    }
}


void GLViewNewModule::updateWorld()
{
   GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after
                          //this call.
   gscene->simulate(0.5);
   gscene->fetchResults(true);

   {
       physx::PxU32 numActors = 0;
       physx::PxActor** actors = gscene->getActiveActors(numActors);
       //make sure you set physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true in your scene!
       //poses that have changed since the last update
       for (physx::PxU32 i = 0; i < numActors; ++i)
       {
           physx::PxActor* actor = actors[i];

          Module5Physics* wo = static_cast<Module5Physics*>(actor->userData);
           //at some point earlier the actor’s userdata was set to the corresponding WO’s pointer
           wo->updatePoseFromPhysicsEngine(actor);//add this function to your inherited class
       }
   }


  
   //Updating the world after creating it
   //Defining look direction and position
   Aftr::Vector look = cam->getLookDirection();
   Aftr::Vector position = cam->getPosition();

   //Setting
   this->sound->setListenerPosition(irrklang::vec3d(position.x, position.y, position.z), irrklang::vec3d(look.x, look.y, look.z));
}


void GLViewNewModule::onResizeWindow( GLsizei width, GLsizei height )
{
   GLView::onResizeWindow( width, height ); //call parent's resize method.
}


void GLViewNewModule::onMouseDown( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseDown( e );
}


void GLViewNewModule::onMouseUp( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseUp( e );
}


void GLViewNewModule::onMouseMove( const SDL_MouseMotionEvent& e )
{
   GLView::onMouseMove( e );
}


void GLViewNewModule::onKeyDown(const SDL_KeyboardEvent& key)
{
    //USED: W,A,S,D,0,1,3

    GLView::onKeyDown(key);

    //Camera movements with WASD keys & Arrow Keys
    if (key.keysym.sym == SDLK_w || key.keysym.sym == SDLK_UP)
    {
        this->cam->moveInLookDirection();
    }
    if (key.keysym.sym == SDLK_a || key.keysym.sym == SDLK_LEFT)
    {
        this->cam->moveLeft();
    }
    if (key.keysym.sym == SDLK_s || key.keysym.sym == SDLK_DOWN)
    {
        this->cam->moveOppositeLookDirection();
    }
    if (key.keysym.sym == SDLK_d || key.keysym.sym == SDLK_RIGHT)
    {
        this->cam->moveRight();
    }

    //Number Key Presses

    if (key.keysym.sym == SDLK_0)
        this->setNumPhysicsStepsPerRender(1);

    if (key.keysym.sym == SDLK_1)
    {
        sound->play2D("../mm/sounds/space.ogg");
    }

    if (key.keysym.sym == SDLK_3)
    {
        irrklang::vec3df position(0, 0, 20);
        irrklang::ISound* sound3d = sound->play3D("../mm/sounds/alienspaceship.wav", position, false, true); //playlooped is false & start paused is true

        if (sound3d) {
            sound3d->setMinDistance(25.0f);
            sound3d->setVolume(0.1);
            sound3d->setIsPaused(false);//unpauses the sound
        }
    }

    if (key.keysym.sym == SDLK_5)
    {
        createAlien();
    }

    if (key.keysym.sym == SDLK_6)
    {
        Module4Message msg1;
        client->sendNetMsgSynchronousTCP(msg1);
        createUfo();
    }
}


void GLViewNewModule::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}


void Aftr::GLViewNewModule::loadMap()
{
    f = PxCreateFoundation(PX_PHYSICS_VERSION, a, e);

    gpvd = PxCreatePvd(*f);
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gpvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

    p = PxCreateBasePhysics(PX_PHYSICS_VERSION, *f, physx::PxTolerancesScale(), true, gpvd);
    physx::PxSceneDesc s(p->getTolerancesScale());
    s.gravity = physx::PxVec3(0.0f, 0.0f, 0.0f);
    s.flags = physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    d = physx::PxDefaultCpuDispatcherCreate(2);
    s.cpuDispatcher = d;
    s.filterShader = physx::PxDefaultSimulationFilterShader;
    gscene = p->createScene(s);
    
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = true;
   this->glRenderer->isUsingShadowMapping( true ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

   this->cam->setPosition( 15,15,10 );

   std::string shinyRedPlasticCube( ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl" );
   std::string wheeledCar( ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl" );
   std::string grass( ManagerEnvironmentConfiguration::getLMM() + "/models/moonFloor400x400_pp.wrl" );
   std::string human( ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl" );
   std::string alien( ManagerEnvironmentConfiguration::getLMM() + "/models/Alien.obj" );
   std::string astro( ManagerEnvironmentConfiguration::getLMM() + "/models/Astronaut.obj" );
   
   //SkyBox Textures readily available
   std::vector< std::string > skyBoxImageNames; //vector to store texture paths
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_evening+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg" );
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );

   float ga = 0.1f; //Global Ambient Light level for this module
   ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
   WOLight* light = WOLight::New();
   light->isDirectionalLight( true );
   light->setPosition( Vector( 0, 0, 100 ) );
   //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
   //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
   light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
   light->setLabel( "Light" );
   worldLst->push_back( light );

   //Create the SkyBox
   WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
   wo->setPosition( Vector( 0,0,0 ) );
   wo->setLabel( "Sky Box" );
   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   worldLst->push_back( wo );

   ////Create the infinite grass plane (the floor)
   wo = Module5Physics::New(p, gscene, grass, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
   wo->setPosition( Vector( 0, 0, 0 ) );
   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 ).getMultiTextureSet().at( 0 )->setTextureRepeats(5.0f);
   wo->setLabel( "Moon" );
   worldLst->push_back( wo );

   ////Create the infinite grass plane that uses the Open Dynamics Engine (ODE)
   //wo = WOStatic::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
   //((WOStatic*)wo)->setODEPrimType( ODE_PRIM_TYPE::PLANE );
   //wo->setPosition( Vector(0,0,0) );
   //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
   //wo->setLabel( "Grass" );
   //worldLst->push_back( wo );

   ////Create the infinite grass plane that uses NVIDIAPhysX(the floor)
   //wo = WONVStaticPlane::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
   //wo->setPosition( Vector(0,0,0) );
   //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
   //wo->setLabel( "Grass" );
   //worldLst->push_back( wo );

   ////Create the infinite grass plane (the floor)
   //wo = WONVPhysX::New( shinyRedPlasticCube, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
   //wo->setPosition( Vector(0,0,50.0f) );
   //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //wo->setLabel( "Grass" );
   //worldLst->push_back( wo );

   //wo = WONVPhysX::New( shinyRedPlasticCube, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
   //wo->setPosition( Vector(0,0.5f,75.0f) );
   //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //wo->setLabel( "Grass" );
   //worldLst->push_back( wo );

   //wo = WONVDynSphere::New( ManagerEnvironmentConfiguration::getVariableValue("sharedmultimediapath") + "/models/sphereRp5.wrl", Vector(1.0f, 1.0f, 1.0f), mstSMOOTH );
   //wo->setPosition( 0,0,100.0f );
   //wo->setLabel( "Sphere" );
   //this->worldLst->push_back( wo );

   //wo = WOHumanCal3DPaladin::New( Vector( .5, 1, 1 ), 100 );
   //((WOHumanCal3DPaladin*)wo)->rayIsDrawn = false; //hide the "leg ray"
   //((WOHumanCal3DPaladin*)wo)->isVisible = false; //hide the Bounding Shell
   //wo->setPosition( Vector(20,20,20) );
   //wo->setLabel( "Paladin" );
   //worldLst->push_back( wo );
   //actorLst->push_back( wo );
   //netLst->push_back( wo );
   //this->setActor( wo );
   //
   //wo = WOHumanCyborg::New( Vector( .5, 1.25, 1 ), 100 );
   //wo->setPosition( Vector(20,10,20) );
   //wo->isVisible = false; //hide the WOHuman's bounding box
   //((WOHuman*)wo)->rayIsDrawn = false; //show the 'leg' ray
   //wo->setLabel( "Human Cyborg" );
   //worldLst->push_back( wo );
   //actorLst->push_back( wo ); //Push the WOHuman as an actor
   //netLst->push_back( wo );
   //this->setActor( wo ); //Start module where human is the actor

   ////Create and insert the WOWheeledVehicle
   //std::vector< std::string > wheels;
   //std::string wheelStr( "../../../shared/mm/models/WOCar1970sBeaterTire.wrl" );
   //wheels.push_back( wheelStr );
   //wheels.push_back( wheelStr );
   //wheels.push_back( wheelStr );
   //wheels.push_back( wheelStr );
   //wo = WOCar1970sBeater::New( "../../../shared/mm/models/WOCar1970sBeater.wrl", wheels );
   //wo->setPosition( Vector( 5, -15, 20 ) );
   //wo->setLabel( "Car 1970s Beater" );
   //((WOODE*)wo)->mass = 200;
   //worldLst->push_back( wo );
   //actorLst->push_back( wo );
   //this->setActor( wo );
   //netLst->push_back( wo );
    
   wo = WO::New(astro, Vector(200,200,200));
   wo->setPosition( Vector(50,50,10) );
   wo->rotateAboutGlobalZ(225);
   wo->moveRelative((25, 25, 10));
   wo->setLabel( "Astronaut" );
   worldLst->push_back( wo );
   
   createNewModuleWayPoints();
}

void GLViewNewModule::createAlien()
{
    Module4Message msg1;
    // Place the alien
    msg1.model = ManagerEnvironmentConfiguration::getLMM() + "/models/Alien.obj";
    msg1.size = Vector(200, 200, 200);
    msg1.position = Vector(25, 50, 10);
    client->sendNetMsgSynchronousTCP(msg1);

    WO* wo = WO::New(msg1.model, msg1.size);
    wo->setPosition(msg1.position);
    ManagerGLView::getGLView()->getWorldContainer()->push_back(wo);
}

void GLViewNewModule::createUfo()
{
    Module5Physics* wo = Module5Physics::New(p, gscene, (ManagerEnvironmentConfiguration::getLMM() + "/models/Ufo.obj"), { 200, 200, 200 }, Aftr::MESH_SHADING_TYPE::mstAUTO);
    wo->setPosition({ 25, 50, 10 });
    worldLst->push_back(wo);
    if (gscene != nullptr)
    {
        std::cout << "Ufo added\n";
    }
    else {
        std::cout << "Null scene\n";
    }
}

void GLViewNewModule::createNewModuleWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical* wayPt = WOWP1::New( params, 3 );
   wayPt->setPosition( Vector( 50, 0, 3 ) );
   worldLst->push_back( wayPt );
}

