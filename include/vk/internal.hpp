#pragma once
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <drawdata.hpp>
#include <iostream>
#include <optional>

namespace vkUI::Render {
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

// -----------------------------------------------------
//    [SECTION] Buffer
// -----------------------------------------------------
struct uiBuffer {
  vk::Buffer buf, staging_buf;
  vk::DeviceMemory buf_mem, staging_buf_mem;
  vk::UniqueDevice* device_ptr;
  vk::MemoryPropertyFlags prop;
  vk::DeviceSize buf_size;
  vk::BufferUsageFlags usage;
  bool allocated;
  bool use_staging_buffer;

  uiBuffer();
  uiBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
  ~uiBuffer();
  inline void setUseStagingBuf(bool v) { use_staging_buffer = v; }
  void create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
  void __create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer* buf_p, vk::DeviceMemory* mem_p);
  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
  static std::vector<char> readFile(const std::string& filename);
  vk::CommandBuffer __beginSingleTimeCommands();
  void __endSingleTimeCommands(vk::CommandBuffer commandBuffer);
  void __copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

  void copyData(const void* src, const size_t size);
  void cleanup();
};

struct vkWndRender {
private:
  vk::SurfaceKHR surface;
  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> swapChainImages;
  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::ImageView> swapChainImageViews;
  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::RenderPass renderPass;
  vk::PipelineLayout pipelineLayout, pipelineLayout_ui;
  vk::UniquePipeline graphicsPipeline, graphicsPipeline_ui;
  std::vector<vk::DescriptorSet> descriptorSet;
  vk::DescriptorSetLayout descriptorSetLayout;
  std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> commandBuffers;
  uiBuffer uniformBuffer, vertexBuffer, vertexBuffer_ui;
  void cleanupSwapChain();
  void recreateSwapChain();

  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createCommandBuffers(const Engine::DrawData*);
  void createFramebuffers();
  void createGraphicsPipeline_UI();
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);


  static vk::VertexInputBindingDescription getBindingDescription() {
    vk::VertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Engine::Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;
    return bindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
#ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
#else
    attributeDescriptions[0].format = vk::Format::eR16G16B16Sint;
#endif
    attributeDescriptions[0].offset = Engine::Vertex::get_offset_pos();

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR8G8B8Uint;
    attributeDescriptions[1].offset = Engine::Vertex::get_offset_col();

    //         attributeDescriptions[2].binding = 0;
    //         attributeDescriptions[2].location = 2;
    // #ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
    //         attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
    // #else
    //         attributeDescriptions[2].format = vk::Format::eR16G16Uint;
    // #endif
    //         attributeDescriptions[2].offset = offsetof(Vertex, uv);
    return attributeDescriptions;
  }


  static vk::VertexInputBindingDescription getBindingDescription_UI() {
    vk::VertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Engine::VertexUI);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;
    return bindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions_UI() {
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR16G16Sint;
    attributeDescriptions[0].offset = Engine::VertexUI::get_offset_pos();

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR8G8B8Uint;
    attributeDescriptions[1].offset = Engine::VertexUI::get_offset_col();

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR16G16Uint;
    attributeDescriptions[2].offset = Engine::VertexUI::get_offset_uv();

    return attributeDescriptions;
  }

public:
  /* void updateVertexBuffer(); */
  inline vk::SurfaceKHR getSurface() {
    return surface;
  }

  void init();
  void update_wndsize();
  void draw(::vkUI::Engine::DrawData* dd);
  void terminate();
  void createSurface(GLFWwindow* window);
  SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device);
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};
class vkRender {
private:
  vk::UniqueInstance instance;
  VkDebugUtilsMessengerEXT callback;
  vk::PhysicalDevice physicalDevice;
  vk::UniqueDevice device;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::DescriptorPool descriptorPool;
  vk::CommandPool commandPool;
  vk::Image TextureImage;
  vk::DeviceMemory textureImageMemory;
  vk::ImageView textureImageView;
  vk::Sampler textureSampler;
  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  vk::UniqueShaderModule vertexShader, fragmentShader, vertexShader_ui, fragmentShader_ui;


  void createCommandPool();
  std::vector<const char*> getRequiredExtensions();
  vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);
  bool isDeviceSuitable(const vk::PhysicalDevice& device);
  bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device);
  bool checkValidationLayerSupport();
  void createSyncObjects();
  void createDescriptorPool();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createInstance();
  void setupDebugCallback();
  vk::UniqueShaderModule createShader(std::string filename);

  void createTextureImage();
  void transitionImageLayout(vk::Image image, [[maybe_unused]] vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
      barrier.srcAccessMask = (vk::AccessFlags)0;
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
      sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
      destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
      sourceStage = vk::PipelineStageFlagBits::eTransfer;
      destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
      throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), {}, {}, barrier);
    endSingleTimeCommands(commandBuffer);
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
  }
  vk::CommandBuffer beginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    vk::CommandBuffer cmdbuf;
    try {
      auto _commandBuffers = device->allocateCommandBuffers(allocInfo);
      assert(_commandBuffers.size() > 0);
      cmdbuf = _commandBuffers[0];
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to allocate command buffers!");
    }

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    auto result = cmdbuf.begin(&beginInfo);
    if(result != vk::Result::eSuccess) {
      throw std::runtime_error("failed to begine command buffer!");
    }
    std::cout << "begin" << std::endl;
    return cmdbuf;
  }

  void endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();
    device->freeCommandBuffers(commandPool, 1, &commandBuffer);
  }
  void createShaders() {
    vertexShader = createShader("../shaders/vert.spv");
    fragmentShader = createShader("../shaders/frag.spv");

    vertexShader_ui = createShader("../shaders/vert_ui.spv");
    fragmentShader_ui = createShader("../shaders/frag_ui.spv");
  }

public:
  void init();
  void terminate();
  auto get_device_ptr() { return &device; }
  auto get_physical_device_ptr() { return &physicalDevice; }
  auto get_instance_ptr() { return &instance; }
  auto get_frag_shader() { return &fragmentShader; }
  auto get_vert_shader() { return &vertexShader; }
  auto get_descriptor_pool() { return &descriptorPool; }
  auto get_texture_sampler() { return &textureSampler; }
  auto get_frag_ui_shader() { return &fragmentShader_ui; }
  auto get_vert_ui_shader() { return &vertexShader_ui; }
  auto get_texture_imageview() { return &textureImageView; }
  auto get_command_pool() { return &commandPool; }
  auto get_present_queue() { return &presentQueue; }
  auto get_graphics_queue() { return &graphicsQueue; }
  auto get_image_available_semaphos() { return &imageAvailableSemaphores; }
  auto get_render_finished_semaphos() { return &renderFinishedSemaphores; }
  auto get_fences() { return &inFlightFences; }
  QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
};


} // namespace vkUI::Render
