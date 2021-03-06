#ifndef LIGHT_H
#define LIGHT_H

#include "PouEngine/core/NotificationSender.h"
#include "PouEngine/scene/SceneEntity.h"
#include "PouEngine/vulkanImpl/VulkanImpl.h"
#include "PouEngine/Types.h"

#include "tinyxml/tinyxml.h"

#define LIGHT_TRIANGLECOUNT 6

namespace pou
{

class SceneRenderer;
class ShadowCaster;

struct LightDatum
{
    glm::vec4 position; //w = 0 for directionnal
    glm::vec4 color; //alpha for intensity
    float     radius;

    glm::uvec2 shadowMap;
    glm::vec2  shadowShift;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();


    //Not in the vbo
    float  shadowBlurRadius;
};

struct LightModel
{
    LightModel();

    bool loadFromXML(TiXmlElement *element);

    LightType   type;
    glm::vec3   direction;
    Color       color;

    float       radius;
    float       intensity;

    bool        castShadow;
    glm::vec2   shadowMapExtent;
    float       shadowBlurRadius;
};

/// I could try to compute an accurate englobing sphere and use early z-testing to discard pixels

class LightEntity : public SceneEntity, public NotificationSender //, public NotificationListener
{
    friend class SceneNode;

    public:
        LightEntity();
        virtual ~LightEntity();

        virtual std::shared_ptr<SceneObject> createCopy();

        LightType   getType();
        glm::vec3   getDirection();
        Color       getDiffuseColor();

        float getRadius();
        float getIntensity();
        bool  isCastingShadows();

        void setType(LightType);
        void setDirection(glm::vec3);
        void setDiffuseColor(glm::vec3);
        void setDiffuseColor(Color);
        void setRadius(float);
        void setIntensity(float);
        void setModel(const LightModel &lightModel);

        void enableShadowCasting();
        void disableShadowCasting();

        void setShadowMapExtent(glm::vec2);
        void setShadowBlurRadius(float radius);

        /*const sf::Texture& GetShadowMap();
        const sf::IntRect& GetShadowMaxShift();

        void RenderShadowMap(const sf::View &);
        void UpdateShadow();*/


        virtual void generateRenderingData(SceneRenderingInstance *renderingInstance);
        virtual void generateShadowMap(SceneRenderer* renderer, std::vector<ShadowCaster*> &shadowCastersList);

        LightDatum getLightDatum();

        virtual void notify(NotificationSender* , int notificationType,
                            void* data = nullptr) override;


    protected:
        //std::list<ShadowCaster*> *GetShadowCasterList();

        virtual void updateDatum();
        void recreateShadowMap(SceneRenderer* renderer);

    private:
        /*LightType   m_type;
        glm::vec3   m_direction;
        Color       m_color;

        float m_radius;
        float m_intensity;

        bool                m_castShadow;
        glm::vec2           m_shadowMapExtent;*/

        LightModel  m_lightModel;
        LightDatum  m_datum;

        VRenderableTexture  m_shadowMap; //Note that I would actually need one shadow map per renderView if multy viewports... (or I partition shadow map ?)

        /*sf::RenderTexture m_shadowMap;
        sf::IntRect m_shadowMaxShift;
        bool m_requireShadowComputation;

        std::list<ShadowCaster*> m_shadowCasterList;*/
};

}

#endif // LIGHT_H
