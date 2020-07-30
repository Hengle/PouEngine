#include "audio/SoundModel.h"

SoundModel::SoundModel() :
    m_isEvent(false)
{
    //ctor
}

SoundModel::~SoundModel()
{
    //dtor
}

bool SoundModel::loadFromXML(TiXmlElement *element)
{
    auto nameAtt    = element->Attribute("name");
    auto pathAtt    = element->Attribute("path");
    auto typeAtt    = element->Attribute("type");

    if(!pathAtt)
        return (false);

    m_path = std::string(pathAtt);

    if(nameAtt)
        m_name = std::string(nameAtt);

    if(typeAtt != nullptr && std::string(typeAtt) != "event")
        m_isEvent = false;
    else
        m_isEvent = true;

    return (true);
}

const std::string &SoundModel::getName()
{
    return m_name;
}

const std::string &SoundModel::getPath()
{
    return m_path;
}

bool SoundModel::isEvent()
{
    return m_isEvent;
}


