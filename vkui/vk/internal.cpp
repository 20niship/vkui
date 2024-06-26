#ifndef WITH_VULKAN
#pragma message("Vulkanでレンダリングされていいないよ！CmakeListsがおかしいかも")
#endif

#include <chrono>
#include <engine.hpp>
#include <logger.hpp>
#include <vk/internal.hpp>

namespace vkUI::Render {

static auto get_device() {
  return getContextPtr()->renderer.get_device_ptr();
}
static auto get_physical_dev_ptr() {
  return getContextPtr()->renderer.get_physical_device_ptr();
}

auto get_vert_shader() {
  return getContextPtr()->renderer.get_vert_shader();
}
auto get_frag_shader() {
  return getContextPtr()->renderer.get_frag_shader();
}
auto get_descriptor_pool() {
  return getContextPtr()->renderer.get_descriptor_pool();
}
auto get_texture_image_view() {
  return getContextPtr()->renderer.get_texture_imageview();
}
auto get_frag_shader_ui() {
  return getContextPtr()->renderer.get_frag_ui_shader();
}
auto get_vert_shader_ui() {
  return getContextPtr()->renderer.get_vert_ui_shader();
}
auto get_sampler() {
  return getContextPtr()->renderer.get_texture_sampler();
}
static auto get_command_pool() {
  return getContextPtr()->renderer.get_command_pool();
}
auto get_presense_queue() {
  return getContextPtr()->renderer.get_present_queue();
}
auto get_image_availabele_semaphos() {
  return getContextPtr()->renderer.get_image_available_semaphos();
}
auto get_render_finished_semaphos() {
  return getContextPtr()->renderer.get_render_finished_semaphos();
}
auto get_fences() {
  return getContextPtr()->renderer.get_fences();
}
auto get_graphics_queue() {
  return getContextPtr()->renderer.get_graphics_queue();
}


const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if(func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if(func != nullptr) {
    func(instance, callback, pAllocator);
  }
}



void vkRender::createInstance() {
#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
  if(!checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }
#endif
  auto appInfo = vk::ApplicationInfo("Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

  auto extensions = getRequiredExtensions();

  auto createInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &appInfo, 0, nullptr,            // enabled layers
                                           static_cast<uint32_t>(extensions.size()), extensions.data() // enabled extensions
  );
#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

  try {
    instance = vk::createInstanceUnique(createInfo, nullptr);
  } catch(const vk::SystemError& err) {
    uiLOGE << "Failed to create Instance!";
    uiLOGE << err.what();
    throw std::runtime_error("failed to create instance!");
  }
  uiLOGI << "createInstance";
}


void vkRender::setupDebugCallback() {
#ifndef VK_ENGINE_ENABLE_VALIDATION_LAUERS
  return;
#endif
  auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
    vk::DebugUtilsMessengerCreateFlagsEXT(), vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance, debugCallback, nullptr);

  // NOTE: Vulkan-hpp has methods for this, but they trigger linking errors...
  // instance->createDebugUtilsMessengerEXT(createInfo);
  // instance->createDebugUtilsMessengerEXTUnique(createInfo);

  // NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these structs
  if(CreateDebugUtilsMessengerEXT(*instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &callback) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
  }
}


void vkRender::pickPhysicalDevice() {
  auto devices = instance->enumeratePhysicalDevices();
  if(devices.size() == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  for(const auto& device : devices) {
    if(isDeviceSuitable(device)) {
      physicalDevice = device;
      break;
    }
  }

  if(!physicalDevice) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}


void vkRender::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;

  for(uint32_t queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back({vk::DeviceQueueCreateFlags(), queueFamily,
                                1, // queueCount
                                &queuePriority});
  }

  auto deviceFeatures = vk::PhysicalDeviceFeatures();
  auto createInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data());
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();
#endif
  try {
    device = physicalDevice.createDeviceUnique(createInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create logical device!");
  }

  graphicsQueue = device->getQueue(indices.graphicsFamily.value(), 0);
  presentQueue = device->getQueue(indices.presentFamily.value(), 0);
}



