#ifndef VRENDERTARGET_H
#define VRENDERTARGET_H

#include "PouEngine/vulkanImpl/VRenderPass.h"
#include "PouEngine/vulkanImpl/VTexture.h"

namespace pou
{

class VRenderTarget
{
    public:
        VRenderTarget();
        virtual ~VRenderTarget();

        VRenderTarget( const VRenderTarget& ) = delete;
        VRenderTarget& operator=( const VRenderTarget& ) = delete;

        bool init(size_t framebuffersCount, VRenderPass *renderPass);
        void destroy();

        ///Add preexisting attachments
        void addAttachments(const std::vector<VFramebufferAttachment> &attachments);
        ///Create new attachments (that will be created when init)
        void createAttachment(VkFormat format);

        void startRendering(size_t framebufferIndex, VkCommandBuffer cmb, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
        void startRendering(size_t framebufferIndex, VkCommandBuffer cmb, VkSubpassContents contents,
                            VRenderPass* renderPass);

        void setExtent(VkExtent2D extent);
        void setMipLevel(size_t mipLevel);
        void setClearValue(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth = {0.0,0});

        VkExtent2D  getExtent();
        const  std::vector<VFramebufferAttachment> &getAttachments(size_t attachmentIndex);

    protected:
        bool createAttachments(size_t framebuffersCount);
        bool createFramebuffers(size_t framebuffersCount);

    protected:
        size_t      m_imagesCount;
        VkExtent2D  m_extent;
        size_t      m_mipLevel;

        std::vector<VkClearValue>   m_clearValues;
        std::vector<VkFramebuffer>  m_framebuffers;

        VRenderPass *m_defaultRenderPass;
        std::vector<std::vector<VFramebufferAttachment> > m_attachments;

        std::vector<VFramebufferAttachment*> m_createdAttachments;
        std::list<VkFormat> m_creatingAttachmentList;


};

}

#endif // VRENDERTARGET_H
