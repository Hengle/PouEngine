#include "world/GameWorld.h"

#include "PouEngine/Types.h"
#include "PouEngine/assets/AssetHandler.h"
#include "PouEngine/assets/SpriteSheetAsset.h"
#include "PouEngine/assets/MeshesHandler.h"
#include "PouEngine/renderers/SceneRenderer.h"

//For tests
#include "PouEngine/system/Clock.h"
#include "PouEngine/scene/CollisionObject.h"
#include "generators/PoissonDiskSampler.h"

#include "net/GameServer.h"
#include "net/GameClient.h"
#include "logic/GameMessageTypes.h"
#include "world/WorldMesh.h"
#include "assets/PrefabAsset.h"


void GameWorld::init()
{
    this->destroy();
    this->createScene();

    m_worldRootNode = std::make_shared<WorldNode>();
    m_scene->getRootNode()->addChildNode(m_worldRootNode);
    m_syncComponent.syncElement(m_worldRootNode);

    m_worldGrid = std::make_shared<WorldGrid>();
    m_worldGrid->setQuadSize(1024);
    m_worldGrid->resizeQuad(glm::ivec2(-100), glm::ivec2(200));
    m_scene->getRootNode()->addChildNode(m_worldGrid);
    m_syncComponent.syncElement(m_worldGrid);
}

void GameWorld::generate(bool generateInThread)
{
    ///this->createScene();
    this->init();

    m_worldReady = false;

    if(generateInThread)
        m_generatingThread = std::thread(&GameWorld::generateImpl, this);
    else
        this->generateImpl();
}


