#include "generators/WorldGenerator_SpawnPoint.h"

#include "PouEngine/assets/AssetHandler.h"
#include "assets/CharacterModelAsset.h"
#include "generators/WorldGenerator.h"

#include "PouEngine/tools/Parser.h"
#include "PouEngine/tools/Logger.h"


WorldGenerator_SpawnPoint_Modifier::WorldGenerator_SpawnPoint_Modifier()
{
    randomType = WorldGenerator_RandomType_None;
    minValue = glm::vec4(0);
    maxValue = glm::vec4(0);
    for(int i = 0 ; i < 4 ; ++i)
        usePrecedingValue[i] = -1;
}


///
///WorldGenerator_SpawnPoint_Parameters
///

WorldGenerator_SpawnPoint_Parameters::WorldGenerator_SpawnPoint_Parameters() :
    spawnProbability(1.0f),
    unique(false),
    //reventOtherSpawns(false),
    groundLayer(nullptr),
    changeSpawnTypeTo(false),
    newSpawnType(TerrainGenerator_SpawnType_None),
    changeGroundLayerTo(false),
    newGroundLayer(nullptr),
    preventGroundSpawning(true)
{

}


///
///WorldGenerator_SpawnPoint_PathConnection
///

WorldGenerator_SpawnPoint_PathConnection::WorldGenerator_SpawnPoint_PathConnection() :
    spawnProbability(1.0f),
    pathName(0),
    position(0)
{

}

///
///WorldGenerator_SpawnPoint_SubSpawn
///

WorldGenerator_SpawnPoint_SubSpawn::WorldGenerator_SpawnPoint_SubSpawn() :
    spawnProbability(1.0f),
    spawnGroupName(0),
    position(0)
{

}

///
///WorldGenerator_SpawnPoint_Ground
///

WorldGenerator_SpawnPoint_Ground::WorldGenerator_SpawnPoint_Ground() :
    position(0),
    groundLayer(nullptr),
    preventGroundSpawning(true)
{

}

///
///WorldGenerator_SpawnPoint
///

WorldGenerator_SpawnPoint::WorldGenerator_SpawnPoint() :
    m_gridSize(1),
    m_radius(-1),
    m_hasSpawn(false)
{
    //ctor
}

WorldGenerator_SpawnPoint::~WorldGenerator_SpawnPoint()
{
    //dtor
}

bool WorldGenerator_SpawnPoint::loadFromXML(const std::string &fileDirectory, TiXmlElement *element, WorldGenerator *worldGenerator)
{
    bool r = true;

    m_worldGenerator = worldGenerator;
    m_fileDirectory = fileDirectory;

    if(!this->loadParameters(element, m_parameters))
        r = false;

    auto sizeChild = element->FirstChildElement("size");
    if(sizeChild)
    {
        auto sizeElement = sizeChild->ToElement();
        if(sizeElement != nullptr)
        {
            if(sizeElement->Attribute("x") != nullptr)
                m_gridSize.x = pou::Parser::parseInt(std::string(sizeElement->Attribute("x")));
            if(sizeElement->Attribute("y") != nullptr)
                m_gridSize.y = pou::Parser::parseInt(std::string(sizeElement->Attribute("y")));
            if(sizeElement->Attribute("radius") != nullptr)
                m_radius = pou::Parser::parseInt(std::string(sizeElement->Attribute("radius")));
        }
    }

    if(m_radius > 0)
        m_gridSize = glm::ivec2(glm::ceil(m_radius*2));

    auto modifierChild = element->FirstChildElement("modifier");
    while(modifierChild != nullptr)
    {
        this->loadModifier(modifierChild->ToElement());
        modifierChild = modifierChild->NextSiblingElement("modifier");
    }

    auto characterChild = element->FirstChildElement("character");
    while(characterChild != nullptr)
    {
        this->loadCharacter(characterChild->ToElement());
        characterChild = characterChild->NextSiblingElement("character");
    }

    auto spriteChild = element->FirstChildElement("sprite");
    while(spriteChild != nullptr)
    {
        this->loadSprite(spriteChild->ToElement());
        spriteChild = spriteChild->NextSiblingElement("sprite");
    }

    auto prefabChild = element->FirstChildElement("prefab");
    while(prefabChild != nullptr)
    {
        this->loadPrefab(prefabChild->ToElement());
        prefabChild = prefabChild->NextSiblingElement("prefab");
    }

    auto pathChild = element->FirstChildElement("pathConnection");
    while(pathChild != nullptr)
    {
        this->loadPathConnection(pathChild->ToElement());
        pathChild = pathChild->NextSiblingElement("pathConnection");
    }

    auto subSpawnChild = element->FirstChildElement("subSpawn");
    while(subSpawnChild != nullptr)
    {
        this->loadSubSpawnPoint(subSpawnChild->ToElement());
        subSpawnChild = subSpawnChild->NextSiblingElement("subSpawn");
    }

    auto groundChild = element->FirstChildElement("ground");
    while(groundChild != nullptr)
    {
        this->loadGroundElement(groundChild->ToElement());
        groundChild = groundChild->NextSiblingElement("ground");
    }

    auto drawGroundChild = element->FirstChildElement("drawGround");
    if(drawGroundChild)
    {
        this->loadDrawGroundElement(drawGroundChild->ToElement());
        drawGroundChild = drawGroundChild->NextSiblingElement("drawGround");
    }
       // std::cout<<drawGroundChild->ToElement()->GetText()<std::endl;

    return r;
}

