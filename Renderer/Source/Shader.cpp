#include <fstream>
#include "Shader.h"
#include "Mesh.h"

using namespace Renderer;

Renderer::Shader::Shader(VKRenderer& pRenderer, const char* pVertex, const char* pFragment) : mRenderer(pRenderer)
{
    createDescriptorSetLayout();
    createGraphicsPipeline(pVertex, pFragment);

    mDefaultTexture = new Texture(mRenderer, "Assets/default.png");
}

Renderer::Shader::~Shader()
{
    vkDestroyPipeline(mRenderer.mDevice, mGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mRenderer.mDevice, mPipelineLayout, nullptr);

    delete mDefaultTexture;
}

std::vector<char> Shader::readFile(const std::string& pFilename) {
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

VkShaderModule Shader::createShaderModule(const std::vector<char>& pCode)
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

void Shader::createDescriptorSetLayout()
{
    vkutil::DescriptorBuilder builder = vkutil::DescriptorBuilder::begin(mRenderer.mDescriptorLayoutCache, mRenderer.mDescriptorAllocator);

    //Light
    VkDescriptorSetLayoutBinding g1lobalLayoutBinding = builder.descriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding g2lobalLayoutBinding = builder.descriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding g3lobalLayoutBinding = builder.descriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    std::array<VkDescriptorSetLayoutBinding, 3> layouts = { g1lobalLayoutBinding, g2lobalLayoutBinding, g3lobalLayoutBinding };

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount = layouts.size();
    setinfo.flags = 0;
    setinfo.pNext = nullptr;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = layouts.data();

    mGlobalSetLayout = mRenderer.mDescriptorLayoutCache->createDescriptorLayout(&setinfo);

    //MVP
    VkDescriptorSetLayoutBinding uboLayoutBinding = builder.descriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    VkDescriptorSetLayoutCreateInfo set2info = {};
    set2info.bindingCount = 1;
    set2info.flags = 0;
    set2info.pNext = nullptr;
    set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2info.pBindings = &uboLayoutBinding;

    mObjectSetLayout = mRenderer.mDescriptorLayoutCache->createDescriptorLayout(&set2info);

    //Texture
    VkDescriptorSetLayoutBinding textureBind = builder.descriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutCreateInfo set3info = {};
    set3info.bindingCount = 1;
    set3info.flags = 0;
    set3info.pNext = nullptr;
    set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set3info.pBindings = &textureBind;

    mSingleTextureSetLayout = mRenderer.mDescriptorLayoutCache->createDescriptorLayout(&set3info);
}

void Shader::createGraphicsPipeline(const char* pVertex, const char* pFragment)
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

    auto bindingDescription = Mesh::Vertex::getBindingDescription();
    auto attributeDescriptions = Mesh::Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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


    VkDescriptorSetLayout setLayouts[] = { mGlobalSetLayout, mObjectSetLayout, mSingleTextureSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 3;
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

void Shader::bind()
{
    for (int i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
        setTexture(mDefaultTexture->mTextureSets[mRenderer.mCurrentFrame]);

    vkCmdBindPipeline(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}

void Shader::setLight(VkDescriptorSet* pDescriptor, void* pUniformBuffer, void* pData, size_t pSize)
{
    vkCmdBindDescriptorSets(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, pDescriptor, 0, nullptr);
    memcpy(pUniformBuffer, pData, pSize);
}

void Shader::setMVP(const VkDescriptorSet& pDescriptor, void* pUniformBuffer, void* pData, size_t pSize)
{
    vkCmdBindDescriptorSets(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 1, 1, &pDescriptor, 0, nullptr);
    memcpy(pUniformBuffer, pData, pSize);
}

void Shader::setTexture(const VkDescriptorSet& pDescriptor)
{
    vkCmdBindDescriptorSets(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 2, 1, &pDescriptor, 0, nullptr);
}