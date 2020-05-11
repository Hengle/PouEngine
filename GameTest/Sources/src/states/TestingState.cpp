#include "states/TestingState.h"

#include "PouEngine/core/StatesManager.h"
#include "PouEngine/utils/Clock.h"

#include "PouEngine/scene/SceneNode.h"
#include "PouEngine/assets/AssetHandler.h"
#include "PouEngine/assets/SpriteSheetAsset.h"
#include "PouEngine/assets/TextureAsset.h"
#include "PouEngine/assets/MeshesHandler.h"
#include "PouEngine/assets/SoundAsset.h"
#include "PouEngine/assets/SoundBankAsset.h"
#include "PouEngine/audio/AudioEngine.h"

#include "PouEngine/renderers/UiRenderer.h"

#include "PouEngine/Types.h"

#include <glm/gtc/random.hpp>

TestingState::TestingState() :
    m_firstEntering(true),
    m_scene(nullptr)
{
    m_nbrFps = 0;

    m_character = nullptr;
    m_character2 = nullptr;
    m_croco = nullptr;
    m_duck = nullptr;
}

TestingState::~TestingState()
{
    this->leaving();
}

void TestingState::init()
{
    m_firstEntering = false;



    m_gameServer.create(46969);
    m_gameClient.create(42420);
    m_gameClient.connect(pou::NetAddress(127,0,0,1,m_gameServer.getPort()));

    //m_camVelocity = glm::vec3(0);
    m_activeCroc = false;
    m_activeDuck = false;

    m_scene = new pou::Scene();

    pou::AssetLoadType loadType = pou::LoadType_InThread;

    pou::SoundBanksHandler::loadAssetFromFile("../data/MasterSoundBank.bank");
    auto music = pou::AudioEngine::createEvent("event:/Music");
    pou::AudioEngine::playEvent(music);

    m_character = new PlayableCharacter();
    m_character->loadModel("../data/char1/char1XML.txt");
    m_character->setPosition(0,0,1);
    m_scene->getRootNode()->addChildNode(m_character);

    m_character2 = new Character();
    m_character2->loadModel("../data/char1/char1XML.txt");
    m_character2->setPosition(-700,-700,1);
    m_scene->getRootNode()->addChildNode(m_character2);

    m_croco = new Character();
    m_croco->loadModel("../data/croco/crocoXML.txt");
    m_croco->setPosition(220,70,1);
    m_croco->setRotationRadius(110.0f);
    m_scene->getRootNode()->addChildNode(m_croco);

    m_duck = new Character();
    m_duck->loadModel("../data/duck/duckXML.txt");
    m_duck->setPosition(-200,70,1);
    m_scene->getRootNode()->addChildNode(m_duck);

    for(auto i = 1 ; i < 10 ; i++)
    {
        m_duckSwarm.push_back(new Character());
        m_duckSwarm.back()->loadModel("../data/duck/duckXML.txt");
        m_duckSwarm.back()->setPosition(glm::linearRand(-2000*i,2000*i),glm::linearRand(-1000*i,1000*i),1);
        m_scene->getRootNode()->addChildNode(m_duckSwarm.back());
    }

    pou::SpriteSheetAsset *grassSheet = pou::SpriteSheetsHandler::loadAssetFromFile("../data/grasslands/grassXML.txt");

    for(auto x = -10 ; x < 10 ; x++)
    for(auto y = -10 ; y < 10 ; y++)
    {
         pou::SceneNode *grassNode = m_scene->getRootNode()->createChildNode(x*64,y*64);
         grassNode->attachObject(m_scene->createSpriteEntity(grassSheet->getSpriteModel("grass1")));
    }

    for(auto x = -3 ; x < 3 ; x++)
    {
        glm::vec2 p = glm::vec2(glm::linearRand(-640,640), glm::linearRand(-640,640));
        m_trees.push_back(new Character());
        m_trees.back()->loadModel("../data/grasslands/treeXML.txt");
        m_trees.back()->setPosition(p);
        m_trees.back()->rotate(glm::vec3(0,0,glm::linearRand(-180,180)));
        m_trees.back()->scale(glm::vec3(
                                glm::linearRand(0,10) > 5 ? 1 : -1,
                                glm::linearRand(0,10) > 5 ? 1 : -1,
                                1));
        float red = glm::linearRand(1.0,1.0);
        float green = glm::linearRand(0.9,1.0);
        float blue = green;//glm::linearRand(0.9,1.0);
        m_trees.back()->setColor(glm::vec4(red,green,blue,1));
        m_scene->getRootNode()->addChildNode(m_trees.back());
    }

    /*pou::SpriteSheetAsset *treeSheet = pou::SpriteSheetsHandler::loadAssetFromFile("../data/grasslands/treeXML.txt");

    for(auto x = -3 ; x < 3 ; x++)
    {
         glm::vec2 p = glm::vec2(glm::linearRand(-640,640), glm::linearRand(-640,640));
         pou::SceneNode *treeTrunkNode = m_scene->getRootNode()->createChildNode(p.x,p.y,40);
         treeTrunkNode->attachObject(m_scene->createSpriteEntity(treeSheet->getSpriteModel("treeTrunk")));
         pou::SceneNode *treeFoliage1Node = m_scene->getRootNode()->createChildNode(p.x,p.y,50);
         treeFoliage1Node->attachObject(m_scene->createSpriteEntity(treeSheet->getSpriteModel("treeFoliage1")));
         pou::SceneNode *treeFoliage2Node = m_scene->getRootNode()->createChildNode(p.x,p.y,60);
         treeFoliage2Node->attachObject(m_scene->createSpriteEntity(treeSheet->getSpriteModel("treeFoliage2")));
    }*/




     pou::SpriteSheetAsset *wallSpriteSheet = pou::SpriteSheetsHandler::loadAssetFromFile("../data/grasslands/wallSpriteXML.txt");
     pou::SceneNode *wallSpriteNode = m_scene->getRootNode()->createChildNode(-50,-100);
     wallSpriteNode->attachObject(m_scene->createSpriteEntity(wallSpriteSheet->getSpriteModel("wallSprite")));



    pou::MaterialAsset *wallMaterial = pou::MaterialsHandler::loadAssetFromFile("../data/wallXML.txt",loadType);
    pou::MeshAsset *wallBoxModel = pou::MeshesHandler::makeBox({0,0,0},{1,1,1},wallMaterial);
    pou::MeshEntity *wallBoxEntity;
    wallBoxEntity = m_scene->createMeshEntity(wallBoxModel);
    //m_shadowBoxEntity->setRmt({0.8,0.0,0.0});
    //wallBoxEntity->setColor({1.0,0.0,0.0,1.0});
    wallBoxEntity->setScale(glm::vec3(20,200,50));
    //m_shadowBoxEntity->setShadowCasting(vlg::ShadowCasting_All);
    //m_shadowBoxNode = m_scene->getRootNode()->createChildNode({0,0,0});
    m_scene->getRootNode()->createChildNode({100,0,0})->attachObject(wallBoxEntity);

    pou::MeshAsset* quackMesh = pou::MeshesHandler::loadAssetFromFile("../data/quackXML.txt",loadType);
    pou::MeshEntity *quackEntity = m_scene->createMeshEntity(quackMesh);

    pou::SceneNode* quackNode = m_scene->getRootNode()->createChildNode(50,200);
    quackNode->attachObject(quackEntity);
    quackNode->setScale(1.5f);

    //m_soundTest = pou::SoundsHandler::loadAssetFromFile("../data/hurtSoundXML.txt");
    //m_soundBankTest = pou::SoundBanksHandler::loadAssetFromFile("../data/char1/char1SoundBank.bank");


    m_camera = m_scene->createCamera();
    m_listeningCamera = m_scene->createCamera();
    m_listeningCamera -> setListening(true);
    //m_camera->setViewport({.2,.3},{.5,.4});
    //m_cameraNode = m_scene->getRootNode()->createChildNode(0,0,5);
    auto *cameraNode = m_character->createChildNode(0,0,-1);
    cameraNode->attachObject(m_camera);

    cameraNode = cameraNode->createChildNode(0,0,250);
    cameraNode->attachObject(m_listeningCamera);
    //m_character->attachObject(m_listeningCamera);

    //m_scene->setCurrentCamera(m_camera);


    m_scene->setAmbientLight({1.0,1.0,1.0,1.0});
    //m_sunLight = m_scene->createLightEntity(pou::LightType_Directional);
    //m_scene->getRootNode()->attachObject(m_sunLight);

    ///Day
   // m_sunLight->setDiffuseColor({1.0,1.0,1.0,1.0});
   // m_sunLight->setIntensity(20.0);

    ///Night
    //m_sunLight.setDiffuseColor({0.7,0.7,1.0,1.0});
    //m_sunLight.setIntensity(0.2);

    //sunLight->setType(pou::LightType_Directional);
    //m_sunLight.setDirection({-1.0,0.0,-1.0});
    /*m_sunLight->setDirection({.2 ,-1.0,-1.0});
    m_sunLight->setShadowMapExtent({1920,1080});
    m_sunLight->enableShadowCasting();*/

    /*pou::LightEntity* sunLight = m_scene->createLightEntity(pou::LightType_Directional);
    m_scene->getRootNode()->attachObject(sunLight);
    sunLight->setDirection({-1.0 ,1.0,-1.0});
    sunLight->setDiffuseColor({1.0,0.0,0.0,1.0});
    sunLight->setIntensity(15.0);
    sunLight->enableShadowCasting();*/

   /* pou::LightEntity *cursorLight = m_scene->createLightEntity();
    m_cursorLightNode = m_scene->getRootNode()->createChildNode(0,0,60);
    m_cursorLightNode->attachObject(cursorLight);
    cursorLight->setDiffuseColor({1.0,1.0,1.0,1.0});
    cursorLight->setIntensity(20.0);
    cursorLight->setRadius(400.0);
    cursorLight->setType(pou::LightType_Omni);*/

    /*for(size_t i = 3 ; i < 3 ; ++i)
    {
        pou::LightEntity *secLight = m_scene->createLightEntity();
        secLight->setDiffuseColor({glm::linearRand(0,100)/100.0,
                                    glm::linearRand(0,100)/100.0,
                                    glm::linearRand(0,100)/100.0});

        secLight->setIntensity(glm::linearRand(2.0,20.0));
        secLight->setRadius(glm::linearRand(100.0,600.0));
        m_cursorLightNode->createChildNode({glm::linearRand(-600,600),
                                            glm::linearRand(-600,600),
                                            glm::linearRand(-50,300)})->attachObject(secLight);
    }*/


  /*  for(size_t i = 0 ; i < 8 ; ++i)
    {
        //vec3(.4,0,.8)
        glm::vec3 v;
        v.x = glm::linearRand(-100,100)/100.0;
        v.y = glm::linearRand(-100,100)/100.0;
        v.z = glm::linearRand(0,100)/100.0;
        v = glm::normalize(v);
        v *= glm::linearRand(10,100)/100.0;

        std::ostringstream buf;
        buf<<"vec3("<<v.x<<","<<v.y<<","<<v.z<<"),";
        Logger::write(buf);

        v.x *= -1;
        v.y *= -1;

        std::ostringstream buf2;
        buf2<<"vec3("<<v.x<<","<<v.y<<","<<v.z<<"),";
        Logger::write(buf2);
    }*/




    m_mainInterface = new pou::UserInterface();

    m_lifeBar = m_mainInterface->createProgressBar(true);
    m_lifeBar->setPosition(17,0);
    m_lifeBar->setSize({288-34,32});
    m_lifeBar->setTextureRect({17,0},{288-34,32},false);
    m_lifeBar->setTexture(pou::TexturesHandler::loadAssetFromFile("../data/ui/Life_bar.png"));

    m_uiPictureTest = m_mainInterface->createUiPicture(false);
    m_uiPictureTest->setPosition(-17,0,-1);
    m_uiPictureTest->setSize(288,32);
    m_uiPictureTest->setTexture(pou::TexturesHandler::loadAssetFromFile("../data/ui/Life_bar_blank.png"));
    m_lifeBar->addChildNode(m_uiPictureTest);

    /*m_uiPictureTest = m_mainInterface->createUiPicture(false);
    m_uiPictureTest->setPosition(12,12,0);
    m_uiPictureTest->setSize(288,32);
    m_uiPictureTest->setTexture(pou::TexturesHandler::loadAssetFromFile("../data/ui/Life_bar.png"));
    m_lifeBar->setBarElement(m_uiPictureTest);*/


}

