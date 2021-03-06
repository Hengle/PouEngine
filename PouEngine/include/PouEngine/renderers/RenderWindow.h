#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Vulkan/vulkan.h>

#include <string>
#include <vector>
#include <map>

#include "PouEngine/Types.h"
#include "PouEngine/vulkanImpl/vulkanImpl.h"

namespace pou
{

class AbstractRenderer;

class RenderWindow
{
    friend class VApp;

    public:
        RenderWindow();
        virtual ~RenderWindow();

        bool create(size_t w, size_t h, const std::string &name, bool fullscreen = false, size_t framesCount = 1);
        bool init(); //Can only be done after initializing Vulkan
        void destroy();

        bool attachRenderer(AbstractRenderer *renderer);
        bool detachRenderer(RendererName renderer);

        void resize();

        void takeScreenshot(const std::string &filepath);

        size_t      getFramesCount();
        size_t      getSwapchainSize();
        size_t      getFrameIndex();
        size_t      getImageIndex();
        VkExtent2D  getSwapchainExtent();
        VkFormat    getSwapchainImageFormat();
        const std::vector<VFramebufferAttachment> &getSwapchainAttachments();
        const std::vector<VFramebufferAttachment> &getSwapchainDepthAttachments();
        glm::vec2   getSize();

        AbstractRenderer* getRenderer(RendererName renderer);

    protected:
        uint32_t    acquireNextImage(bool ignoreFences = false);
        void        submitToGraphicsQueue(std::vector<VkCommandBuffer> &commandBuffers,
                                          std::vector<VkSemaphore> &waitSemaphores);
        void        display();

        bool    checkVideoMode(size_t w, size_t h, GLFWmonitor *monitor);
        bool    createGLFWindow(size_t w, size_t h, const std::string &name, bool fullscreen);
        bool    createSurface();

        VkSurfaceFormatKHR  chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR    chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D          chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        bool                createSwapchain();
        bool                createSemaphoresAndFences();

        bool                recreateSwapchain();

        void                destroySwapchain();

        VkSurfaceKHR &getSurface();
        GLFWwindow  *getWindowPtr();


    private:
        GLFWwindow         *m_window;
        VkSurfaceKHR        m_surface;

        VkSwapchainKHR                       m_swapchain;
        std::vector<VFramebufferAttachment>  m_swapchainAttachments;
        std::vector<VFramebufferAttachment>  m_depthStencilAttachments;


        size_t      m_framesCount;
        uint32_t    m_curImageIndex;
        size_t      m_curFrameIndex;
        std::vector<VkSemaphore>    m_imageAvailableSemaphore;
        std::vector<VkSemaphore>    m_finishedRenderingSemaphores;
        std::vector<VkFence>        m_inFlightFences;

        std::map<RendererName, AbstractRenderer*>  m_attachedRenderers;

        bool m_resized;
};

}

#endif // RENDERWINDOW_H