void vkRender::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

  vk::CommandPoolCreateInfo poolInfo = {};
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  try {
    commandPool = device->createCommandPool(poolInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void vkRender::createSyncObjects() {
  imageAvailableSemaphores.resize(VK_ENGINE_MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(VK_ENGINE_MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(VK_ENGINE_MAX_FRAMES_IN_FLIGHT);

  try {
    for(size_t i = 0; i < VK_ENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
      imageAvailableSemaphores[i] = device->createSemaphore({});
      renderFinishedSemaphores[i] = device->createSemaphore({});
      inFlightFences[i] = device->createFence({vk::FenceCreateFlagBits::eSignaled});
    }
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create synchronization objects for a frame!");
  }
}

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
  auto physicalDevice = get_physical_dev_ptr();
  vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice->getMemoryProperties();
  for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

void vkRender::createTextureImage() {
  const auto text_renderer = getTextRendererPtr();
  unsigned char* pixels = text_renderer->getData();
  const int texHeight = text_renderer->TexHeight;
  const int texWidth = text_renderer->TexWidth;
  const VkDeviceSize imageSize = texWidth * texHeight;

  if(!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }
  std::cout << "Creating image texture!" << texWidth << ", " << texHeight << std::endl;

  uiBuffer stagingBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  stagingBuffer.copyData((void*)pixels, static_cast<size_t>(imageSize));
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
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to allocate buffer memory!");
    }
  }
  std::cout << "bindimagememory" << std::endl;

  device->bindImageMemory(TextureImage, textureImageMemory, 0);
  std::cout << "transitionImageLayout" << std::endl;

  transitionImageLayout(TextureImage, vk::Format::eR8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
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
    region.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};
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
    info.format = vk::Format::eR8Unorm;
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
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
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



vk::UniqueShaderModule vkRender::createShader(std::string filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if(!file.is_open()) {
    throw std::runtime_error("failed to open  shader file!" + filename);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  try {
    return device->createShaderModuleUnique({vk::ShaderModuleCreateFlags(), buffer.size(), reinterpret_cast<const uint32_t*>(buffer.data())});
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create shader module!");
  }
}


bool vkRender::isDeviceSuitable(const vk::PhysicalDevice& device) {
  const auto ctx = getContextPtr();
  assert(ctx->windows.size() > 0);
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if(extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = ctx->windows[0]->get_renderer()->querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void vkRender::createDescriptorPool() {
  std::vector<vk::DescriptorPoolSize> poolSizes{{vk::DescriptorType::eUniformBuffer, 1}, {vk::DescriptorType::eCombinedImageSampler, 1}};
  assert(!poolSizes.empty());
  uint32_t maxSets = 2;
  vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, poolSizes);
  descriptorPool = device->createDescriptorPool(descriptorPoolCreateInfo);
}


bool vkRender::checkDeviceExtensionSupport(const vk::PhysicalDevice& device) {
  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for(const auto& extension : device.enumerateDeviceExtensionProperties()) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

QueueFamilyIndices vkRender::findQueueFamilies(vk::PhysicalDevice device) {
  assert(getContextPtr()->windows.size() > 0);
  QueueFamilyIndices indices;
  auto queueFamilies = device.getQueueFamilyProperties();

  int i = 0;
  for(const auto& queueFamily : queueFamilies) {
    if(queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;
    }

    if(queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, (*getAllWindows())[0]->get_renderer()->getSurface())) {
      indices.presentFamily = i;
    }

    if(indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

std::vector<const char*> vkRender::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
  return extensions;
}

bool vkRender::checkValidationLayerSupport() {
  auto availableLayers = vk::enumerateInstanceLayerProperties();
  for(const char* layerName : validationLayers) {
    bool layerFound = false;

    for(const auto& layerProperties : availableLayers) {
      if(strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if(!layerFound) {
      return false;
    }
  }
  return true;
}

vkRender::vkRender() {}

void vkRender::init() {
  uiLOGE << "create instance!";
  createInstance();
  setupDebugCallback();
  const auto engine = getContextPtr();
  for(auto&& w : engine->windows) w->get_renderer()->createSurface(w->getGLFWwindow());
  // createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createDescriptorPool();
  createCommandPool();
  createShaders();
  createSyncObjects();
  createTextureImage();
  std::cout << "createTextureImage end" << std::endl;
  const auto windows = getContextPtr()->windows;
  for(auto&& w : windows) w->init(); // TODO: AddWindowを複数回したときに同じWindowのinitが複数よされることになる
  std::cout << "createSyncObjects" << std::endl;
}

void vkRender::terminate() {
  // NOTE: instance destruction is handled by UniqueInstance, same for device
  // cleanupSwapChain();
  // for(int i=0; i<windows.size(); i++;){windows[i]->cleanupSwapChainU(); } // TODO: cleanupSwapChainU

  for(size_t i = 0; i < VK_ENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
    device->destroySemaphore(renderFinishedSemaphores[i]);
    device->destroySemaphore(imageAvailableSemaphores[i]);
    device->destroyFence(inFlightFences[i]);
  }

  device->destroyCommandPool(commandPool);

  // surface is created by glfw, therefore not using a Unique handle
  // instance->destroySurfaceKHR(surface); // TODO:

#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
  DestroyDebugUtilsMessengerEXT(*instance, callback, nullptr);
#endif
}

SwapChainSupportDetails vkWndRender::querySwapChainSupport(const vk::PhysicalDevice& device) {
  assert(surface);
  SwapChainSupportDetails details;
  details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
  details.formats = device.getSurfaceFormatsKHR(surface);
  details.presentModes = device.getSurfacePresentModesKHR(surface);

  return details;
}

void vkWndRender::createSurface(GLFWwindow* window) {
  if(!glfwVulkanSupported()) {
    uiLOGE << "Vulkan is not supported! ";
    throw std::runtime_error("Vulkan is not supported (glfwVulkanSupported() failed)");
  }

  auto instance = getContextPtr()->renderer.get_instance_ptr();
  assert(window != nullptr);
  assert(instance != nullptr);

  VkSurfaceKHR rawSurface;
  if(glfwCreateWindowSurface((*instance).get(), window, nullptr, &rawSurface) != VK_SUCCESS) {
    uiLOGE << "failed to create window surface!";
    const char* description;
    int code = glfwGetError(&description);
    std::cout << code << "= " << description << std::endl;
    throw std::runtime_error("failed to create window surface!");
  }
  uiLOGI << "createSurface called";
  surface = rawSurface;
}

void vkWndRender::createSwapChain() {
  assert(surface);
  auto device = getContextPtr()->renderer.get_device_ptr();
  auto physicalDevice = getContextPtr()->renderer.get_physical_device_ptr();
  assert(device != nullptr);
  assert(physicalDevice != nullptr);
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*physicalDevice);

  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainCreateFlagsKHR(), surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent,
                                        1, // imageArrayLayers
                                        vk::ImageUsageFlagBits::eColorAttachment);
  QueueFamilyIndices indices = getContextPtr()->renderer.findQueueFamilies(*physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if(indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

  try {
    swapChain = (*device)->createSwapchainKHR(createInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create swap chain!");
  }

  swapChainImages = (*device)->getSwapchainImagesKHR(swapChain);

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void vkWndRender::createImageViews() {
  auto device = get_device();
  swapChainImageViews.resize(swapChainImages.size());

  for(size_t i = 0; i < swapChainImages.size(); i++) {
    vk::ImageViewCreateInfo createInfo = {};
    createInfo.image = swapChainImages[i];
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = swapChainImageFormat;
    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    try {
      swapChainImageViews[i] = (*device)->createImageView(createInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void vkWndRender::createRenderPass() {
  auto device = get_device();

  vk::AttachmentDescription colorAttachment = {};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass = {};
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  vk::SubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  // dependency.srcAccessMask = 0;
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  try {
    renderPass = (*device)->createRenderPass(renderPassInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void vkWndRender::createDescriptorSetLayout() {
  auto device = get_device();
  auto descriptorPool = get_descriptor_pool();
  auto textureImageView = get_texture_image_view();
  auto textureSampler = get_sampler();

  assert(device != nullptr);
  assert(descriptorPool);
  assert(textureImageView);
  assert(textureSampler);

  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  bindings.resize(2);
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  bindings[0].pImmutableSamplers = nullptr;
  bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

  // CombinedImageSampler (Depth buffer) for PixelShader
  bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
  bindings[1].descriptorCount = 1;
  bindings[1].binding = 1;
  bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
  bindings[1].pImmutableSamplers = nullptr;

  try {
    // レイアウトを生成
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    std::cout << "createDescriptorsetLayout" << std::endl;
    descriptorSetLayout = (*device)->createDescriptorSetLayout(layoutInfo, nullptr);

    // デスクリプタセットは作成済みのデスクリプタプールから確保する
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = *descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    std::cout << "allocateDescriptorSets" << std::endl;
    descriptorSet = (*device)->allocateDescriptorSets(allocInfo);

    // デスクリプタセットの情報を更新する
    vk::DescriptorBufferInfo g_VkUniformInfo;
    g_VkUniformInfo.buffer = uniformBuffer.buf;
    g_VkUniformInfo.offset = 0;
    g_VkUniformInfo.range = VK_WHOLE_SIZE; // sizeof(float)*16; // VK_WHOLE_SIZE

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eGeneral;
    imageInfo.imageView = *textureImageView;
    imageInfo.sampler = *textureSampler;

    std::vector<vk::WriteDescriptorSet> descSetInfos;
    descSetInfos.resize(2);
    // descSetInfos.resize(1);
    descSetInfos[0].dstSet = descriptorSet[0];
    descSetInfos[0].descriptorCount = 1;
    descSetInfos[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descSetInfos[0].pBufferInfo = &g_VkUniformInfo;
    descSetInfos[0].dstArrayElement = 0;
    descSetInfos[0].dstBinding = 0;

    descSetInfos[1].dstSet = descriptorSet[0];
    descSetInfos[1].dstBinding = 1;
    descSetInfos[1].dstArrayElement = 0;
    descSetInfos[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descSetInfos[1].descriptorCount = 1;
    descSetInfos[1].pImageInfo = &imageInfo;

    std::cout << "updateDescriptorSets" << std::endl;
    (*device)->updateDescriptorSets(descSetInfos, nullptr);
    std::cout << "end" << std::endl;
  } catch(vk::SystemError& err) {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit(-1);
  } catch(std::exception& e) {
    std::cerr << "Exception caught : " << e.what() << std::endl;
  } catch(...) {
    std::cout << "errrrrrr" << std::endl;
    exit(-1);
  }
}

void vkWndRender::createGraphicsPipeline() {
  const auto device = get_device();
  const auto vertexShader = get_vert_shader();
  const auto fragmentShader = get_frag_shader();

  vk::PipelineShaderStageCreateInfo shaderStages[] = {{vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, (*vertexShader).get(), "main"},
                                                      {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, (*fragmentShader).get(), "main"}};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  auto bindingDescription = getBindingDescription();
  auto attributeDescriptions = getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
  // inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Viewoortに関する設定
  vk::PipelineViewportStateCreateInfo viewportState = {};
  // {
  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor = {};
  scissor.offset = vk::Offset2D(0, 0);
  scissor.extent = swapChainExtent;

  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
  // }

  vk::PipelineRasterizationStateCreateInfo rasterizer = {};
  // {
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = VK_FALSE;
  // }

  vk::PipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendStateCreateInfo colorBlending = {};
  // {
  vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_FALSE;

  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;
  // }

  // vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
  // pipelineLayoutInfo.setLayoutCount = 0;
  // pipelineLayoutInfo.pushConstantRangeCount = 0;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout);
  // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // pipelineLayoutInfo.setLayoutCount = 1;
  // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  try {
    pipelineLayout = (*device)->createPipelineLayout(pipelineLayoutInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = nullptr;

  vk::ResultValue<vk::UniquePipeline> result = (*device)->createGraphicsPipelineUnique({}, pipelineInfo);
  if(result.result == vk::Result::eSuccess) {
    graphicsPipeline = std::move(result.value);
  } else {
    throw std::runtime_error("failed to create a pipeline!");
  }
}


void vkWndRender::createGraphicsPipeline_UI() {
  auto device = get_device();
  auto vertexShader = get_vert_shader_ui();
  auto fragmentShader = get_frag_shader_ui();

  vk::PipelineShaderStageCreateInfo shaderStages[] = {{vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, (*vertexShader).get(), "main"},
                                                      {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, (*fragmentShader).get(), "main"}};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  auto bindingDescription = getBindingDescription_UI();
  auto attributeDescriptions = getAttributeDescriptions_UI();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
  // inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Viewoortに関する設定
  vk::PipelineViewportStateCreateInfo viewportState = {};
  // {
  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor = {};
  scissor.offset = vk::Offset2D(0, 0);
  scissor.extent = swapChainExtent;

  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
  // }

  vk::PipelineRasterizationStateCreateInfo rasterizer = {};
  // {
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = VK_FALSE;
  // }

  vk::PipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendStateCreateInfo colorBlending = {};
  // {
  vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
  colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachment.alphaBlendOp = vk::BlendOp::eSubtract;

  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;
  // }

  // vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
  // pipelineLayoutInfo.setLayoutCount = 0;
  // pipelineLayoutInfo.pushConstantRangeCount = 0;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout);
  // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // pipelineLayoutInfo.setLayoutCount = 1;
  // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  std::cout << "create createPipelineLayout" << std::endl;
  try {
    pipelineLayout_ui = (*device)->createPipelineLayout(pipelineLayoutInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout_ui;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = nullptr;

  vk::ResultValue<vk::UniquePipeline> result = (*device)->createGraphicsPipelineUnique({}, pipelineInfo);
  if(result.result == vk::Result::eSuccess) {
    graphicsPipeline_ui = std::move(result.value);
  } else {
    throw std::runtime_error("failed to create a pipeline!");
  }
}



vk::SurfaceFormatKHR vkWndRender::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
  if(availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for(const auto& availableFormat : availableFormats) {
    if(availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

vk::PresentModeKHR vkWndRender::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes) {
  vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

  for(const auto& availablePresentMode : availablePresentModes) {
    if(availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    } else if(availablePresentMode == vk::PresentModeKHR::eImmediate) {
      bestMode = availablePresentMode;
    }
  }
  return bestMode;
}

vk::Extent2D vkWndRender::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
  if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    const auto window = getDrawingWindow();
    glfwGetFramebufferSize(window->getGLFWwindow(), &width, &height);
    vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
  }
}

void vkWndRender::createFramebuffers() {
  auto device = get_device();
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for(size_t i = 0; i < swapChainImageViews.size(); i++) {
    vk::ImageView attachments[] = {swapChainImageViews[i]};

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    try {
      swapChainFramebuffers[i] = (*device)->createFramebuffer(framebufferInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}


/* void vkWndRender::createCommandBuffers(bool force = false) { */
void vkWndRender::createCommandBuffers(const DrawData* dd) {
  auto device = get_device();
  auto commandPool = get_command_pool();
  commandBuffers.resize(swapChainFramebuffers.size());

  static bool command_buffer_created = false;
  static size_t last_vertex_size = 0;
  static size_t last_vertex_ui_size = 0;

  if(!command_buffer_created) {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = *commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    try {
      commandBuffers = (*device)->allocateCommandBuffers(allocInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
    command_buffer_created = true;
  }

  /* if(last_vertex_size == dd->vertices.size() && last_vertex_ui_size == dd->vertices_ui.size() && !force) return; */
  if(last_vertex_size == dd->vertices.size() && last_vertex_ui_size == dd->vertices_ui.size()) return;

  last_vertex_size = dd->vertices.size();
  last_vertex_ui_size = dd->vertices_ui.size();

  for(size_t i = 0; i < commandBuffers.size(); i++) {
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

    try {
      commandBuffers[i].begin(beginInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    vk::ClearValue clearColor = {std::array<float, 4>{0.5f, 0.01f, 0.05f, 1.0f}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.get());
    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
    // vk::Buffer vertexBuffers[] = { vertexBuffer.buf, vertexBuffer_ui.buf  };
    vk::Buffer vertexBuffers[] = {vertexBuffer.buf};
    vk::DeviceSize offsets[] = {0};
    commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffers[i].draw(static_cast<uint32_t>(dd->vertices.size()), 1, 0, 0);
    // commandBuffers[i].draw(VK_WHOLE_SIZE, 1, 0, 0);

    commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline_ui.get());
    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_ui, 0, descriptorSet, nullptr);
    vk::Buffer vertexBuffers2[] = {vertexBuffer_ui.buf};
    commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers2, offsets);

    // for(auto&& [[maybe_unused]]d : dd->drawlist) {
      /* commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_ui, d.tex_id, descriptorSet, nullptr); */
      commandBuffers[i].draw(static_cast<uint32_t>(dd->vertices_ui.size()), 1, 0, 0);
    // }
    // for(int i=0; i<dd->vertices_ui.size(); i++){
    //     std::cout << dd->vertices_ui[i].pos[0] << ", " << vertices_ui[i].pos[1] << std::endl;
    // }
    commandBuffers[i].endRenderPass();
    try {
      commandBuffers[i].end();
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void vkWndRender::cleanupSwapChain() {
  auto device = get_device();
  // auto commandPool = getCommandPool();
  for(auto framebuffer : swapChainFramebuffers) {
    (*device)->destroyFramebuffer(framebuffer);
  }

  // (*device)->freeCommandBuffers(*commandPool, commandBuffers);

  // (*device)->destroyPipeline(graphicsPipeline);
  (*device)->destroyPipelineLayout(pipelineLayout);
  (*device)->destroyRenderPass(renderPass);

  for(auto imageView : swapChainImageViews) {
    (*device)->destroyImageView(imageView);
  }

  (*device)->destroySwapchainKHR(swapChain);
}

void vkWndRender::recreateSwapChain() {
  auto device = get_device();

  int width = 0, height = 0;
  while(width == 0 || height == 0) {
    glfwGetFramebufferSize(getDrawingWindow()->getGLFWwindow(), &width, &height);
    glfwWaitEvents();
  }

  (*device)->waitIdle();

  cleanupSwapChain();
  createSwapChain();
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createGraphicsPipeline_UI();
  createFramebuffers();
  /* createCommandBuffers(true); */
}


void vkWndRender::init() {
  std::cout << "createSwapchain" << std::endl;
  createSwapChain();
  std::cout << "createImageView" << std::endl;
  createImageViews();
  std::cout << "descriptorlayout" << std::endl;
  createUniformBuffer();
  createDescriptorSetLayout();
  std::cout << "renderpass" << std::endl;
  createRenderPass();
  std::cout << "pipeline" << std::endl;
  createGraphicsPipeline();
  std::cout << "pipeline_ui" << std::endl;
  createGraphicsPipeline_UI();
  std::cout << "framebuffer" << std::endl;
  createFramebuffers();
  std::cout << "commandbuffer" << std::endl;
  /* createCommandBuffers(); */
}

void vkWndRender::update_wndsize() {
  recreateSwapChain();
}

void vkWndRender::createUniformBuffer() {
  static bool _created_uniform_buffer = false;
  const vk::DeviceSize bufferSize = sizeof(UniformData);
  if(!_created_uniform_buffer) {
    uniformBuffer.create(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    _created_uniform_buffer = true;
  }
  UniformData tmp;
  uniformBuffer.copyData((void*)&tmp, sizeof(UniformData));
}

void vkWndRender::draw(DrawData* dd) {
  static uint64_t currentFrame = 0;
  currentFrame++;
  auto currentFrame_2 = currentFrame % VK_ENGINE_MAX_FRAMES_IN_FLIGHT;
  const bool verbose = true;
  {
    if(dd->vertices.size() == 0) {
      dd->vertices.push_back(Vertex({0, 0, 0}, {0, 0, 0}));
      dd->vertices.push_back(Vertex({0, 0, 0}, {0, 0, 0}));
      dd->vertices.push_back(Vertex({0, 0, 0}, {0, 0, 0}));
    }

    if(dd->vertices_ui.size() == 0) {
      dd->vertices_ui.push_back(VertexUI({0, 0}, {0, 0, 0}, {0, 0}));
      dd->vertices_ui.push_back(VertexUI({0, 0}, {0, 0, 0}, {0, 0}));
      dd->vertices_ui.push_back(VertexUI({0, 0}, {0, 0, 0}, {0, 0}));
    }

    static bool _created_vertex_buffer = false;
    if(!_created_vertex_buffer) {
      const vk::DeviceSize bufferSize = dd->vertices.size_in_bytes();
      vertexBuffer.create(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      const vk::DeviceSize bufferSize2 = dd->vertices_ui.size_in_bytes();
      vertexBuffer_ui.create(bufferSize2, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      _created_vertex_buffer = true;
    }
    vertexBuffer.copyData((void*)dd->vertices.data(), dd->vertices.size_in_bytes());
    vertexBuffer_ui.copyData((void*)dd->vertices_ui.data(), dd->vertices_ui.size_in_bytes());
  }

  assert(uniformBuffer.iscreated());
  uniformBuffer.copyData((void*)&dd->uniform_data, sizeof(UniformData));
  if(verbose) {
    std::cout << "vertex size(3d, 2d, 2d_overlay) = " << dd->vertices.size() << ", " << dd->vertices_ui.size() << std::endl;
  }

  // updateVertexBuffer();
  createCommandBuffers(dd);

  auto device = get_device();
  auto presentQueue = get_presense_queue();
  auto graphicsQueue = get_graphics_queue();
  auto imageAvailableSemaphores = get_image_availabele_semaphos();
  auto renderFinishedSemaphores = get_render_finished_semaphos();
  auto inFlightFences = get_fences();
  assert(imageAvailableSemaphores->size() > currentFrame_2);
  assert(renderFinishedSemaphores->size() > currentFrame_2);
  assert(inFlightFences->size() > currentFrame_2);

  auto result = (*device)->waitForFences(1, &(*inFlightFences)[currentFrame_2], VK_TRUE, std::numeric_limits<uint64_t>::max());
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to waitForFences!");
  }
  uint32_t imageIndex;
  try {
    auto result = (*device)->acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), (*imageAvailableSemaphores)[currentFrame_2], nullptr);
    imageIndex = result.value;
  } catch(const vk::OutOfDateKHRError& err) {
    recreateSwapChain();
    return;
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vk::SubmitInfo submitInfo = {};

  vk::Semaphore waitSemaphores[] = {(*imageAvailableSemaphores)[currentFrame_2]};
  vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

  vk::Semaphore signalSemaphores[] = {(*renderFinishedSemaphores)[currentFrame_2]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;
  result = (*device)->resetFences(1, &(*inFlightFences)[currentFrame_2]);
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to submit resetFences!");
  }

  try {
    graphicsQueue->submit(submitInfo, (*inFlightFences)[currentFrame_2]);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  vk::PresentInfoKHR presentInfo = {};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  vk::SwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  vk::Result resultPresent;
  try {
    resultPresent = presentQueue->presentKHR(presentInfo);
  } catch(const vk::OutOfDateKHRError& err) {
    resultPresent = vk::Result::eErrorOutOfDateKHR;
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  if(resultPresent == vk::Result::eSuboptimalKHR || resultPresent == vk::Result::eSuboptimalKHR ) { 
    // framebufferResized = false; 
    // recreateSwapChain(); 
    return; 
  }
}

} // namespace vkUI::Render