void TestingState::entered()
{
    m_totalTime = pou::TimeZero();

    if(m_firstEntering)
        this->init();
}

void TestingState::leaving()
{
    m_gameClient.disconnect();

    delete m_mainInterface;
    m_mainInterface = nullptr;

    delete m_scene;
    m_scene = nullptr;

    delete m_character;
    m_character = nullptr;

    delete m_character2;
    m_character2 = nullptr;

    delete m_croco;
    m_croco = nullptr;

    delete m_duck;
    m_duck = nullptr;

    for(auto duck : m_duckSwarm)
        delete duck;
    m_duckSwarm.clear();

    for(auto tree : m_trees)
        delete tree;
    m_trees.clear();
}

void TestingState::revealed()
{

}

void TestingState::obscuring()
{

}

void TestingState::handleEvents(const EventsManager *eventsManager)
{
    m_mainInterface->handleEvents(eventsManager);

    if(eventsManager->keyReleased(GLFW_KEY_ESCAPE))
        m_manager->stop();

    if(eventsManager->isAskingToClose())
        m_manager->stop();

    if(m_scene == nullptr)
        return;

    glm::vec2 worldMousePos = m_scene->convertScreenToWorldCoord(eventsManager->centeredMousePosition(), m_camera);

    //if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_1))
       // m_croco->setDestination(worldMousePos);
    if(m_activeCroc)
    {
        if(glm::length(m_croco->getGlobalXYPosition() - m_character->getGlobalXYPosition()) > 150.0f)
        {
            glm::vec2 nDist = glm::normalize(m_croco->getGlobalXYPosition() - m_character->getGlobalXYPosition());
            m_croco->setDestination(m_character->getGlobalXYPosition()+nDist*150.0f);
        }
    }

    if(m_activeDuck)
    {
        if(glm::length(m_duck->getGlobalXYPosition() - m_character->getGlobalXYPosition()) > 100.0f)
        {
            glm::vec2 nDist = glm::normalize(m_duck->getGlobalXYPosition() - m_character->getGlobalXYPosition());
            m_duck->setDestination(m_character->getGlobalXYPosition()+nDist*95.0f);
        } else if(m_character->isAlive())
            m_duck->attack(m_character->getGlobalXYPosition() - m_duck->getGlobalXYPosition());

        for(auto duck : m_duckSwarm)
        {
            if(glm::length(duck->getGlobalXYPosition() - m_character->getGlobalXYPosition()) > 100.0f)
            {
                glm::vec2 nDist = glm::normalize(duck->getGlobalXYPosition() - m_character->getGlobalXYPosition());
                duck->setDestination(m_character->getGlobalXYPosition()+nDist*95.0f);
            } else if(m_character->isAlive())
                duck->attack(m_character->getGlobalXYPosition() - duck->getGlobalXYPosition());
        }
    }


    if(eventsManager->keyPressed(GLFW_KEY_U))
    {
        if(m_duck->isAlive())
            m_activeDuck = !m_activeDuck;
        else
            m_duck->resurrect();
    }

    if(eventsManager->keyPressed(GLFW_KEY_C))
        m_activeCroc = !m_activeCroc;

    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_1))
        m_character->setLookingDirection(eventsManager->mousePosition()-glm::vec2(1024.0/2.0,768.0/2.0));
    if(eventsManager->mouseButtonIsPressed(GLFW_MOUSE_BUTTON_2))
        m_character->askToAttack(eventsManager->mousePosition()-glm::vec2(1024.0/2.0,768.0/2.0));

    if(eventsManager->keyIsPressed(GLFW_KEY_E))
        m_character2->setDestination(worldMousePos);
    if(eventsManager->keyIsPressed(GLFW_KEY_F))
        m_character2->attack();


    if(eventsManager->keyPressed(GLFW_KEY_G))
        pou::AudioEngine::playSound(m_soundTest);
    if(eventsManager->keyPressed(GLFW_KEY_H))
        pou::AudioEngine::playEvent(m_soundEventTest);

   // m_camVelocity = glm::vec3(0);

    /*if(eventsManager->keyIsPressed(GLFW_KEY_DOWN))
        m_camVelocity.y = 200.0;
    if(eventsManager->keyIsPressed(GLFW_KEY_UP))
        m_camVelocity.y = -200.0;
    if(eventsManager->keyIsPressed(GLFW_KEY_LEFT))
        m_camVelocity.x = -200.0;
    if(eventsManager->keyIsPressed(GLFW_KEY_RIGHT))
        m_camVelocity.x = 200.0;*/
   /* if(eventsManager->keyIsPressed(GLFW_KEY_PAGE_UP))
        m_camVelocity.z = -1.0;
    if(eventsManager->keyIsPressed(GLFW_KEY_PAGE_DOWN))
        m_camVelocity.z = 1.0;*/

    if(eventsManager->keyPressed(GLFW_KEY_1))
        m_character->loadItem("../data/char1/swordXML.txt");
    if(eventsManager->keyPressed(GLFW_KEY_2))
        m_character->loadItem("../data/char1/axeXML.txt");

    glm::vec2 charDirection = {0,0};

    if(eventsManager->keyIsPressed(GLFW_KEY_S))
        charDirection.y = 1;
    if(eventsManager->keyIsPressed(GLFW_KEY_W))
        charDirection.y = -1;
    if(eventsManager->keyIsPressed(GLFW_KEY_A))
        charDirection.x = -1;
    if(eventsManager->keyIsPressed(GLFW_KEY_D))
        charDirection.x = 1;

    if(eventsManager->keyPressed(GLFW_KEY_LEFT_ALT))
        m_character->askToDash(charDirection);

    m_character->walk(charDirection);

    m_character->lookAt(worldMousePos);
    if(eventsManager->keyIsPressed(GLFW_KEY_LEFT_SHIFT))
        m_character->forceAttackMode();

    /*if(eventsManager->keyIsPressed(GLFW_KEY_SPACE))
        m_croco->walk({0,-1});
    else
        m_croco->walk({0,0});*/
    if(eventsManager->keyIsPressed(GLFW_KEY_SPACE))
    {
        //m_character->startAnimation("attack",true);
        m_character->askToAttack();
    }
}

