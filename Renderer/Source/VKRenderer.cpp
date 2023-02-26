#include "VKRenderer.h"
#include <iostream>
#include <algorithm>
#include <set>

using namespace Renderer;

const int VKRenderer::MAX_FRAMES_IN_FLIGHT = 3;

VKAPI_ATTR VkBool32 VKAPI_CALL VKRenderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VKRenderer::VKRenderer(Window& pWindow) : mWindow(pWindow)
{
    mClearValues[0].color = { {0.f, 0.f, 0.f, 1.0f} };
    mClearValues[1].depthStencil = { 1.0f, 0 };
}

VKRenderer::~VKRenderer()
{
    cleanupSwapChain();

    mDescriptorAllocator->cleanup();
    delete mDescriptorAllocator;

    mDescriptorLayoutCache->cleanup();
    delete mDescriptorLayoutCache;

    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    vkDestroyDevice(mDevice, nullptr);

    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);

    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyInstance(mInstance, nullptr);
}

void VKRenderer::destroyDebugUtilsMessengerEXT(VkInstance pInstance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(pInstance, pDebugMessenger, pAllocator);
}

void VKRenderer::init()
{
    //set up
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &mGpuProperties);

    //presentation
    createSwapChain();
    createImageViews();
    createRenderPass();

    //Deepthtest
    createDepthResources();

    //Drawing
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();

    //descriptor
    mDescriptorAllocator = new vkutil::DescriptorAllocator{};
    mDescriptorAllocator->init(mDevice);

    mDescriptorLayoutCache = new vkutil::DescriptorLayoutCache{};
    mDescriptorLayoutCache->init(mDevice);
}

bool VKRenderer::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : mValidationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char*> VKRenderer::getRequiredExtensions()
{
    uint32_t windowExtensionCount = 0;
    const char** windowExtensions;
    windowExtensions = mWindow.getRequiredInstanceExtensions(windowExtensionCount);

    std::vector<const char*> extensions(windowExtensions, windowExtensions + windowExtensionCount);

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void VKRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& pCreateInfo)
{
    pCreateInfo = {};
    pCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    pCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    pCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    pCreateInfo.pfnUserCallback = VKRenderer::debugCallback;
}

VkResult VKRenderer::createDebugUtilsMessengerEXT(VkInstance pInstance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(pInstance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

QueueFamilyIndices VKRenderer::findQueueFamilies(VkPhysicalDevice pdevice)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.mGraphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, mSurface, &presentSupport);

        if (presentSupport)
            indices.mPresentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

bool VKRenderer::checkDeviceExtensionSupport(VkPhysicalDevice pdevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails VKRenderer::querySwapChainSupport(VkPhysicalDevice pdevice)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, mSurface, &details.mCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, mSurface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.mFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, mSurface, &formatCount, details.mFormats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, mSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.mPresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, mSurface, &presentModeCount, details.mPresentModes.data());
    }

    return details;
}

bool VKRenderer::isDeviceSuitable(VkPhysicalDevice pdevice)
{
    QueueFamilyIndices indices = findQueueFamilies(pdevice);

    bool extensionsSupported = checkDeviceExtensionSupport(pdevice);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pdevice);
        swapChainAdequate = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(pdevice, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkSurfaceFormatKHR VKRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& pAvailableFormats)
{
    for (const auto& availableFormat : pAvailableFormats) 
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return pAvailableFormats[0];
}

