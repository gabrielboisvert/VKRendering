#pragma once
#include <optional>
#include "Window.h"
#include <array>
#include <functional>
#include <queue>
#include <mutex>
#include "VkDescriptor.h"

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

namespace Renderer
{
    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> mGraphicsFamily;
        std::optional<uint32_t> mPresentFamily;

        bool isComplete() 
        {
            return mGraphicsFamily.has_value() && mPresentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR mCapabilities;
        std::vector<VkSurfaceFormatKHR> mFormats;
        std::vector<VkPresentModeKHR> mPresentModes;
    };

	class VKRenderer
	{
		public:
            std::mutex mMainThreadMuxtex;
            std::queue<std::function<void()>> mMainThread;

            static const int MAX_FRAMES_IN_FLIGHT;
            uint32_t mCurrentFrame = 0;

            Window& mWindow;
            const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
            const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME };
            VkInstance mInstance = nullptr;
            VkDebugUtilsMessengerEXT mDebugMessenger = nullptr;
            VkSurfaceKHR mSurface = nullptr;
            VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
            VkDevice mDevice = nullptr;
            VkPhysicalDeviceProperties mGpuProperties;
            VkQueue mGraphicsQueue = nullptr;
            VkQueue mPresentQueue = nullptr;

            VkSwapchainKHR mSwapChain = nullptr;
            std::vector<VkImage> mSwapChainImages;
            VkFormat mSwapChainImageFormat;
            VkExtent2D mSwapChainExtent;
            std::vector<VkImageView> mSwapChainImageViews;

            VkRenderPass mRenderPass = nullptr;
            
            VkImage mDepthImage;
            VkDeviceMemory mDepthImageMemory;
            VkImageView mDepthImageView;

            std::vector<VkSemaphore> mImageAvailableSemaphores;
            std::vector<VkSemaphore> mRenderFinishedSemaphores;
            std::vector<VkFence> mInFlightFences;
            VkCommandPool mCommandPool = nullptr;
            std::vector<VkCommandBuffer> mCommandBuffers;
            std::vector<VkFramebuffer> mSwapChainFramebuffers;

            uint32_t mImageIndex = 0;
            std::array<VkClearValue, 2> mClearValues{};

            bool mFramebufferResized = false;

            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);


            vkutil::DescriptorLayoutCache* mDescriptorLayoutCache;
            vkutil::DescriptorAllocator* mDescriptorAllocator;
            vkutil::DescriptorBuilder mBuilder;

            VKRenderer(Window& pWindow);
            ~VKRenderer();
            void waitForCleanUp();
            void destroyDebugUtilsMessengerEXT(VkInstance pInstance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator);
			void init();
            void createInstance();
            void setupDebugMessenger();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createSwapChain();
            void createImageViews();
            void createRenderPass();

            void createFramebuffers();
            void createCommandPool();
            void createCommandBuffers();

            void createSyncObjects();

            void createDepthResources();

            void cleanupSwapChain();
            void recreateSwapChain();

            bool checkValidationLayerSupport();
            std::vector<const char*> getRequiredExtensions();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& pCreateInfo);
            VkResult createDebugUtilsMessengerEXT(VkInstance pInstance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pdevice);
            bool checkDeviceExtensionSupport(VkPhysicalDevice pdevice);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pdevice);
            bool isDeviceSuitable(VkPhysicalDevice pdevice);
            VkSurfaceFormatKHR VKRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& pAvailableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& pAvailablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& pCapabilities);
            VkFormat findSupportedFormat(const std::vector<VkFormat>& pCandidates, VkImageTiling pTiling, VkFormatFeatureFlags pFeatures);
            VkFormat findDepthFormat();
            bool hasStencilComponent(VkFormat pFormat);
            void createBuffer(VkDeviceSize pSize, VkBufferUsageFlags pUsage, VkMemoryPropertyFlags pProperties, VkBuffer& pBuffer, VkDeviceMemory& pBufferMemory);
            uint32_t findMemoryType(uint32_t pTypeFilter, VkMemoryPropertyFlags pProperties);
            void createImage(uint32_t pWidth, uint32_t pHeight, uint32_t pMipLevels, VkFormat pFormat, VkImageTiling pTiling, VkImageUsageFlags pUsage, VkMemoryPropertyFlags pProperties, VkImage& pImage, VkDeviceMemory& pImageMemory);
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer pCommandBuffer);
            void copyBuffer(VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize pSize);
            VkImageView createImageView(VkImage pImage, VkFormat pFormat, VkImageAspectFlags pAspectFlags, uint32_t pMipLevels);
            void transitionImageLayout(VkImage pImage, VkFormat pFormat, VkImageLayout pOldLayout, VkImageLayout pNewLayout, uint32_t pMipLevels);

            void finishSetup();

            void beginDraw();
            void endDraw();

            size_t padUniformBufferSize(size_t pOriginalSize);
            size_t padStorageBufferSize(size_t pOriginalSize);
	};
}