void GameWorld::generateImpl()
{
    pou::Logger::write("Generating world...");

    m_dayTime = pou::RNGesus::uniformInt(0,360);

    auto loadType = pou::LoadType_Now;

    auto worldSeed = pou::RNGesus::rand();
    m_worldGenerator.loadFromFile("../data/grasslands/grasslandsBiomeXML.txt");
    m_worldGenerator.generatesOnNode(m_worldGrid.get(), worldSeed, &m_syncComponent);


    auto airBalloonModel = CharacterModelsHandler::loadAssetFromFile("../data/airBalloon/airBalloonXML.txt",loadType);
    m_syncComponent.syncElement(airBalloonModel);
    {
        auto airBalloon = std::make_shared<Character>();
        airBalloon->createFromModel(airBalloonModel);
        //airBalloon->pou::SceneNode::setPosition(0);
        //airBalloon->rotate(glm::vec3(0,0,pou::RNGesus::uniformInt(-2,2)*90),false);
        m_worldGrid->addChildNode(airBalloon);
        m_syncComponent.syncElement(airBalloon);
    }



    /*auto treeModel = CharacterModelsHandler::loadAssetFromFile("../data/grasslands/treeXML.txt",loadType);
    m_syncComponent.syncElement(treeModel);

    for(auto x = -10 ; x < 10 ; x++)
    for(auto y = -10 ; y < 10 ; y++)
    {
        glm::vec3 p = glm::vec3(pou::RNGesus::uniformFloat(-640.0f,640.0f),
                                pou::RNGesus::uniformFloat(-640.0f,640.0f),
                                pou::RNGesus::uniformFloat(0.0f,0.1f));

        p += glm::vec3(x*640, y*640,0);

        auto tree = std::make_shared<Character>();

        tree->createFromModel(treeModel);
        tree->setPosition(p);
        tree->rotate(glm::vec3(0,0,pou::RNGesus::uniformFloat(-180,180)));
        tree->scale(glm::vec3(
                    pou::RNGesus::uniformFloat(0,10) > 5 ? 1 : -1,
                    pou::RNGesus::uniformFloat(0,10) > 5 ? 1 : -1,
                    1));
        float red = pou::RNGesus::uniformFloat(1.0,1.0);
        float green = pou::RNGesus::uniformFloat(0.8,1.0);
        float blue = green;//pou::RNGesus::uniformFloat(0.9,1.0);
        tree->setColor(glm::vec4(red,green,blue,1));
        ///m_scene->getRootNode()->addChildNode(tree);
        m_worldGrid->addChildNode(tree);

        m_syncComponent.syncElement(tree);
    }*/

    /**auto lanternModel = CharacterModelsHandler::loadAssetFromFile("../data/poleWithLantern/poleWithLanternXML.txt",loadType);
    m_syncComponent.syncElement(lanternModel);

    PoissonDiskSampler sampler;
    auto pointDistribution = sampler.generateDistribution(glm::vec2(10000,10000), 64);
    std::cout<<"NBR LANTERNS:"<<pointDistribution.size()<<std::endl;
    for(auto pointIt : pointDistribution)
    {
        glm::vec2 p = pointIt;

        auto lantern = std::make_shared<Character>();
        lantern->createFromModel(lanternModel);
        lantern->pou::SceneNode::setPosition(p);
        //lantern->rotate(glm::vec3(0,0,pou::RNGesus::uniformFloat(-180,180)));
        m_worldGrid->addChildNode(lantern);

        m_syncComponent.syncElement(lantern);
    }**/

    /*for(auto x = -10 ; x < 10 ; x++)
    for(auto y = -10 ; y < 10 ; y++)
    //for(auto x = -1 ; x < 0 ; x++)
    //for(auto y = -1 ; y < 0 ; y++)
    {
        glm::vec2 p = glm::vec2(pou::RNGesus::uniformFloat(-640,640), pou::RNGesus::uniformFloat(-640,640));

        p += glm::vec2(x*640, y*640);

        auto lantern = std::make_shared<Character>();
        lantern->createFromModel(lanternModel);
        lantern->pou::SceneNode::setPosition(p);
        lantern->rotate(glm::vec3(0,0,pou::RNGesus::uniformFloat(-180,180)));
        ///m_scene->getRootNode()->addChildNode(lantern);
        m_worldGrid->addChildNode(lantern);

        m_syncComponent.syncElement(lantern);
    }*/

    /**auto duckModel = CharacterModelsHandler::loadAssetFromFile("../data/duck/duckXML.txt",loadType);
    m_syncComponent.syncElement(duckModel);

    for(auto x = -10 ; x < 10 ; x++)
    for(auto y = -10 ; y < 10 ; y++)
    {
        glm::vec2 p, pp;

        pp = glm::vec2(x*640, y*640);

        do
        {
            p = pp + glm::vec2(pou::RNGesus::uniformFloat(-640,640), pou::RNGesus::uniformFloat(-640,640));
        }while(glm::length(p) < 500.0f);

        //p = glm::vec2(x*300, y*300);


        auto duck = std::make_shared<Character>();
        duck->createFromModel(duckModel);
        duck->pou::SceneNode::setPosition(p);
        ///m_scene->getRootNode()->addChildNode(duck);
        m_worldGrid->addChildNode(duck);

        m_syncComponent.syncElement(duck);
    }

    auto crocoModel = CharacterModelsHandler::loadAssetFromFile("../data/croco/crocoXML.txt",loadType);
    m_syncComponent.syncElement(crocoModel);
    {
        glm::vec2 p, pp;

        pp = glm::vec2(1000, 1000);

        do
        {
            p = pp + glm::vec2(pou::RNGesus::uniformFloat(-640,640), pou::RNGesus::uniformFloat(-640,640));
        }while(glm::length(p) < 500.0f);

        auto croco = std::make_shared<Character>();
        croco->createFromModel(crocoModel);
        croco->setPosition(p);
        m_worldGrid->addChildNode(croco);

        m_syncComponent.syncElement(croco);
    }


    auto prefabHouseModel = PrefabsHandler::loadAssetFromFile("../data/wall/smallHousePrefabXML.txt");
    m_syncComponent.syncElement(prefabHouseModel);
    {
        auto houseNode = prefabHouseModel->generate();
        houseNode->move(512,512);
        m_worldGrid->addChildNode(houseNode);
        m_syncComponent.syncElement(houseNode);
    }**/

    /**auto barrelModel = CharacterModelsHandler::loadAssetFromFile("../data/furnitures/barrel1XML.txt",loadType);
    m_syncComponent.syncElement(barrelModel);
    {
        glm::vec3 p(100,0,60);

        auto barrel = std::make_shared<Character>();
        barrel->createFromModel(barrelModel);
        barrel->setPosition(p);
         m_worldGrid->addChildNode(barrel);

        m_syncComponent.syncElement(barrel);
    }

    auto prefabWall = PrefabsHandler::loadAssetFromFile("../data/wall/wallWithCollisionPrefabXML.txt");
    m_syncComponent.syncElement(prefabWall);

    auto wallNode = prefabWall->generate();
    m_worldGrid->addChildNode(wallNode);
    m_syncComponent.syncElement(wallNode);

    wallNode = prefabWall->generate();
    wallNode->move(128,0);
    m_worldGrid->addChildNode(wallNode);
    m_syncComponent.syncElement(wallNode);


    wallNode = prefabWall->generate();
    wallNode->move(128,128);
    m_worldGrid->addChildNode(wallNode);
    m_syncComponent.syncElement(wallNode);


    wallNode = prefabWall->generate();
    m_worldGrid->addChildNode(wallNode);
    wallNode->rotate(glm::vec3(0,0,90),false);
    wallNode->move(128,64);
    m_syncComponent.syncElement(wallNode);**/

    m_scene->update(pou::Time(0));
    m_worldReady = true;

    pou::Logger::write("World generated !");
}