VkPresentModeKHR VKRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& pAvailablePresentModes)
{
    for (const auto& availablePresentMode : pAvailablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& pCapabilities)
{
    if (pCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return pCapabilities.currentExtent;
    else 
    {
        int width, height;
        mWindow.getFramebufferSize(&width, &height);

        VkExtent2D actualExtent = 
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, pCapabilities.minImageExtent.width, pCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, pCapabilities.minImageExtent.height, pCapabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkFormat VKRenderer::findSupportedFormat(const std::vector<VkFormat>& pCandidates, VkImageTiling pTiling, VkFormatFeatureFlags pFeatures)
{
    for (VkFormat format : pCandidates) 
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

        if (pTiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & pFeatures) == pFeatures)
            return format;
        else if (pTiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & pFeatures) == pFeatures)
            return format;
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VKRenderer::findDepthFormat()
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool VKRenderer::hasStencilComponent(VkFormat pFormat)
{
    return pFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || pFormat == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VKRenderer::createBuffer(VkDeviceSize pSize, VkBufferUsageFlags pUsage, VkMemoryPropertyFlags pProperties, VkBuffer& pBuffer, VkDeviceMemory& pBufferMemory) 
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = pSize;
    bufferInfo.usage = pUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &pBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice, pBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, pProperties);


    //Should not allocate for every mesh check out https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &pBufferMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory!");

    vkBindBufferMemory(mDevice, pBuffer, pBufferMemory, 0);
}

uint32_t VKRenderer::findMemoryType(uint32_t pTypeFilter, VkMemoryPropertyFlags pProperties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((pTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pProperties) == pProperties)
            return i;

    throw std::runtime_error("failed to find suitable memory type!");
}

void VKRenderer::createImage(uint32_t pWidth, uint32_t pHeight, uint32_t pMipLevels, VkFormat pFormat, VkImageTiling pTiling, VkImageUsageFlags pUsage, VkMemoryPropertyFlags pProperties, VkImage& pImage, VkDeviceMemory& pImageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = pWidth;
    imageInfo.extent.height = pHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = pMipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = pFormat;
    imageInfo.tiling = pTiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = pUsage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(mDevice, &imageInfo, nullptr, &pImage) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mDevice, pImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, pProperties);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &pImageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    vkBindImageMemory(mDevice, pImage, pImageMemory, 0);
}

VkCommandBuffer VKRenderer::beginSingleTimeCommands() 
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VKRenderer::endSingleTimeCommands(VkCommandBuffer pCommandBuffer)
{
    vkEndCommandBuffer(pCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &pCommandBuffer;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);

    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &pCommandBuffer);
}

void VKRenderer::transitionImageLayout(VkImage pImage, VkFormat pFormat, VkImageLayout pOldLayout, VkImageLayout pNewLayout, uint32_t pMipLevels)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = pOldLayout;
    barrier.newLayout = pNewLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = pImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = pMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (pOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && pNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (pOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && pNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
        throw std::invalid_argument("unsupported layout transition!");


    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    endSingleTimeCommands(commandBuffer);
}

void VKRenderer::copyBuffer(VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize pSize)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = pSize;
    vkCmdCopyBuffer(commandBuffer, pSrcBuffer, pDstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void VKRenderer::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) 
        throw std::runtime_error("validation layers requested, but not available!");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
        createInfo.ppEnabledLayerNames = mValidationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");
}

void VKRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers) 
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
        throw std::runtime_error("failed to set up debug messenger!");
}

void VKRenderer::createSurface()
{
    if (mWindow.createWindowSurface(*this) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

void VKRenderer::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (const auto& pdevice : devices) 
    {
        if (isDeviceSuitable(pdevice)) 
        {
            mPhysicalDevice = pdevice;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU!");
}

void VKRenderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.mGraphicsFamily.value(), indices.mPresentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) 
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
        createInfo.ppEnabledLayerNames = mValidationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    vkGetDeviceQueue(mDevice, indices.mGraphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.mPresentFamily.value(), 0, &mPresentQueue);
}

void VKRenderer::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.mFormats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.mPresentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.mCapabilities);

    uint32_t imageCount = swapChainSupport.mCapabilities.minImageCount + 1;
    if (swapChainSupport.mCapabilities.maxImageCount > 0 && imageCount > swapChainSupport.mCapabilities.maxImageCount)
        imageCount = swapChainSupport.mCapabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.mGraphicsFamily.value(), indices.mPresentFamily.value() };

    if (indices.mGraphicsFamily != indices.mPresentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    createInfo.preTransform = swapChainSupport.mCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
}