void TestingState::update(const pou::Time &elapsedTime)
{
    m_totalTime += elapsedTime;
    m_nbrFps++;

    /*glm::vec2 camMoveXY = m_scene->convertScreenToWorldCoord(glm::vec2(m_camVelocity.x,m_camVelocity.y));
    glm::vec3 camMove = glm::vec3(camMoveXY.x, camMoveXY.y, m_camVelocity.z);
    camMove.x *= elapsedTime.count();
    camMove.y *= elapsedTime.count();
    camMove.z *= elapsedTime.count();
    m_cameraNode->move(camMove);*/

    //m_cameraNode->move(glm::vec3(0,0,elapsedTime.count()));

    m_lifeBar->setMinMaxValue(0,m_character->getAttributes().maxLife);
    m_lifeBar->setValue(m_character->getAttributes().life);

    m_character->addToNearbyCharacters(m_duck);
    m_duck->addToNearbyCharacters(m_character);

    for(auto duck : m_duckSwarm)
    {
        m_character->addToNearbyCharacters(duck);
        duck->addToNearbyCharacters(m_character);
    }

    for(auto tree : m_trees)
        m_character->addToNearbyCharacters(tree);




    m_gameServer.update(elapsedTime);
    m_gameClient.update(elapsedTime);




    pou::Time remainingTime = elapsedTime;
    pou::Time maxTickTime(.03);

    while(remainingTime > maxTickTime)
    {
        m_mainInterface->update(maxTickTime);
        m_scene->update(maxTickTime);
        remainingTime -= maxTickTime;
    }
    m_mainInterface->update(remainingTime);
    m_scene->update(remainingTime);

    if(m_totalTime.count() > 1)
    {
        m_totalTime -= std::chrono::seconds(1);
        std::cout<<"FPS : "<<m_nbrFps<<std::endl;
        m_nbrFps = 0;
    }
}

void TestingState::draw(pou::RenderWindow *renderWindow)
{
    if(renderWindow->getRenderer(pou::Renderer_Scene) != nullptr)
    {
        pou::SceneRenderer *renderer = dynamic_cast<pou::SceneRenderer*>(renderWindow->getRenderer(pou::Renderer_Scene));
        m_scene->render(renderer, m_camera);
    }

    if(renderWindow->getRenderer(pou::Renderer_Ui) != nullptr)
    {
        pou::UiRenderer *renderer = dynamic_cast<pou::UiRenderer*>(renderWindow->getRenderer(pou::Renderer_Ui));
        m_mainInterface->render(renderer);
    }

}



