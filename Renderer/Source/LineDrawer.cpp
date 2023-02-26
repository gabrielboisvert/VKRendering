#include "LineDrawer.h"
#include <fstream>

using namespace Renderer;

VkVertexInputBindingDescription DebugVertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(DebugVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> DebugVertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(DebugVertex, mPosition);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(DebugVertex, mColor);

    return attributeDescriptions;
}

LineDrawer::LineDrawer(Camera& pCamera, VKRenderer& pRenderer, const char* pVertex, const char* pFragment)
    : mCamera(pCamera), mRenderer(pRenderer), mUniBuffer(mRenderer)
{
	createDescriptorSetLayout();
    mUniBuffer.init(sizeof(lm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
	createGraphicsPipeline(pVertex, pFragment);
}

LineDrawer::~LineDrawer()
{
	vkDestroyPipeline(mRenderer.mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mRenderer.mDevice, mPipelineLayout, nullptr);

    if (mVertexBuffer != nullptr)
        vkDestroyBuffer(mRenderer.mDevice, mVertexBuffer, nullptr);

    if (mVertexBufferMemory != nullptr)
        vkFreeMemory(mRenderer.mDevice, mVertexBufferMemory, nullptr);
}

std::vector<char> LineDrawer::readFile(const std::string& pFilename) {
    std::ifstream file(pFilename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule LineDrawer::createShaderModule(const std::vector<char>& pCode)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = pCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(pCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(mRenderer.mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    return shaderModule;
}

void LineDrawer::createDescriptorSetLayout()
{
    vkutil::DescriptorBuilder builder = vkutil::DescriptorBuilder::begin(mRenderer.mDescriptorLayoutCache, mRenderer.mDescriptorAllocator);

    //VP
    VkDescriptorSetLayoutBinding uboLayoutBinding = builder.descriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount = 1;
    setinfo.flags = 0;
    setinfo.pNext = nullptr;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = &uboLayoutBinding;

    mObjectSetLayout = mRenderer.mDescriptorLayoutCache->createDescriptorLayout(&setinfo);
}

void LineDrawer::createGraphicsPipeline(const char* pVertex, const char* pFragment)
{
    std::vector<char> vertShaderCode = readFile(pVertex);
    std::vector<char> fragShaderCode = readFile(pFragment);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription = DebugVertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = DebugVertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;// VK_CULL_MODE_BACK_BIT;
    //rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();


    VkDescriptorSetLayout setLayouts[] = { mObjectSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = setLayouts;

    if (vkCreatePipelineLayout(mRenderer.mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.renderPass = mRenderer.mRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(mRenderer.mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    vkDestroyShaderModule(mRenderer.mDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(mRenderer.mDevice, vertShaderModule, nullptr);
}

void LineDrawer::drawLine(const lm::vec3& pFrom, const lm::vec3& pTo, const lm::vec3& pColor)
{
    mLineVertice.emplace_back(DebugVertex(pFrom.X(), pFrom.Y(), pFrom.Z(), pColor.X(), pColor.Y(), pColor.Z()));
    mLineVertice.emplace_back(DebugVertex(pTo.X(), pTo.Y(), pTo.Z(), pColor.X(), pColor.Y(), pColor.Z()));
}

void LineDrawer::flushLines()
{
    if (mLineVertice.size() == 0)
        return;

    createVertexBuffer();

    vkCmdBindPipeline(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
    setVP(mUniBuffer.mDescriptorSets[mRenderer.mCurrentFrame], mUniBuffer.mUniformBuffersMapped[mRenderer.mCurrentFrame], &(mCamera.mVp[0][0]), sizeof(lm::mat4) );

    VkBuffer vertexBuffers[] = { mVertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], 0, 1, vertexBuffers, offsets);

    vkCmdDraw(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], static_cast<uint32_t>(mLineVertice.size()), 1, 0, 0);
}

void LineDrawer::setVP(const VkDescriptorSet& pDescriptor, void* pUniformBuffer, void* pData, size_t pSize)
{
    vkCmdBindDescriptorSets(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &pDescriptor, 0, nullptr);
    memcpy(pUniformBuffer, pData, pSize);
}

void LineDrawer::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(mLineVertice[0]) * mLineVertice.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mRenderer.mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mLineVertice.data(), (size_t)bufferSize);
    vkUnmapMemory(mRenderer.mDevice, stagingBufferMemory);

    mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);

    mRenderer.copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);
    vkDestroyBuffer(mRenderer.mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mRenderer.mDevice, stagingBufferMemory, nullptr);
}

void LineDrawer::reset()
{
    vkDeviceWaitIdle(mRenderer.mDevice);

    vkDestroyBuffer(mRenderer.mDevice, mVertexBuffer, nullptr);
    mVertexBuffer = nullptr;

    vkFreeMemory(mRenderer.mDevice, mVertexBufferMemory, nullptr);
    mVertexBufferMemory = nullptr;
}