void WorldGenerator_SpawnPoint::generatesOnNode(glm::vec2 worldPos, float pointRotation,
                                                pou::SceneNode *targetNode, GameWorld_Sync *syncComponent,
                                                bool generateCharacters, pou::RNGenerator *rng)
{
    auto gridShift = this->generateRandomValue(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_GridPosition], rng);
    worldPos += glm::vec2(gridShift) * m_worldGenerator->terrain()->getTileSize();

    if(abs(worldPos.x) + m_gridSize.x*0.5 > m_worldGenerator->terrain()->getExtent().x*0.5
    || abs(worldPos.y) + m_gridSize.y*0.5 > m_worldGenerator->terrain()->getExtent().y*0.5)
        return;

    m_hasSpawn = true;

    for(auto characterSpawnModel : m_characterSpawnModels)
    {
        int amount = pou::RNGesus::uniformInt(characterSpawnModel.minAmount,
                                              characterSpawnModel.maxAmount, rng);

        for(int i = 0 ; i < amount ; ++i)
            this->spawnCharacter(characterSpawnModel.modelAsset, worldPos, pointRotation,
                                 targetNode, syncComponent, generateCharacters, rng);
    }

    for(auto spriteModel : m_spriteModelAssets)
        this->spawnSprite(spriteModel, worldPos, pointRotation, targetNode, rng);

    for(auto prefabModel : m_prefabAssets)
        this->spawnPrefab(prefabModel, worldPos, pointRotation,
                          targetNode, syncComponent, generateCharacters, rng);

    for(auto &pathConnection : m_pathConnections)
        this->spawnPathConnection(pathConnection, worldPos, rng);

    for(auto &subSpawn : m_subSpawnPoints)
        this->spawnSubSpawn(subSpawn, worldPos, rng);

    auto startingPos = worldPos - glm::vec2(m_gridSize) * 0.5f * m_worldGenerator->terrain()->getTileSize();

    for(auto groundElement : m_groundElements)
    {
        auto worldElementPos = startingPos + glm::vec2(groundElement.position) * m_worldGenerator->terrain()->getTileSize();
        m_worldGenerator->terrain()->setGroundLayer(worldElementPos, groundElement.groundLayer, groundElement.preventGroundSpawning);
    }

    //if(m_parameters.changeSpawnTypeTo || m_parameters.changeGroundLayerTo || !m_pathConnections.empty())
    {
        for(auto y = 0 ; y < m_gridSize.y ; ++y)
        for(auto x = 0 ; x < m_gridSize.x ; ++x)
        {
            if(m_radius > 0 && glm::length(glm::vec2(x,y) + glm::vec2(.5) - glm::vec2(m_gridSize) * 0.5f) > m_radius)
                continue;

            auto gridElementPos = startingPos + glm::vec2(x,y) * m_worldGenerator->terrain()->getTileSize();

            if(m_parameters.changeSpawnTypeTo)
                m_worldGenerator->terrain()->setSpawnType(gridElementPos, m_parameters.newSpawnType);
            if(m_parameters.changeGroundLayerTo)
                m_worldGenerator->terrain()->setGroundLayer(gridElementPos, m_parameters.newGroundLayer,
                                                            m_parameters.preventGroundSpawning);

            for(auto &pathConnection : m_pathConnections)
                m_worldGenerator->terrain()->addPathType(gridElementPos, pathConnection.pathName);

            if(m_pathConnections.empty())
                m_worldGenerator->terrain()->addPathType(gridElementPos, 0);
        }
    }
}