void GameWorld::createWorldInitializationMsg(std::shared_ptr<NetMessage_WorldInit> worldInitMsg)
{
    //worldInitMsg->localTime = m_curLocalTime;
    worldInitMsg->dayTime = (int)m_dayTime;

    worldInitMsg->worldGeneratorModel = m_worldGenerator.getFilePath();
    worldInitMsg->worldGeneratorSeed  = m_worldGenerator.getGeneratingSeed();

    //worldInitMsg->worldGrid_nodeId = (int)m_syncNodes.findId(m_worldGrid);
    m_syncComponent.createWorldSyncMsg(worldInitMsg, worldInitMsg->player_id, -1);
}

void GameWorld::generateFromMsg(std::shared_ptr<NetMessage_WorldInit> worldInitMsg, bool useLockStepMode, bool generateInThread)
{
    this->init();
    m_worldReady = false;

    if(generateInThread)
        m_generatingThread = std::thread(&GameWorld::generateFromMsgImpl, this, worldInitMsg, useLockStepMode);
    else
        this->generateFromMsgImpl(worldInitMsg, useLockStepMode);
}

void GameWorld::generateFromMsgImpl(std::shared_ptr<NetMessage_WorldInit> worldInitMsg, bool useLockStepMode)
{
    m_dayTime = worldInitMsg->dayTime;

    m_worldGenerator.loadFromFile(worldInitMsg->worldGeneratorModel);
    m_worldGenerator.generatesOnNode(m_worldGrid.get(), worldInitMsg->worldGeneratorSeed, &m_syncComponent, false);

    m_syncComponent.syncWorldFromMsg(worldInitMsg, worldInitMsg->player_id,0,useLockStepMode);
    ///m_curLocalTime = m_syncComponent.getLastSyncTime();

    auto player = m_syncComponent.getPlayer(worldInitMsg->player_id);
    this->createPlayerCamera(player.get());
    ///m_worldGrid->addUpdateProbe(player.get()/*->node()*/, 2048);

    m_scene->update(pou::Time(0),m_syncComponent.getLocalTime());

    m_worldReady = true;

    pou::Logger::write("World generated !");
}