VkImageView VKRenderer::createImageView(VkImage pImage, VkFormat pFormat, VkImageAspectFlags pAspectFlags, uint32_t pMipLevels)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = pImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = pFormat;
    viewInfo.subresourceRange.aspectMask = pAspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = pMipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;


    VkImageView imageView;
    if (vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");

    return imageView;
}

void VKRenderer::createImageViews()
{
    mSwapChainImageViews.resize(mSwapChainImages.size());

    for (size_t i = 0; i < mSwapChainImages.size(); i++) 
        mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VKRenderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass!");
}

void VKRenderer::createFramebuffers() 
{
    mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

    for (size_t i = 0; i < mSwapChainImageViews.size(); i++) 
    {
        std::array<VkImageView, 2> attachments = { mSwapChainImageViews[i], mDepthImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = mSwapChainExtent.width;
        framebufferInfo.height = mSwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

void VKRenderer::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.mGraphicsFamily.value();

    if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

void VKRenderer::createCommandBuffers()
{
    mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

    if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}

void VKRenderer::createSyncObjects()
{
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

}

void VKRenderer::createDepthResources() 
{
    VkFormat depthFormat = findDepthFormat();

    createImage(mSwapChainExtent.width, mSwapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);
    mDepthImageView = createImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void VKRenderer::cleanupSwapChain()
{
    vkDestroyImageView(mDevice, mDepthImageView, nullptr);
    vkDestroyImage(mDevice, mDepthImage, nullptr);
    vkFreeMemory(mDevice, mDepthImageMemory, nullptr);

    for (auto framebuffer : mSwapChainFramebuffers)
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);

    for (auto imageView : mSwapChainImageViews)
        vkDestroyImageView(mDevice, imageView, nullptr);

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

void VKRenderer::recreateSwapChain()
{
    int width = 0, height = 0;
    mWindow.getFramebufferSize(&width, &height);
    while (width == 0 || height == 0)
    {
        mWindow.getFramebufferSize(&width, &height);
        mWindow.waitEvent();
    }

    vkDeviceWaitIdle(mDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void VKRenderer::beginDraw()
{
    vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
    
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);
    vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    //Begin draw clear
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(mCommandBuffers[mCurrentFrame], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = mSwapChainFramebuffers[mImageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapChainExtent;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(mClearValues.size());
    renderPassInfo.pClearValues = mClearValues.data();

    vkCmdBeginRenderPass(mCommandBuffers[mCurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)mSwapChainExtent.width;
    viewport.height = (float)mSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(mCommandBuffers[mCurrentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mSwapChainExtent;
    vkCmdSetScissor(mCommandBuffers[mCurrentFrame], 0, 1, &scissor);
    
    //vkCmdSetPrimitiveTopology(commandBuffers[currentFrame], VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
}

void VKRenderer::endDraw()
{
    vkCmdEndRenderPass(mCommandBuffers[mCurrentFrame]);

    if (vkEndCommandBuffer(mCommandBuffers[mCurrentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];

    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &mImageIndex;

    VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
    {
        mFramebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VKRenderer::waitForCleanUp()
{
    vkDeviceWaitIdle(mDevice);
}

void VKRenderer::finishSetup()
{
    std::unique_lock<std::mutex> lock(mMainThreadMuxtex);
    while (mMainThread.size() != 0)
    {
        std::function<void()> job = mMainThread.front();
        mMainThread.pop();
        job();
    }
}

size_t VKRenderer::padStorageBufferSize(size_t pOriginalSize)
{
    size_t minUboAlignment = mGpuProperties.limits.minStorageBufferOffsetAlignment;
    size_t alignedSize = pOriginalSize;
    if (minUboAlignment > 0)
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    return alignedSize;
}

size_t VKRenderer::padUniformBufferSize(size_t pOriginalSize)
{
    size_t minUboAlignment = mGpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = pOriginalSize;
    if (minUboAlignment > 0)
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    return alignedSize;
}