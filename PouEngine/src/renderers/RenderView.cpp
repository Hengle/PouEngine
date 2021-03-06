#include "PouEngine/renderers/RenderView.h"

namespace pou
{

RenderView::RenderView() :
    m_curBufferIndex(0),
    m_position(0.0f, 0.0f, 0.0f),
    m_lookAt(0.0f, 0.0f, -1.0f),
    m_zoom(1.0f),
    m_descriptorPool(VK_NULL_HANDLE),
    m_descriptorSetLayout(VK_NULL_HANDLE)
{
    //ctor
    m_viewUbo = {};
    m_viewUbo.depthOffsetAndFactor  = glm::vec2(0.0f, 0.0f);
    m_viewUbo.screenOffset          = glm::vec2(0.0f, 0.0f);
    m_viewUbo.screenSizeFactor      = glm::vec2(1.0f, 1.0f);
    m_viewUbo.projFactor            = 1.0f;
    ///m_viewUbo.view                  = glm::mat4(1.0f);
    ///m_viewUbo.viewInv               = glm::mat4(1.0f);
}

RenderView::~RenderView()
{
    this->destroy();
}

bool RenderView::create(size_t framesCount/*, bool isDynamic*/)
{
    //m_isDynamic = isDynamic;

    //if(m_isDynamic)
      //  framesCount = 1;

    if(!this->createBuffers(framesCount))
        return (false);
    if(!this->createDescriptorSetLayout())
        return (false);
    if(!this->createDescriptorPool(framesCount))
        return (false);
    if(!this->createDescriptorSets(framesCount))
        return (false);

    return (true);
}

void RenderView::destroy()
{
    VkDevice device = VInstance::device();

    for(auto buffer : m_buffers)
        VBuffersAllocator::freeBuffer(buffer);

    if(m_descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    m_descriptorSetLayout = VK_NULL_HANDLE;

    if(m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;

    m_descriptorSets.clear();
    m_needToUpdateBuffers.clear();
    m_buffers.clear();
}

void RenderView::update(size_t frameIndex, VkCommandBuffer cmb)
{
    /*if(m_isDynamic)
    {
        m_curBufferIndex = 0;
    }
    else*/ if(m_needToUpdateBuffers[frameIndex])
    {
        /*if(m_isDynamic)
            vkCmdUpdateBuffer(cmb, m_buffers[frameIndex].buffer, m_buffers[frameIndex].offset, sizeof(m_viewUbo), &m_viewUbo);
        else*/
            VBuffersAllocator::writeBuffer(m_buffers[frameIndex],&m_viewUbo,sizeof(m_viewUbo));

        m_needToUpdateBuffers[frameIndex] = false;
    }
}

void RenderView::setupViewport(const ViewInfo &viewInfo, VkCommandBuffer cmb)
{
    //this->setView(viewInfo.view, viewInfo.viewInv);
    //this->update(0, cmb);

    VkViewport viewport = {};
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    /*viewport.x = viewInfo.viewportOffset.x*m_extent.x;
    viewport.y = viewInfo.viewportOffset.y*m_extent.y;
    viewport.width  = viewInfo.viewportExtent.x*m_extent.x;
    viewport.height = viewInfo.viewportExtent.y*m_extent.y;*/
    viewport.width = m_extent.x;
    viewport.height = m_extent.y;

    vkCmdSetViewport(cmb, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {(int32_t)viewInfo.viewportOffset.x*(int32_t)m_extent.x,
                      (int32_t)viewInfo.viewportOffset.y*(int32_t)m_extent.y};
    scissor.extent = {(uint32_t)viewInfo.viewportExtent.x*(uint32_t)m_extent.x,
                      (uint32_t)viewInfo.viewportExtent.y*(uint32_t)m_extent.y};

    vkCmdSetScissor(cmb, 0, 1, &scissor);

}

void RenderView::setDepthFactor(float depthFactor)
{
    if(m_viewUbo.depthOffsetAndFactor.y != 1.0f/depthFactor)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.depthOffsetAndFactor.y = 1.0f/depthFactor;
}

void RenderView::setExtent(glm::vec2 extent)
{
    if(m_viewUbo.screenSizeFactor.x != 2.0/extent.x
    || m_viewUbo.screenSizeFactor.y != 2.0/extent.y)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.screenSizeFactor.x = 2.0/extent.x;
    m_viewUbo.screenSizeFactor.y = 2.0/extent.y;
    m_extent = extent;
}

void RenderView::setScreenOffset(glm::vec3 offset)
{
    if(m_viewUbo.screenOffset.x != offset.x
    || m_viewUbo.screenOffset.y != offset.y
    || m_viewUbo.depthOffsetAndFactor.x != offset.z)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.screenOffset.x = offset.x;
    m_viewUbo.screenOffset.y = offset.y;
    m_viewUbo.depthOffsetAndFactor.x = offset.z;
}

/**void RenderView::setLookAt(glm::vec3 position, glm::vec3 lookAt)
{
    for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.view = glm::lookAt(position, lookAt, glm::vec3(0.0,0.0,1.0));
    m_viewUbo.viewInv = glm::inverse(m_viewUbo.view); ///Could maybe be optimize
}**/

void RenderView::setProjectionFactor(float projFactor)
{
    if(projFactor != m_viewUbo.projFactor)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.projFactor = projFactor;
}

/**void RenderView::setView(glm::mat4 view, glm::mat4 viewInv)
{
    if(view != m_viewUbo.view)
        for(auto b : m_needToUpdateBuffers) b = true;
    m_viewUbo.view      = view;
    m_viewUbo.viewInv   = viewInv;
}**/

/**void RenderView::setZoom(float zoom)
{

}**/


/**glm::vec3 RenderView::getTranslate()
{
    return {m_viewUbo.view[3][0], m_viewUbo.view[3][1], m_viewUbo.view[3][2]};
}**/

VkDescriptorSetLayout RenderView::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet RenderView::getDescriptorSet(size_t frameIndex)
{
    return m_descriptorSets[frameIndex];
}


/// Protected ///

bool RenderView::createBuffers(size_t framesCount)
{
    VkDeviceSize bufferSize = sizeof(ViewUBO);

    m_buffers.resize(framesCount);
    m_needToUpdateBuffers.resize(framesCount);

    for (size_t i = 0 ; i < framesCount ; ++i)
    {
        VkMemoryPropertyFlags properties;
        VkBufferUsageFlagBits usage{};

        /*if(m_isDynamic)
        {
            properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        } else*/
            properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        if(!VBuffersAllocator::allocBuffer(bufferSize, usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           properties, m_buffers[i]))
            return (false);

        m_needToUpdateBuffers[i] = true;
    }
    return (true);
}

bool RenderView::createDescriptorSetLayout()
{
    VkDevice device = VInstance::device();

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; ///I could need to update to frag stage also
    layoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;

    return (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) == VK_SUCCESS);
}

bool RenderView::createDescriptorPool(size_t framesCount)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(framesCount);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(framesCount);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool RenderView::createDescriptorSets(size_t framesCount)
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayout> layouts(framesCount, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(framesCount);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(framesCount);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < framesCount ; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_buffers[i].buffer;
        bufferInfo.offset = m_buffers[i].offset;
        bufferInfo.range = sizeof(ViewUBO);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    return (true);
}



}
