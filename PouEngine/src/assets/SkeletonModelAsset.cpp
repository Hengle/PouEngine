#include "PouEngine/assets/SkeletonModelAsset.h"

#include "PouEngine/Types.h"

#include "PouEngine/assets/AssetHandler.h"
#include "PouEngine/assets/TextureAsset.h"

#include "PouEngine/tools/Logger.h"
#include "PouEngine/tools/Parser.h"

namespace pou
{


SkeletonModelAsset::SkeletonModelAsset() : SkeletonModelAsset(-1)
{

}

SkeletonModelAsset::SkeletonModelAsset(const AssetTypeId id) : Asset(id)
{
    m_allowLoadFromFile     = true;
    m_allowLoadFromMemory   = false;
}

SkeletonModelAsset::~SkeletonModelAsset()
{
    //dtor
}


bool SkeletonModelAsset::loadFromFile(const std::string &filePath)
{
    TiXmlDocument file(filePath.c_str());

    if(!file.LoadFile())
    {
        Logger::error("Cannot load skeleton from file: "+filePath);
        std::ostringstream errorReport;
        errorReport << "Because: "<<file.ErrorDesc();
        Logger::error(errorReport);
        return (false);
    }

    TiXmlHandle hdl(&file);
    hdl = hdl.FirstChildElement();

    return this->loadFromXML(&hdl);
}


bool SkeletonModelAsset::loadFromXML(TiXmlHandle *hdl)
{
    bool loaded = true;

    if(hdl == nullptr)
        return (false);

    this->loadNode(&m_rootNode, hdl->FirstChildElement("node").Element());

    /*auto child = hdl->FirstChildElement("node");
    if(child == nullptr)
        return;*/


    auto animElement = hdl->FirstChildElement("animation").Element();
    while(animElement != nullptr)
    {
        this->loadAnimation(animElement);
        animElement = animElement->NextSiblingElement("animation");
    }

    if(loaded)
        Logger::write("Skeleton loaded from file: "+m_filePath);

    return (loaded);
}

void SkeletonModelAsset::loadNode(SceneNode* rootNode, TiXmlElement *element)
{
    HashedString nodeName(0); //, nodeState(0);
    glm::vec3 nodePos(0,0,0);

    auto nameAtt = element->Attribute("name");
    if(nameAtt != nullptr)
    {
        nodeName = Hasher::unique_hash(nameAtt);
        rootNode->setName(nodeName);
    }

    if(nodeName != 0)
    {
        if(!m_nodesByName.insert({nodeName,rootNode}).second)
            Logger::warning("Multiple nodes with the same name \"" + std::string(nameAtt) + "\" in Skeleton Asset : "+m_filePath);
    }

    /**int nodeId = -1;
    if(!nodeName.empty())
    {
        if(!m_nodesByName.insert({nodeName,rootNode}).second)
            Logger::warning("Multiple nodes with the same name \"" + nodeName + "\" in Skeleton Asset : "+m_filePath);

        nodeId = this->generateNodeId(nodeName);
        m_nodesById.insert({nodeId, rootNode});
    }**/

    auto attribute = element->Attribute("x");
    if(attribute != nullptr)
        nodePos.x = Parser::parseFloat(std::string(attribute));

    attribute = element->Attribute("y");
    if(attribute != nullptr)
        nodePos.y = Parser::parseFloat(std::string(attribute));

    attribute = element->Attribute("z");
    if(attribute != nullptr)
        nodePos.z = Parser::parseFloat(std::string(attribute));


    attribute = element->Attribute("state");
    if(attribute != nullptr)
    {
        /**int stateId = this->generateStateId(std::string(attribute));
        m_initialStates.push_back({nodeId, stateId});**/

        auto stateName = Hasher::unique_hash(attribute);
        m_initialStates.push_back({nodeName, stateName});
    }

    attribute = element->Attribute("rigidity");
    if(attribute != nullptr)
    {
        float rigidity = Parser::parseFloat(std::string(attribute));
        rootNode->transform()->setRigidity(rigidity);
    }

    rootNode->transform()->setPosition(nodePos);

    auto rotElement = element->FirstChildElement("rotate");
    if(rotElement != nullptr)
    {
        glm::vec3 rotation = glm::vec3(0);
        auto att = rotElement->Attribute("x");
        if(att != nullptr)
            rotation.x = Parser::parseFloat(att);

        att = rotElement->Attribute("y");
        if(att != nullptr)
            rotation.y = Parser::parseFloat(att);

        att = rotElement->Attribute("z");
        if(att != nullptr)
            rotation.z = Parser::parseFloat(att);

        rootNode->transform()->rotateInDegrees(rotation);
    }

    auto child = element->FirstChildElement("node");
    if(child == nullptr)
        return;

    TiXmlElement* childElement = child->ToElement();
    while(childElement != nullptr)
    {
        auto node = rootNode->createChildNode();
        this->loadNode(node.get(), childElement);

        childElement = childElement->NextSiblingElement("node");
    }
}

void SkeletonModelAsset::loadAnimation(TiXmlElement *element)
{
    std::string animationName;
    auto nameAtt = element->Attribute("name");
    if(nameAtt != nullptr)
        animationName = std::string(nameAtt);
    else
        animationName = "Animation"+std::to_string(m_animations.size());

    ///auto id = this->generateAnimationId(animationName);
    auto hashedAnimatioName = Hasher::unique_hash(animationName);

    std::unique_ptr<SkeletalAnimationModel> animationModel(new SkeletalAnimationModel(this, hashedAnimatioName));

    animationModel->setName(hashedAnimatioName);
    animationModel->loadFromXml(element/*, &m_nodesById*/);

    ///m_animationById.insert({id, animationModel.get()});

    if(!m_animations.insert({hashedAnimatioName,std::move(animationModel)}).second)
        Logger::warning("Multiple animations named \""+animationName+"\" in the skeleton : "+m_filePath);
}

const SceneNode* SkeletonModelAsset::getRootNode() const
{
    return &m_rootNode;
}


std::unordered_map<HashedString, SceneNode*> SkeletonModelAsset::getNodesByName()
{
    return m_nodesByName;
}

/**const std::unordered_map<int, SceneNode*> *SkeletonModelAsset::getNodesById()
{
    return &m_nodesById;
}**/

const std::list<std::pair<HashedString,HashedString> > *SkeletonModelAsset::getInitialStates() const
{
    return &m_initialStates;
}

SkeletalAnimationModel* SkeletonModelAsset::findAnimation(HashedString name)
{
    auto animation = m_animations.find(name);
    if(animation == m_animations.end())
        return (nullptr);
    return animation->second.get();
}

/**SkeletalAnimationModel* SkeletonModelAsset::findAnimation(HashedString id)
{
    auto animation = m_animations.find(id);
    if(animation == m_animations.end())
        return (nullptr);
    return animation->second;
}**/


/**int SkeletonModelAsset::generateNodeId(const std::string &nodeName)
{
    return this->generateId(nodeName, m_nodeIdByName);
}

int SkeletonModelAsset::generateAnimationId(const std::string &animationName)
{
    return this->generateId(animationName, m_animationIdByName);
}

int SkeletonModelAsset::generateSoundId(const std::string &soundName)
{
    return this->generateId(soundName, m_soundIdByName);
}


int SkeletonModelAsset::generateStateId(const std::string &stateName)
{
    return this->generateId(stateName, m_stateIdByName);
}


int SkeletonModelAsset::generateId(HashedString name, std::unordered_map<HashedString, int> &namesMap)
{
    int id = 0;
    if(!namesMap.empty())
    {
        auto founded = namesMap.find(name);
        if(founded != namesMap.end())
            return founded->second;
    }

    id = namesMap.size();//(--m_soundIdByName.end())->second++;
    namesMap.insert({name,id});
    return id;
}


int SkeletonModelAsset::getNodeId(HashedString nodeName) const
{
    return this->getId(nodeName, m_nodeIdByName);
}

int SkeletonModelAsset::getAnimationId(HashedString animationName) const
{
    return this->getId(animationName, m_animationIdByName);
}

int SkeletonModelAsset::getSoundId(HashedString soundName) const
{
    return this->getId(soundName, m_soundIdByName);
}


int SkeletonModelAsset::getStateId(HashedString stateName) const
{
    return this->getId(stateName, m_stateIdByName);
}


int SkeletonModelAsset::getId(HashedString name, const std::unordered_map<HashedString, int> &namesMap) const
{

    auto founded = namesMap.find(name);
    if(founded == namesMap.end())
        return (-1);

    return founded->second;
}**/


/**                        **/
/// SkeletalAnimationModel ///
/**                        **/


SkeletalAnimationModel::SkeletalAnimationModel(SkeletonModelAsset *skeletonModel, HashedString name) :
    m_skeletonModel(skeletonModel),
    m_isLooping(false),
    m_name(name)
{

}

SkeletalAnimationModel::~SkeletalAnimationModel()
{

}

bool SkeletalAnimationModel::loadFromXml(TiXmlElement *element)
{
    auto loopAtt = element->Attribute("loop");
    if(loopAtt != nullptr)
        m_isLooping = Parser::parseBool(loopAtt);

    SkeletalAnimationFrameModel *lastFrame = nullptr;
    auto frameElement = element->FirstChildElement("frame");
    while(frameElement != nullptr)
    {
        m_frames.push_back(SkeletalAnimationFrameModel (m_skeletonModel));
        m_frames.back().loadFromXml(frameElement->ToElement()/*, mapOfNodes*/);
        if(lastFrame != nullptr)
            lastFrame->setNextFrame(&m_frames.back());
        lastFrame = &m_frames.back();
        frameElement = frameElement->NextSiblingElement("frame");
    }

    return (true);
}

std::pair<SkeletalAnimationFrameModel*, bool> SkeletalAnimationModel::nextFrame(SkeletalAnimationFrameModel* curFrame)
{
    bool loop = false;

    if(curFrame == nullptr)
        return {&m_frames.front(), false};

    auto frame = curFrame->m_nextFrame;
    if(frame == nullptr && m_isLooping)
    {
        frame = &m_frames.front();
        loop = true;
    }

    return {frame,loop};
}

void SkeletalAnimationModel::setName(HashedString name)
{
    m_name = name;
}

HashedString SkeletalAnimationModel::getName() const
{
    return m_name;
}

/**int SkeletalAnimationModel::getId() const
{
    return m_id;
}**/

bool SkeletalAnimationModel::isLooping()
{
    return m_isLooping;
}


/**                              **/
/// SkeletalAnimationFrameModel  ///
/**                              **/

SkeletalAnimationFrameModel::SkeletalAnimationFrameModel(SkeletonModelAsset *skeletonModel) :
    m_skeletonModel(skeletonModel),
    m_nextFrame(nullptr),
    m_speedFactor(1.0f),
    m_frameTime(0.0f)
{
}

SkeletalAnimationFrameModel::~SkeletalAnimationFrameModel()
{

}

bool SkeletalAnimationFrameModel::loadFromXml(TiXmlElement *element)
{
    auto mapOfNodes = m_skeletonModel->getNodesByName();

    auto speedAtt = element->Attribute("speedFactor");
    if(speedAtt != nullptr)
        m_speedFactor = Parser::parseFloat(speedAtt);

    auto timeAtt = element->Attribute("frameTime");
    if(timeAtt != nullptr)
        m_frameTime = Parser::parseFloat(timeAtt);

    /*auto tagAtt = element->Attribute("tag");
    if(tagAtt != nullptr)
        m_tag = tagAtt;*/

    auto tagElement = element->FirstChildElement("tag");
    while(tagElement != nullptr)
    {
        FrameTag tag;

        auto valueAtt = tagElement->Attribute("value");
        if(valueAtt != nullptr)
            tag.value = Hasher::unique_hash(valueAtt);

        m_tags.insert({Hasher::unique_hash(tagElement->GetText()), tag});
        tagElement = tagElement->NextSiblingElement("tag");
    }

    auto commandElement = element->FirstChildElement("sound");
    while(commandElement != nullptr)
    {
        ///m_sounds.push_back(m_skeletonModel->generateSoundId(commandElement->GetText()));
        m_sounds.push_back(Hasher::unique_hash(commandElement->GetText()));

        //m_commands.push_back(SkeletalAnimationCommandModel (this));
        //m_commands.back().loadFromXml(commandElement->ToElement());
        commandElement = commandElement->NextSiblingElement("sound");
    }

    commandElement = element->FirstChildElement("state");
    while(commandElement != nullptr)
    {
        auto att = commandElement->ToElement()->Attribute("node");
        if(att != nullptr)
        {
            /**int nodeId  = this->m_skeletonModel->getNodeId(std::string(att));
            int stateId = this->m_skeletonModel->generateStateId(commandElement->GetText());**/

            auto hashedNodeName = Hasher::unique_hash(att);
            auto hashedStateName = Hasher::unique_hash(commandElement->GetText());
            m_states.push_back({hashedNodeName,hashedStateName});
        }
        else {}
            /** generate state for each node in mapOfNodes **/
        commandElement = commandElement->NextSiblingElement("state");
    }

    commandElement = element->FirstChildElement("command");
    while(commandElement != nullptr)
    {
        m_commands.push_back(SkeletalAnimationCommandModel (this));
        m_commands.back().loadFromXml(commandElement->ToElement());
        commandElement = commandElement->NextSiblingElement("command");
    }

    commandElement = element->FirstChildElement("globalCommand");
    while(commandElement != nullptr)
    {
        for(const auto n : mapOfNodes)
        {
            m_commands.push_back(SkeletalAnimationCommandModel (this,Unknown_Command,n.first));
            m_commands.back().loadFromXml(commandElement->ToElement());
        }
        commandElement = commandElement->NextSiblingElement("globalCommand");
    }

    commandElement = element->FirstChildElement("reset");
    while(commandElement != nullptr)
    {
        auto att = commandElement->ToElement()->Attribute("node");
        if(att != nullptr)
        {
            auto nodeName = Hasher::unique_hash(att);
            m_commands.push_back(SkeletalAnimationCommandModel (this,Move_To,nodeName));
            m_commands.push_back(SkeletalAnimationCommandModel (this,Rotate_To,nodeName));
            m_commands.push_back(SkeletalAnimationCommandModel (this,Scale_To,nodeName));

            /**std::string nodeName = std::string(att);
            m_commands.push_back(SkeletalAnimationCommandModel (this,Move_To,m_skeletonModel->getNodeId(nodeName)));
            m_commands.push_back(SkeletalAnimationCommandModel (this,Rotate_To,m_skeletonModel->getNodeId(nodeName)));
            m_commands.push_back(SkeletalAnimationCommandModel (this,Scale_To,m_skeletonModel->getNodeId(nodeName)));
            //m_commands.push_back(SkeletalAnimationCommandModel (this,Color_To,m_skeletonModel->getNodeId(nodeName)));**/
        }
        else
        for(const auto n : mapOfNodes)
        {
            m_commands.push_back(SkeletalAnimationCommandModel (this,Move_To,n.first));
            m_commands.push_back(SkeletalAnimationCommandModel (this,Rotate_To,n.first));
            m_commands.push_back(SkeletalAnimationCommandModel (this,Scale_To,n.first));
            // m_commands.push_back(SkeletalAnimationCommandModel (this,Color_To,n.first));
        }

        commandElement = commandElement->NextSiblingElement("reset");
    }

    return (true);
}

const SkeletonModelAsset *SkeletalAnimationFrameModel::getSkeletonModel() const
{
    return m_skeletonModel;
}

const std::list<SkeletalAnimationCommandModel> *SkeletalAnimationFrameModel::getCommands()const
{
    return &m_commands;
}

const std::list<HashedString> *SkeletalAnimationFrameModel::getSounds() const
{
    return &m_sounds;
}

const std::list<std::pair<HashedString,HashedString> > *SkeletalAnimationFrameModel::getStates() const
{
    return &m_states;
}

float SkeletalAnimationFrameModel::getSpeedFactor() const
{
    return m_speedFactor;
}

float SkeletalAnimationFrameModel::getFrameTime() const
{
    return m_frameTime;
}


bool SkeletalAnimationFrameModel::hasTag(HashedString tag)
{
    return (m_tags.find(tag) != m_tags.end());
}

std::pair <std::unordered_multimap<HashedString, FrameTag>::iterator, std::unordered_multimap<HashedString, FrameTag>::iterator>
    SkeletalAnimationFrameModel::getTagValues(HashedString tag)
{
    return m_tags.equal_range(tag);
}

void SkeletalAnimationFrameModel::setNextFrame(SkeletalAnimationFrameModel *nextFrame)
{
    m_nextFrame = nextFrame;
}


/**                                 **/
/// SkeletalAnimationCommandModel   ///
/**                                 **/

SkeletalAnimationCommandModel::SkeletalAnimationCommandModel(SkeletalAnimationFrameModel *frameModel,
                                        SkelAnimCmdType type, HashedString nodeName/*int nodeId const std::string &node*/) :
    m_frameModel(frameModel),
    m_type(type),
    m_amount(0),
    m_enabledDirection(type != Unknown_Command),
    m_rate(0),
    m_nodeName(nodeName)
    //m_nodeId(nodeId)
    //m_node(node)
{
    /*if(type != Unknown_Command)
        m_enabledDirection = glm::vec4(true);
    else
        m_enabledDirection = glm::vec4(false);*/
}

SkeletalAnimationCommandModel::~SkeletalAnimationCommandModel()
{
}

bool SkeletalAnimationCommandModel::loadFromXml(TiXmlElement *element)
{
    auto att = element->Attribute("type");
    if(att != nullptr)
    {
        std::string type = std::string(att);
        if(type.compare("moveTo") == 0)
            m_type = Move_To;
        else if(type.compare("rotateTo") == 0)
            m_type = Rotate_To;
        else if(type.compare("scaleTo") == 0)
            m_type = Scale_To;
        else if(type.compare("colorTo") == 0)
            m_type = Color_To;
        /**  Add more **/
    }
    else
        Logger::warning("There are animation commands without type");

    att = element->Attribute("node");
    if(att != nullptr)
        m_nodeName = Hasher::unique_hash(att);//std::string(att);

    att = element->Attribute("x");
    if(att != nullptr)
        m_amount.x = Parser::parseFloat(att), m_enabledDirection.x = true;

    att = element->Attribute("y");
    if(att != nullptr)
        m_amount.y = Parser::parseFloat(att), m_enabledDirection.y = true;

    att = element->Attribute("z");
    if(att != nullptr)
        m_amount.z = Parser::parseFloat(att), m_enabledDirection.z = true;



    att = element->Attribute("r");
    if(att != nullptr)
        m_amount.x = Parser::parseFloat(att), m_enabledDirection.x = true;

    att = element->Attribute("g");
    if(att != nullptr)
        m_amount.y = Parser::parseFloat(att), m_enabledDirection.y = true;

    att = element->Attribute("b");
    if(att != nullptr)
        m_amount.z = Parser::parseFloat(att), m_enabledDirection.z = true;

    att = element->Attribute("a");
    if(att != nullptr)
        m_amount.w = Parser::parseFloat(att), m_enabledDirection.w = true;



    att = element->Attribute("rate");
    if(att != nullptr)
        m_rate = glm::abs(Parser::parseFloat(att));

    return (true);
}

SkelAnimCmdType SkeletalAnimationCommandModel::getType() const
{
    return m_type;
}

std::pair<const glm::vec4&,const glm::vec4&> SkeletalAnimationCommandModel::getAmount() const
{
    return {m_amount, m_enabledDirection};
}

float SkeletalAnimationCommandModel::getRate() const
{
    return m_rate;
}

float SkeletalAnimationCommandModel::getFrameTime() const
{
    if(m_frameModel == nullptr)
        return (0.0f);
    return m_frameModel->getFrameTime();
}

/*const std::string &SkeletalAnimationCommandModel::getNode() const
{
    return m_node;
}*/

/*int SkeletalAnimationCommandModel::getNodeId() const
{
    return m_nodeId;
}*/

HashedString SkeletalAnimationCommandModel::getNodeName() const
{
    return m_nodeName;
}

}
