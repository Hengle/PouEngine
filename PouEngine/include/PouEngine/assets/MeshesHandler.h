#ifndef MESHESHANDLER_H
#define MESHESHANDLER_H

#include "PouEngine/assets/AssetHandler.h"
#include "PouEngine/assets/MeshAsset.h"

namespace pou
{

class MeshesHandler : public AssetHandler<MeshAsset>
{
    public:
        MeshesHandler();
        virtual ~MeshesHandler();

        static MeshAsset *makeQuad(glm::vec2 corner, glm::vec2 extent,
                                    MaterialAsset *material,
                                    glm::vec2 texCorner = {0,0}, glm::vec2 texExtent = {1.0,1.0});

        static MeshAsset *makeBox(glm::vec3 corner, glm::vec3 extent,
                                  MaterialAsset *material);

        static MeshAsset *makeMesh(std::vector<MeshVertex> &vertexList,
                                    std::vector<uint16_t> &indexList,
                                    MaterialAsset *material);

    protected:

    private:
};

}

#endif // MESHESHANDLER_H
