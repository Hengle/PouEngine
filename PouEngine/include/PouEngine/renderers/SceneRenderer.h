/**
* This class create the graphic pipeline necessary to render scenes.
**/

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "PouEngine/renderers/AbstractRenderer.h"
#include "PouEngine/renderers/RenderGraph.h"

#include "PouEngine/scene/Scene.h"
#include "PouEngine/scene/SpriteEntity.h"
#include "PouEngine/scene/MeshEntity.h"
#include "PouEngine/scene/LightEntity.h"

#include "PouEngine/vulkanImpl/DynamicVBO.h"

namespace pou
{

/*struct ShadowMapRenderingInstance
{
    glm::vec4 lightPosition;
    glm::vec2 shadowShift;
    size_t    spritesVboSize;
    size_t    spritesVboOffset;
    std::map<VMesh*, size_t>    meshesVboSize; //I would gain perf by pairing them
    std::map<VMesh*, size_t>    meshesVboOffset;
};*/

class SceneRenderer : public AbstractRenderer
{
    public:
        SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order);
        virtual ~SceneRenderer();

        virtual void update(size_t frameIndex) override;

        void addRenderingInstance(SceneRenderingInstance *renderingInstance);

        //I'll need light position and so forth (maybe I'll use push constants, it's like cam pos)...
        /*void addShadowMapToRender(VRenderTarget* shadowMap, const LightDatum &datum);
        void addSpriteShadowToRender(VRenderTarget* spriteShadow, const SpriteShadowGenerationDatum &datum);
        void addToSpriteShadowsVbo(const SpriteShadowDatum &datum);
        void addToMeshShadowsVbo(VMesh *mesh, const MeshDatum &datum);*/

        void addToSpritesVbo(const SpriteDatum &datum);
        void addToMeshesVbo(VMesh *mesh, const MeshDatum &datum);
        void addToLightsVbo(const LightDatum &datum);

        size_t getSpritesVboSize();
        size_t getMeshesVboSize(VMesh *mesh);
        size_t getLightsVboSize();

        /*VRenderPass *getSpriteShadowsRenderPass();
        VRenderPass *getShadowMapsRenderPass();*/

    protected:
        virtual bool init();
        virtual void cleanup();

        virtual void prepareRenderPass() override;
        virtual bool createGraphicsPipeline() override;

        bool createAttachments();


        ///Render passes
        /*void prepareShadowRenderPass();*/
        //Deferred
        void prepareDeferredRenderPass();
        /*void prepareAlphaDetectRenderPass();
        void prepareAlphaDeferredRenderPass();*/
        //Lighting
        /*void prepareSsgiBNRenderPasses();*/
        void prepareLightingRenderPass();
        /*void prepareAlphaLightingRenderPass();
        void prepareSsgiLightingRenderPass();*/
        void prepareAmbientLightingRenderPass();
        //Final pass
        void prepareToneMappingRenderPass();

        ///Pipelines
        /*bool createSpriteShadowsGenPipeline();
        bool createSpriteShadowsPipeline();
        bool createMeshDirectShadowsPipeline();*/
        //Deferred
        bool createDeferredSpritesPipeline();
        bool createDeferredMeshesPipeline();
        /*bool createAlphaDetectPipeline();
        bool createAlphaDeferredPipeline();*/
        //Lighting
        /*bool createSsgiBNPipelines();*/
        bool createLightingPipeline();
        /*bool createAlphaLightingPipeline();
        bool createSsgiLightingPipeline();*/
        bool createAmbientLightingPipeline();
        //Tone mapping
        bool createToneMappingPipeline();

        virtual bool    recordPrimaryCmb(uint32_t imageIndex) override;

        virtual void    uploadVbos();

        /*virtual bool    recordShadowCmb(uint32_t imageIndex);*/
        virtual bool    recordDeferredCmb(uint32_t imageIndex);
        virtual bool    recordLightingCmb(uint32_t imageIndex);
        /*virtual bool    recordSsgiBnCmb(uint32_t imageIndex);*/
        virtual bool    recordAmbientLightingCmb(uint32_t imageIndex);
        virtual bool    recordToneMappingCmb(uint32_t imageIndex);

    private:
        /*VGraphicsPipeline   m_spriteShadowsGenPipeline,
                            m_spriteShadowFilteringPipeline,
                            //m_spriteShadowsBlurPipelines[2],
                            m_spriteShadowsPipeline,
                            m_meshDirectShadowsPipeline;*/

        VGraphicsPipeline   m_deferredSpritesPipeline,
                            m_deferredMeshesPipeline/*,
                            m_alphaDetectPipeline,
                            m_alphaDeferredPipeline*/;

        VGraphicsPipeline   /*m_ssgiBNPipeline,
                            m_ssgiBNBlurPipelines[2],*/
                            m_lightingPipeline,
                            /*m_alphaLightingPipeline,
                            m_ssgiLightingPipeline,
                            m_ssgiLightingBlurPipelines[2],*/
                            m_ambientLightingPipeline,
                            m_toneMappingPipeline;

        //std::vector<std::list<VRenderableTexture> > m_spriteShadowBufs;

