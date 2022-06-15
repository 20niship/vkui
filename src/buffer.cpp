#include "../include/engine.hpp"

namespace vkUI::Engine{

// -----------------------------------------------------
//    [SECTION] uiBuffer
// -----------------------------------------------------
uiBuffer::uiBuffer(){use_staging_buffer = false; }

uiBuffer::uiBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    use_staging_buffer = false; 
    create(size, usage, properties);
}

void uiBuffer::create(vk::DeviceSize _size, vk::BufferUsageFlags _usage, vk::MemoryPropertyFlags properties) {
    buf_size = _size;
    usage = _usage;
    prop = properties;
    
    if(use_staging_buffer){ 
        assert(_usage && static_cast<vk::BufferUsageFlags>(vk::BufferUsageFlagBits::eTransferDst)); 
        assert(properties == vk::MemoryPropertyFlagBits::eDeviceLocal); 
    }
    __create(_size, _usage, properties, &buf, &buf_mem);

    if(use_staging_buffer){ 
        __create(_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &staging_buf, &staging_buf_mem);
    }
    allocated = true;
}

void uiBuffer::__create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer *buf_p, vk::DeviceMemory *mem_p) {
    auto device = getDevicePtr();
    try {
    	vk::BufferCreateInfo bufInfo{};
        bufInfo.size = size;
        bufInfo.usage = usage;
        bufInfo.sharingMode = vk::SharingMode::eExclusive;
        *buf_p = (*device)->createBuffer(bufInfo);
    }catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to create buffer!");
    }

    vk::MemoryRequirements memRequirements = (*device)->getBufferMemoryRequirements(*buf_p);
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    try { *mem_p = (*device)->allocateMemory(allocInfo); }catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    (*device)->bindBufferMemory(buf, *mem_p, 0);
}

uint32_t uiBuffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto physicalDevice = getPhysicalDevicePtr();
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice->getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void uiBuffer::copyData(void *src, size_t size){
    if(size > buf_size ){
        cleanup();
        create(size, usage, prop);
    }
    assert(allocated == true);
    
    if(use_staging_buffer){
        auto device = getDevicePtr();
        void* ptr = (*device)->mapMemory(staging_buf_mem, 0, buf_size);
        memcpy(ptr, src, size);
        (*device)->unmapMemory(staging_buf_mem);
        __copyBuffer(staging_buf, buf, buf_size);
    }else{
        auto device = getDevicePtr();
        void* ptr = (*device)->mapMemory(buf_mem, 0, buf_size);
    // result = checkVkResult(vkMapMemory(mDevice, bo.mMemory.value(), 0, VK_WHOLE_SIZE, 0, &p));
        memcpy(ptr, src, size);
        (*device)->unmapMemory(buf_mem);
    }
}

vk::CommandBuffer uiBuffer::__beginSingleTimeCommands() {
    const auto commandPool = getCommandPool();
    const auto device = getDevicePtr();
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = *commandPool;
    allocInfo.commandBufferCount = 1;
    vk::CommandBuffer cmdbuf;
    try {
        auto _commandBuffers = (*device)->allocateCommandBuffers(allocInfo);
        assert(_commandBuffers.size() > 0);
        cmdbuf = _commandBuffers[0];
    } catch (const vk::SystemError &err) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    auto result = cmdbuf.begin(&beginInfo);
    if(result != vk::Result::eSuccess){
        throw std::runtime_error("failed to begine command buffer!");
    }
    return cmdbuf;
}

void uiBuffer::__endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    const auto commandPool = getCommandPool();
    const auto device = getDevicePtr();
    const auto graphicsQueue = getGraphicsQueuePtr();
    commandBuffer.end();
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue->submit(submitInfo, nullptr);
    graphicsQueue->waitIdle();
    (*device)->freeCommandBuffers(*commandPool, 1, &commandBuffer);
}

// TODO: sizeパラメータはいらない？
void uiBuffer::__copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, [[maybe_unused]]vk::DeviceSize size) {
    auto cmdbuf = __beginSingleTimeCommands();
    vk::BufferCopy copyRegion{};
    // copyRegion.size = size;
    cmdbuf.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    __endSingleTimeCommands(cmdbuf);
}

void uiBuffer::cleanup(){
    auto device = getDevicePtr();
    (*device)->destroyBuffer(buf);
    (*device)->freeMemory(buf_mem);
    if(use_staging_buffer){
        (*device)->destroyBuffer(staging_buf);
        (*device)->freeMemory(staging_buf_mem);
    }
    allocated = false;
}
uiBuffer::~uiBuffer(){ cleanup(); }

} // namspace vkUI::Engine
