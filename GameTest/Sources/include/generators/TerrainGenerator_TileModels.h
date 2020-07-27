#ifndef TERRAINGENERATOR_TILEMODELS_H
#define TERRAINGENERATOR_TILEMODELS_H

#include "tinyxml/tinyxml.h"
#include "world/WorldSprite.h"

#include "PouEngine/tools/RNGesus.h"

#include <memory>

class TerrainLayerModelAsset;

struct TerrainGenerator_TileModel
{
    std::shared_ptr<WorldSprite> spriteModel;
    bool allowFlipX;
    bool allowFlipY;
};

class TerrainGenerator_TileModels
{
    public:
        TerrainGenerator_TileModels();
        virtual ~TerrainGenerator_TileModels();

        bool loadFromXML(TiXmlElement *element, TerrainLayerModelAsset *parentLayerModel);

        void addTileModel(float probability, TerrainGenerator_TileModel tileModel);
        std::shared_ptr<WorldSprite> generateSprite(int modValue = -1, pou::RNGenerator *rng = nullptr);

    protected:

    private:
        //std::map<float, std::shared_ptr<WorldSprite> > m_spriteMap;
        bool m_chosenSpriteMap;
        std::map<float, TerrainGenerator_TileModel > m_spriteMap1;
        float m_totalProbability1;

        std::map<float, TerrainGenerator_TileModel > m_spriteMap2;
        float m_totalProbability2;
};

#endif // TERRAINGENERATOR_TILEMODELS_H
