#include "PouEngine/assets/Asset.h"
#include "PouEngine/tools/Parser.h"
#include "PouEngine/tools/Logger.h"
///#include "PouEngine/gfx/SceneEntity.h"

namespace pou
{

Asset::Asset() :
    m_allowLoadFromFile(false),
    m_allowLoadFromMemory(false),
    m_loadData(nullptr),
    m_loadSource(LoadSource_None),
    m_loadType(LoadType_Now),
    m_loaded(false),
    m_id(0)
{
   /// m_allowLoadFromStream = false;
}

Asset::Asset(const AssetTypeId id) : Asset()
{
    m_id=id;
}

Asset::~Asset()
{
    //dtor
}


bool Asset::loadFromFile(const std::string &)
{
    return (false);
}

bool Asset::loadFromMemory(void *data, std::size_t size)
{
    return (false);
}

bool Asset::prepareLoadFromFile(const std::string &filePath, AssetLoadType loadType)
{
    if(!m_allowLoadFromFile)
    {
        Logger::error("Asset is not allowed to load from file");
        return (false);
    }

    m_loadSource = LoadSource_File;
    m_filePath = filePath;
    m_fileDirectory = Parser::findFileDirectory(m_filePath);
    m_loadType = loadType;

    return (true);
}

bool Asset::prepareLoadFromMemory(void *data, std::size_t dataSize, AssetLoadType loadType)
{
    if(!m_allowLoadFromMemory)
    {
        Logger::error("Asset is not allowed to load from memory");
        return (false);
    }

    m_loadSource = LoadSource_Memory;
    m_loadData = data;
    m_loadDataSize = dataSize;
    m_loadType = loadType;

    return (true);
}

/**bool Asset::prepareLoadFromStream(sf::InputStream *stream, AssetLoadType loadType)
{
    if(!m_allowLoadFromStream)
    {
        Logger::Error("Asset is not allowed to load from stream");
        return (false);
    }

    m_loadSource = LoadSource_Stream;
    m_loadStream = stream;
    m_loadType = loadType;

    return (true);
}**/

bool Asset::loadNow()
{
    bool loaded = true;

    if(!m_loaded)
    {
        if(m_loadSource == LoadSource_File)
        {
            loaded = this->loadFromFile(m_filePath);
        } else if(m_loadSource == LoadSource_Memory) {
            loaded = this->loadFromMemory(m_loadData, m_loadDataSize);
        } else if(m_loadSource == LoadSource_Stream) {
            ///this->loadFromStream(*m_loadStream);
        } else {
            Logger::error("Cannot load asset");
            loaded = false;
        }

        m_loaded = loaded;
    }

    if(!m_loaded)
        return (false);

    this->sendNotification(NotificationType_AssetLoaded);
    return (true);
}

bool Asset::isLoaded()
{
    return m_loaded;
}

const std::string &Asset::getFilePath()
{
    if(m_loadSource == LoadSource_File)
        return m_filePath;
    return emptyString;
}

const std::string &Asset::getFileDirectory()
{
    if(m_loadSource == LoadSource_File)
        return m_fileDirectory;
    return emptyString;
}


AssetLoadType Asset::getLoadType()
{
    return m_loadType;
}

AssetLoadSource Asset::getLoadSource()
{
    return m_loadSource;
}

AssetTypeId Asset::getId()
{
    return m_id;
}

void Asset::forceLoadType(AssetLoadType type)
{
    m_loadType = type;
}


void Asset::forceLoaded(bool loaded)
{
    m_loaded = loaded;
}

}