float WorldGenerator_SpawnPoint::getSpawnProbability(glm::vec2 worldPos)
{
    if(m_parameters.unique && m_hasSpawn)
        return (0);

    auto gridPos = m_worldGenerator->terrain()->worldToGridPosition(worldPos);
    if(m_parameters.groundLayer && m_worldGenerator->terrain()->getGridGroundLayer(gridPos.x, gridPos.y) != m_parameters.groundLayer)
        return (0);

    return m_parameters.spawnProbability;
}

/*bool WorldGenerator_SpawnPoint::preventOtherSpawns()
{
    return m_parameters.preventOtherSpawns;
}*/

///
///Protected
///


bool WorldGenerator_SpawnPoint::loadParameters(TiXmlElement *element, WorldGenerator_SpawnPoint_Parameters &parameters)
{
    auto groundLayerAtt = element->Attribute("groundLayer");
    auto spawnProbabilityAtt = element->Attribute("spawnProbability");
    auto uniqueAtt = element->Attribute("unique");
    //auto preventSpawnsAtt = element->Attribute("preventOtherSpawns");
    auto changeSpawnTypeAtt = element->Attribute("changeSpawnTypeTo");
    auto changeGroundLayerAtt = element->Attribute("changeGroundLayerTo");


    auto hashedGroundLayerName = pou::Hasher::unique_hash(groundLayerAtt);
    if(groundLayerAtt)
        parameters.groundLayer = m_worldGenerator->terrain()->getGroundLayer(hashedGroundLayerName);

    if(spawnProbabilityAtt)
    {
        if(std::string("inf") == spawnProbabilityAtt)
            parameters.spawnProbability = -1;
        else if(pou::Parser::isFloat(spawnProbabilityAtt))
            parameters.spawnProbability = pou::Parser::parseFloat(spawnProbabilityAtt);
    }

    if(uniqueAtt && pou::Parser::isBool(uniqueAtt))
        parameters.unique = uniqueAtt;

    //if(preventSpawnsAtt)
      //  parameters.preventOtherSpawns = pou::Parser::parseBool(preventSpawnsAtt);

    if(changeSpawnTypeAtt)
    {
        parameters.changeSpawnTypeTo = true;
        auto changeSpawnTypeStr = std::string(changeSpawnTypeAtt);
        if(changeSpawnTypeStr == "all")
            parameters.newSpawnType = TerrainGenerator_SpawnType_All;
        else if(changeSpawnTypeStr == "safe")
            parameters.newSpawnType = TerrainGenerator_SpawnType_Safe;
        else if(changeSpawnTypeStr == "none")
            parameters.newSpawnType = TerrainGenerator_SpawnType_None;
        else
            parameters.changeSpawnTypeTo = false;
    }


    if(changeGroundLayerAtt)
    {
        auto hashedChangeGroundLayer = pou::Hasher::unique_hash(changeGroundLayerAtt);
        parameters.changeGroundLayerTo = true;
        parameters.newGroundLayer = m_worldGenerator->terrain()->getGroundLayer(hashedChangeGroundLayer);
    }

    pou::XMLLoader::loadBool(parameters.preventGroundSpawning, element, "preventGroundSpawning");

    return (true);
}

