#define STB_IMAGE_IMPLEMENTATION
#include "../include/engine.hpp"
#include "../include/logger.hpp"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

namespace vkUI::Engine{


void uiEngine::createInstance() {
#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
    if (!checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
#endif
    auto appInfo = vk::ApplicationInfo(
        "Hello Triangle",
        VK_MAKE_VERSION(1, 0, 0),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0
    );

    auto extensions = getRequiredExtensions();

    auto createInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &appInfo,
        0, nullptr, // enabled layers
        static_cast<uint32_t>(extensions.size()), extensions.data() // enabled extensions
    );
#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

    try {
        instance = vk::createInstanceUnique(createInfo, nullptr);
    }
    catch (const vk::SystemError &err) {
       uiLOGE << "Failed to create Instance!";
       uiLOGE << err.what();
        throw std::runtime_error("failed to create instance!");
    }
}


void uiEngine::setupDebugCallback() {
#ifndef VK_ENGINE_ENABLE_VALIDATION_LAUERS
    return;
#endif
    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback,
        nullptr
    );

    // NOTE: Vulkan-hpp has methods for this, but they trigger linking errors...
    //instance->createDebugUtilsMessengerEXT(createInfo);
    //instance->createDebugUtilsMessengerEXTUnique(createInfo);

    // NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these structs
    if (CreateDebugUtilsMessengerEXT(*instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &callback) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug callback!");
    }
}


void uiEngine::pickPhysicalDevice() {
    auto devices = instance->enumeratePhysicalDevices();
    if (devices.size() == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (!physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}


void uiEngine::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({
            vk::DeviceQueueCreateFlags(),
            queueFamily,
            1, // queueCount
            &queuePriority
            });
    }

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    auto createInfo = vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(),
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data()
    );
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#endif
    try {
        device = physicalDevice.createDeviceUnique(createInfo);
    }
    catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to create logical device!");
    }

    graphicsQueue = device->getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device->getQueue(indices.presentFamily.value(), 0);
}



void uiEngine::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    try {
        commandPool = device->createCommandPool(poolInfo);
    }
    catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void uiEngine::createSyncObjects() {
    imageAvailableSemaphores.resize(VK_ENGINE_MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(VK_ENGINE_MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(VK_ENGINE_MAX_FRAMES_IN_FLIGHT);

    try {
        for (size_t i = 0; i < VK_ENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = device->createSemaphore({});
            renderFinishedSemaphores[i] = device->createSemaphore({});
            inFlightFences[i] = device->createFence({vk::FenceCreateFlagBits::eSignaled});
        }
    } catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto physicalDevice = getPhysicalDevicePtr();
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice->getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void uiEngine::createTextureImage() {
    unsigned char *pixels = text_renderer.getData();
    const int texHeight = text_renderer.TexHeight;
    const int texWidth = text_renderer.TexWidth;
    const VkDeviceSize imageSize = texWidth * texHeight;

    if (!pixels) {throw std::runtime_error("failed to load texture image!");}
    std::cout << "Creating image texture!" << texWidth << ", " << texHeight << std::endl;

    uiBuffer stagingBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.copyData((void *)pixels, static_cast<size_t>(imageSize));
    std::cout << "copydata" << std::endl;
    /* stbi_image_free(pixels); */
    {
        vk::ImageCreateInfo info;
        info.imageType = vk::ImageType::e2D;
        info.extent.width = texWidth;
        info.extent.height = texHeight;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.format = vk::Format::eR8Unorm;
        info.tiling = vk::ImageTiling::eOptimal;
        info.initialLayout = vk::ImageLayout::eUndefined;
        info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        info.samples = vk::SampleCountFlagBits::e1;
        info.sharingMode = vk::SharingMode::eExclusive;
    std::cout << "create texture image" << std::endl;
        TextureImage = device->createImage(info);
    }

    {
        vk::MemoryRequirements memr;
    std::cout << "get image requirements" << std::endl;
        device->getImageMemoryRequirements(TextureImage, &memr);
        vk::MemoryAllocateInfo info;
        info.allocationSize = memr.size;
        info.memoryTypeIndex = findMemoryType(memr.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

        try {
            textureImageMemory = device->allocateMemory(info);
        }
        catch (const vk::SystemError &err) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
    }
    std::cout << "bindimagememory" << std::endl;
    
    device->bindImageMemory(TextureImage, textureImageMemory, 0);
    std::cout << "transitionImageLayout" << std::endl;

    transitionImageLayout(TextureImage,vk::Format::eR8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    std::cout << "copyBufferToImage" << std::endl;

    { // copy buffer to image
        auto commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buf, TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        endSingleTimeCommands(commandBuffer);
    }

    std::cout << "transitionImageLayout" << std::endl;
    transitionImageLayout(TextureImage, vk::Format::eR8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    // vkDestroyBuffer(device, stagingBuffer, nullptr);
    // vkFreeMemory(device, stagingBufferMemory, nullptr);

    std::cout << "createimageView" << std::endl;

    // image viewの作成
    { 
        vk::ImageViewCreateInfo info{};
        info.image = TextureImage;
        info.viewType = vk::ImageViewType::e2D;
        info.format =  vk::Format::eR8Unorm;
        info.components.r = vk::ComponentSwizzle::eIdentity;
        info.components.g = vk::ComponentSwizzle::eR;
        info.components.b = vk::ComponentSwizzle::eR;
        info.components.a = vk::ComponentSwizzle::eR;
        // info.components.a = vk::ComponentSwizzle::eIdentity;
        info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        textureImageView = device->createImageView(info);
    }

    {
        auto properties = physicalDevice.getProperties();

        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eNearest;
        samplerInfo.minFilter = vk::Filter::eNearest;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV =vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = false;
        samplerInfo.compareEnable = false;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        textureSampler = device->createSampler(samplerInfo);
    }
}



vk::UniqueShaderModule uiEngine::createShader(std::string filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open  shader file!" + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    try {
        return device->createShaderModuleUnique({
            vk::ShaderModuleCreateFlags(),
            buffer.size(), 
            reinterpret_cast<const uint32_t*>(buffer.data())
        });
    } catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to create shader module!");
    }
}


bool uiEngine::isDeviceSuitable(const vk::PhysicalDevice& device) {
    assert(windows.size() > 0);
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = windows[0]->querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void uiEngine::createDescriptorPool(){
    std::vector<vk::DescriptorPoolSize> poolSizes{ { vk::DescriptorType::eUniformBuffer, 1 }, {vk::DescriptorType::eCombinedImageSampler, 1} };
    assert( !poolSizes.empty() );
    uint32_t maxSets = 2;
    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo( vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, poolSizes );
    descriptorPool = device->createDescriptorPool( descriptorPoolCreateInfo );
}


bool uiEngine::checkDeviceExtensionSupport(const vk::PhysicalDevice& device) {
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : device.enumerateDeviceExtensionProperties()) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

QueueFamilyIndices uiEngine::findQueueFamilies(vk::PhysicalDevice device) {
    assert(windows.size() > 0);
    QueueFamilyIndices indices;
    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, windows[0]->surface)) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<const char*> uiEngine::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    return extensions;
}

bool uiEngine::checkValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}




uiEngine engine;

} // namespace vkUI::Engine