bool GameWorld::initPlayer(size_t player_id, std::shared_ptr<PlayerSave> playerSave)
{
    auto player = m_syncComponent.getPlayer(player_id);
    if(!player)
        return (false);

    //pou::Clock clock;
    //clock.restart();

    ///player->update(pou::Time(0), 0);

    glm::vec2 pos(pou::RNGesus::uniformFloat(-200,200), pou::RNGesus::uniformFloat(-200,200));
    player->pou::SceneNode::setPosition(pos);

    player->update(pou::Time(0), m_syncComponent.getLocalTime());

    CharacterModelAsset *playerModel(nullptr);
    if(playerSave->getPlayerType() == 0)
        playerModel = CharacterModelsHandler::loadAssetFromFile("../data/player/player_1_XML.txt"/*, pou::LoadType_InThread*/);
    else if(playerSave->getPlayerType() == 1)
        playerModel = CharacterModelsHandler::loadAssetFromFile("../data/player/player_mokou_XML.txt");
    else if(playerSave->getPlayerType() == 2)
        playerModel = CharacterModelsHandler::loadAssetFromFile("../data/player/player_sith_XML.txt");
    else if(playerSave->getPlayerType()  == 3)
        playerModel = CharacterModelsHandler::loadAssetFromFile("../data/player/player_leather_XML.txt");
    else if(playerSave->getPlayerType()  == 4)
        playerModel = CharacterModelsHandler::loadAssetFromFile("../data/player/player_bone_XML.txt");

    m_syncComponent.syncElement(playerModel);
    player->setModel(playerModel);

    playerSave->loadToPlayer(player.get());

    m_worldGrid->addChildNode(player/*->node()*/);
    //m_scene->getRootNode()->addChildNode(player/*->node()*/);

    ///player/*->node()*/->pou::SceneNode::setPosition(pos);

    ItemModelAsset *playerWeapon;

    playerWeapon = ItemModelsHandler::loadAssetFromFile("../data/player/swordXML.txt");
    m_syncComponent.syncElement(playerWeapon);
    player->addItemToInventory(playerWeapon,1);

    playerWeapon = ItemModelsHandler::loadAssetFromFile("../data/player/axeXML.txt");
    m_syncComponent.syncElement(playerWeapon);
    player->addItemToInventory(playerWeapon,2);

    playerWeapon = ItemModelsHandler::loadAssetFromFile("../data/player/mokouSwordXML.txt");
    m_syncComponent.syncElement(playerWeapon);
    player->addItemToInventory(playerWeapon,3);

    playerWeapon = ItemModelsHandler::loadAssetFromFile("../data/player/laserSwordXML.txt");
    m_syncComponent.syncElement(playerWeapon);
    player->addItemToInventory(playerWeapon,4);

    playerWeapon = ItemModelsHandler::loadAssetFromFile("../data/player/energySwordXML.txt");
    m_syncComponent.syncElement(playerWeapon);
    player->addItemToInventory(playerWeapon,5);

    player->setTeam(1);

    ///m_worldGrid->addUpdateProbe(player.get()/*->node()*/, 2048);

    GameMessage_World_NewPlayer gameMsg;
    gameMsg.player = player.get();
    pou::MessageBus::postMessage(GameMessageType_World_NewPlayer, &gameMsg);

    return (true);
}

bool GameWorld::removePlayer(size_t player_id)
{
    auto player = m_syncComponent.getPlayer(player_id);

    if(!player)
        return (false);

     /**for(auto it = m_syncCharacters.begin() ; it != m_syncCharacters.end() ; ++it)
            it->second->removeFromNearbyCharacters(player);**/

    //m_worldGrid->removeChildNode(player/*->node()*/);
    player->removeFromParent();
    m_worldGrid->removeUpdateProbe(player/*->node()*/.get());
    m_syncComponent.desyncElement(player.get());

    ///return m_syncPlayers.freeId(player_id);
    return (true);
}