void WorldGenerator_SpawnPoint::loadModifier(TiXmlElement *modifierElement)
{
    auto typeAtt = modifierElement->Attribute("type");
    if(typeAtt)
    {
        int type = 0;
        if(std::string(typeAtt) == "gridPosition")
            type = WorldGenerator_SpawnPoint_ModifierType_GridPosition;
        else if(std::string(typeAtt) == "position")
            type = WorldGenerator_SpawnPoint_ModifierType_Position;
        else if(std::string(typeAtt) == "rotation")
            type = WorldGenerator_SpawnPoint_ModifierType_Rotation;
        else if(std::string(typeAtt) == "flip")
            type = WorldGenerator_SpawnPoint_ModifierType_Flip;
        else if(std::string(typeAtt) == "color")
            type = WorldGenerator_SpawnPoint_ModifierType_Color;

        auto &randomModifier = m_randomModifiers[type];

        auto rngAtt = modifierElement->Attribute("rng");
        if(rngAtt)
        {
            if(std::string(rngAtt) == "uniform")
                randomModifier.randomType = WorldGenerator_RandomType_Uniform;
            else if(std::string(rngAtt) == "gaussian")
                randomModifier.randomType = WorldGenerator_RandomType_Gaussian;
        }

        for(int i = 0 ; i < 4 ; ++i)
            this->loadRandomModifierValue(modifierElement, i, randomModifier);
    }
}

void WorldGenerator_SpawnPoint::loadCharacter(TiXmlElement *characterElement)
{
    auto pathAtt = characterElement->Attribute("path");
    auto amountAtt = characterElement->Attribute("amount");

    if(!pathAtt)
        return;

    auto modelAsset = CharacterModelsHandler::loadAssetFromFile(m_fileDirectory+pathAtt);
    if(!modelAsset)
        return;

    m_characterSpawnModels.emplace_back();
    auto &characterSpawnModel = m_characterSpawnModels.back();
    characterSpawnModel.modelAsset = modelAsset;

    if(amountAtt)
    {
        auto [minV, maxV] = pou::Parser::parseIntSegment(amountAtt);
        characterSpawnModel.minAmount = minV;
        characterSpawnModel.maxAmount = maxV;
    }
}

void WorldGenerator_SpawnPoint::loadSprite(TiXmlElement *spriteElement)
{
    auto spriteSheetAtt = spriteElement->Attribute("spritesheet");
    auto spriteAtt = spriteElement->Attribute("sprite");

    if(!spriteSheetAtt || !spriteAtt)
        return;

    auto spriteSheet = pou::SpriteSheetsHandler::loadAssetFromFile(m_fileDirectory+spriteSheetAtt);
    if(!spriteSheet)
        return;

    auto hashedSpriteName = pou::Hasher::unique_hash(spriteAtt);
    auto spriteModel = spriteSheet->getSpriteModel(hashedSpriteName);
    if(!spriteModel)
        return;

    m_spriteModelAssets.push_back(spriteModel);
}

void WorldGenerator_SpawnPoint::loadPrefab(TiXmlElement *prefabElement)
{
    auto pathAtt = prefabElement->Attribute("path");

    if(!pathAtt)
        return;

    auto modelAsset = PrefabsHandler::loadAssetFromFile(m_fileDirectory+pathAtt);
    if(!modelAsset)
        return;

    m_prefabAssets.push_back(modelAsset);
}

void WorldGenerator_SpawnPoint::loadPathConnection(TiXmlElement *pathElement)
{
    auto pathAtt = pathElement->Attribute("path");

    if(!pathAtt)
        return;

    auto hashedPathName = pou::Hasher::unique_hash(pathAtt);
    auto &pathConnection = m_pathConnections.emplace_back();
    pathConnection.pathName = hashedPathName;

    pou::XMLLoader::loadFloat(pathConnection.position.x, pathElement, "x");
    pou::XMLLoader::loadFloat(pathConnection.position.y, pathElement, "y");

    pou::XMLLoader::loadFloat(pathConnection.spawnProbability, pathElement, "spawnProbability");
}

void WorldGenerator_SpawnPoint::loadSubSpawnPoint(TiXmlElement *subSpawnElement)
{
    auto spawnGroupAtt = subSpawnElement->Attribute("spawnGroup");

    if(!spawnGroupAtt)
        return;

    auto hashedSpawnGroup = pou::Hasher::unique_hash(spawnGroupAtt);
    auto &subSpawn = m_subSpawnPoints.emplace_back();
    subSpawn.spawnGroupName = hashedSpawnGroup;

    pou::XMLLoader::loadFloat(subSpawn.position.x, subSpawnElement, "x");
    pou::XMLLoader::loadFloat(subSpawn.position.y, subSpawnElement, "y");

    pou::XMLLoader::loadFloat(subSpawn.spawnProbability, subSpawnElement, "spawnProbability");
}