        /**VFramebufferAttachment m_deferredDepthAttachment;
        VFramebufferAttachment m_albedoAttachments[NBR_ALPHA_LAYERS],
                                            m_positionAttachments[NBR_ALPHA_LAYERS], //The opac.a contains existence of truly trasparent frag, the alpha.a contains alphaAlbedo.a
                                            m_normalAttachments[NBR_ALPHA_LAYERS], //The opac.a = 0 and alpha.a contains existence of truly transparent frag
                                            m_rmtAttachments[NBR_ALPHA_LAYERS];
        VFramebufferAttachment m_hdrAttachements[NBR_ALPHA_LAYERS];

        VFramebufferAttachment m_ssgiBentNormalsAttachment;
        VFramebufferAttachment m_ssgiAccuLightingAttachment;
        VFramebufferAttachment m_ssgiCollisionsAttachments[NBR_SSGI_SAMPLES];
        VFramebufferAttachment m_SSGIBlurBentNormalsAttachments[2];
        VFramebufferAttachment m_SSGIBlurLightingAttachments[2];**/

        VFramebufferAttachment m_deferredDepthAttachment;
        VFramebufferAttachment m_albedoAttachment;
        VFramebufferAttachment m_hdrAttachement;

        size_t  /*m_spriteShadowsPass,
                //m_spriteShadowsBlurPasses[2],
                m_shadowMapsPass,*/
                m_deferredPass,
                /*m_alphaDetectPass,
                m_alphaDeferredPass,
                m_ssgiBNPass,
                m_ssgiBNBlurPasses[2],*/
                m_lightingPass,
                /*m_alphaLightingPass,
                m_ssgiLightingPass,
                m_ssgiLigghtingBlurPasses[2],*/
                m_ambientLightingPass,
                m_toneMappingPass;


        ///I should maybe sort by material (?)
        /*std::vector<DynamicVBO<SpriteShadowGenerationDatum>*>   m_spriteShadowGenerationVbos;
        std::vector<DynamicVBO<SpriteShadowDatum>*>          m_spriteShadowsVbos;*/
        std::vector<DynamicVBO<SpriteDatum>*>                m_spritesVbos;
        std::vector<std::map<VMesh* ,DynamicVBO<MeshDatum>*> >  m_meshesVbos;
        std::vector<DynamicVBO<LightDatum>*>                    m_lightsVbos;

        std::list<SceneRenderingInstance*>      m_renderingInstances;
        //std::list<ShadowMapRenderingInstance>   m_shadowMapsInstances;

        /*static const float SSGI_SIZE_FACTOR;

        static const char *ISOSPRITE_SHADOWGEN_VERTSHADERFILE;
        static const char *ISOSPRITE_SHADOWGEN_FRAGSHADERFILE;
        static const char *ISOSPRITE_SHADOWFILT_VERTSHADERFILE;
        static const char *ISOSPRITE_SHADOWFILT_FRAGSHADERFILE;
        static const char *ISOSPRITE_SHADOW_VERTSHADERFILE;
        static const char *ISOSPRITE_SHADOW_FRAGSHADERFILE;
        static const char *MESH_DIRECTSHADOW_VERTSHADERFILE;
        static const char *MESH_DIRECTSHADOW_FRAGSHADERFILE;
        static const char *ISOSPRITE_DEFERRED_VERTSHADERFILE;
        static const char *ISOSPRITE_DEFERRED_FRAGSHADERFILE;
        static const char *MESH_DEFERRED_VERTSHADERFILE;
        static const char *MESH_DEFERRED_FRAGSHADERFILE;
        static const char *ISOSPRITE_ALPHADETECT_VERTSHADERFILE;
        static const char *ISOSPRITE_ALPHADETECT_FRAGSHADERFILE;
        static const char *ISOSPRITE_ALPHADEFERRED_VERTSHADERFILE;
        static const char *ISOSPRITE_ALPHADEFERRED_FRAGSHADERFILE;
        static const char *SSGI_BN_VERTSHADERFILE;
        static const char *SSGI_BN_FRAGSHADERFILE;
        static const char *LIGHTING_VERTSHADERFILE;
        static const char *LIGHTING_FRAGSHADERFILE;
        static const char *SSGI_LIGHTING_VERTSHADERFILE;
        static const char *SSGI_LIGHTING_FRAGSHADERFILE;
        static const char *AMBIENTLIGHTING_VERTSHADERFILE;
        static const char *AMBIENTLIGHTING_FRAGSHADERFILE;
        static const char *TONEMAPPING_VERTSHADERFILE;
        static const char *TONEMAPPING_FRAGSHADERFILE;
        static const char *BLUR_VERTSHADERFILE;
        static const char *BLUR_FRAGSHADERFILE;*/


        static const char *SPRITE_DEFERRED_VERTSHADERFILE;
        static const char *SPRITE_DEFERRED_FRAGSHADERFILE;
        static const char *MESH_DEFERRED_VERTSHADERFILE;
        static const char *MESH_DEFERRED_FRAGSHADERFILE;
        static const char *LIGHTING_VERTSHADERFILE;
        static const char *LIGHTING_FRAGSHADERFILE;
        static const char *AMBIENTLIGHTING_VERTSHADERFILE;
        static const char *AMBIENTLIGHTING_FRAGSHADERFILE;
        static const char *TONEMAPPING_VERTSHADERFILE;
        static const char *TONEMAPPING_FRAGSHADERFILE;
};

}

#endif // SCENERENDERER_H