void WorldGenerator_SpawnPoint::loadGroundElement(TiXmlElement *pathElement)
{
    auto groundLayerAtt = pathElement->Attribute("groundLayer");

    if(!groundLayerAtt)
        return;

    auto hashedGroundName = pou::Hasher::unique_hash(groundLayerAtt);
    auto &groundElement = m_groundElements.emplace_back();
    groundElement.groundLayer = m_worldGenerator->terrain()->getGroundLayer(hashedGroundName);

    pou::XMLLoader::loadInt(groundElement.position.x, pathElement, "x");
    pou::XMLLoader::loadInt(groundElement.position.y, pathElement, "y");
}

void WorldGenerator_SpawnPoint::loadDrawGroundElement(TiXmlElement *pathElement)
{
    auto groundLayerAtt = pathElement->Attribute("groundLayer");

    if(!groundLayerAtt)
        return;

    auto hashedGroundName = pou::Hasher::unique_hash(groundLayerAtt);

    auto bmpStr = std::string(pathElement->GetText());
    int t = 0;
    for(auto c : bmpStr)
    {
        if(c == '0')
            t++;
        if(c == '1')
        {
            glm::ivec2 pos(t % m_gridSize.x, t / m_gridSize.x);
            t++;

            auto &groundElement = m_groundElements.emplace_back();
            groundElement.groundLayer = m_worldGenerator->terrain()->getGroundLayer(hashedGroundName);
            groundElement.position = pos;
        }
    }
}


void WorldGenerator_SpawnPoint::loadRandomModifierValue(TiXmlElement *element, int i, WorldGenerator_SpawnPoint_Modifier &modifier)
{
    const char *valueAtt(nullptr);

    if(i == 0)
    {
        valueAtt = element->Attribute("x");
        if(!valueAtt)
            valueAtt = element->Attribute("r");
    } else if(i == 1) {
        valueAtt = element->Attribute("y");
        if(!valueAtt)
            valueAtt = element->Attribute("g");
    } else if(i == 2) {
        valueAtt = element->Attribute("z");
        if(!valueAtt)
            valueAtt = element->Attribute("b");
    } else if(i == 3) {
        valueAtt = element->Attribute("w");
        if(!valueAtt)
            valueAtt = element->Attribute("a");
    }

    if(!valueAtt)
        return;

    auto valueString = std::string(valueAtt);

     if(valueString == "x" || valueString == "r")
        modifier.usePrecedingValue[i] = 0;
    else if(valueString == "y" || valueString == "g")
        modifier.usePrecedingValue[i] = 1;
    else if(valueString == "z" || valueString == "b")
        modifier.usePrecedingValue[i] = 2;
    else if(valueString == "w" || valueString == "a")
        modifier.usePrecedingValue[i] = 3;
    else
    {
        auto [minV, maxV] = pou::Parser::parseFloatSegment(valueString);
        /**if(minV == maxV)
            minV = -maxV;**/
        modifier.minValue[i] = minV;
        modifier.maxValue[i] = maxV;
    }
}

void WorldGenerator_SpawnPoint::spawnCharacter(CharacterModelAsset *characterModel, glm::vec2 worldPos, float pointRotation,
                                                pou::SceneNode *targetNode, GameWorld_Sync *syncComponent,
                                                bool generateCharacters, pou::RNGenerator *rng)
{
    auto character = std::make_shared<Character>();
    character->createFromModel(characterModel);
    character->transform()->setPosition(worldPos);
    this->applyRandomModifiers(character.get(), pointRotation, rng);

    if(generateCharacters)
    {
        targetNode->addChildNode(character);
        syncComponent->syncElement(character);
        syncComponent->syncElement(characterModel);
    }
}

void WorldGenerator_SpawnPoint::spawnSprite(pou::SpriteModel *spriteModel, glm::vec2 worldPos, float pointRotation,
                                            pou::SceneNode *targetNode, pou::RNGenerator *rng)
{
    auto spriteNode = targetNode->createChildNode();
    spriteNode->transform()->setPosition(worldPos);
    this->applyRandomModifiers(spriteNode.get(), pointRotation, rng);

    auto sprite = std::make_shared<pou::SpriteEntity>();
    sprite->setSpriteModel(spriteModel);
    spriteNode->attachObject(sprite);
    //targetNode->addChildNode(spriteNode);
}

void WorldGenerator_SpawnPoint::spawnPrefab(PrefabAsset *prefabAsset, glm::vec2 worldPos, float pointRotation,
                                            pou::SceneNode *targetNode, GameWorld_Sync *syncComponent,
                                            bool generateCharacters, pou::RNGenerator *rng)
{
    auto prefabNode = prefabAsset->generate();
    prefabNode->transform()->setPosition(worldPos);
    this->applyRandomModifiers(prefabNode.get(), pointRotation, rng);
    targetNode->addChildNode(prefabNode);

    //prefabNode->spawnCharactersOnParent();
    auto characterList = prefabAsset->generateCharacters(prefabNode->transform());
    for(auto character : characterList)
    {
        targetNode->addChildNode(character);
        syncComponent->syncElement(character);
        syncComponent->syncElement(character->getModel());
    }
}


void WorldGenerator_SpawnPoint::spawnPathConnection(WorldGenerator_SpawnPoint_PathConnection &pathConnection, glm::vec2 worldPos,
                                                    pou::RNGenerator *rng)
{
    float probability = pou::RNGesus::uniformFloat(0.0f, 1.0f, rng);
    if(probability > pathConnection.spawnProbability)
        return;

    m_worldGenerator->terrain()->addPathConnection(pathConnection.pathName, worldPos);
}


void WorldGenerator_SpawnPoint::spawnSubSpawn(WorldGenerator_SpawnPoint_SubSpawn &subSpawn, glm::vec2 worldPos,
                         pou::RNGenerator *rng)
{
    float probability = pou::RNGesus::uniformFloat(0.0f, 1.0f, rng);
    if(probability > subSpawn.spawnProbability)
        return;

    worldPos += subSpawn.position * m_worldGenerator->terrain()->getTileSize();

    m_worldGenerator->addToSpawnGroup(subSpawn.spawnGroupName, worldPos);
}

glm::vec4 WorldGenerator_SpawnPoint::generateRandomValue(WorldGenerator_SpawnPoint_Modifier &modifier, pou::RNGenerator *rng)
{
    glm::vec4 randomValue(0);

    for(int i = 0 ; i < 4 ; ++i)
    {
        if(modifier.usePrecedingValue[i] != -1)
        {
            randomValue[i] = randomValue[modifier.usePrecedingValue[i]];
            continue;
        }

        if(modifier.randomType == WorldGenerator_RandomType_Uniform)
            randomValue[i] = pou::RNGesus::uniformFloat(modifier.minValue[i], modifier.maxValue[i], rng);
    }

    return randomValue;
}

void WorldGenerator_SpawnPoint::applyRandomModifiers(pou::SceneNode *targetNode, float pointRotation, pou::RNGenerator *rng)
{
    targetNode->transform()->rotateInRadians({0,0,pointRotation});

    if(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Position].randomType != WorldGenerator_RandomType_None)
    {
        auto v = this->generateRandomValue(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Position], rng);

        float cosRot = cos(pointRotation);
        float sinRot = sin(pointRotation);
        v = glm::vec4(glm::mat2(cosRot, sinRot, -sinRot, cosRot) * glm::vec2(v), v.z, v.w);

        targetNode->transform()->move(glm::vec3(v));
    }

    if(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Rotation].randomType != WorldGenerator_RandomType_None)
    {
        auto v = this->generateRandomValue(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Rotation], rng);
        targetNode->transform()->rotateInDegrees(v);
    }

    if(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Flip].randomType != WorldGenerator_RandomType_None)
    {
        auto v = this->generateRandomValue(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Flip], rng);
        auto scale = glm::vec3(v.x >= 0 ? 1 : -1, v.y >= 0 ? 1 : -1, 1);
        targetNode->transform()->scale(scale);
    }

    if(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Color].randomType != WorldGenerator_RandomType_None)
    {
        auto v = this->generateRandomValue(m_randomModifiers[WorldGenerator_SpawnPoint_ModifierType_Color], rng);
        targetNode->colorize(v);
    }
}

