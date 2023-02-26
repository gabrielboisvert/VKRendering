#include "Mesh.h"

using namespace Renderer;

VkVertexInputBindingDescription Mesh::Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 8> Mesh::Vertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, mPosition);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, mNormal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, mTextureUV);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, mColor);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, mTangent);

    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[5].offset = offsetof(Vertex, mBiTangent);

    attributeDescriptions[6].binding = 0;
    attributeDescriptions[6].location = 6;
    attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SINT;
    attributeDescriptions[6].offset = offsetof(Vertex, mBoneIDs);

    attributeDescriptions[7].binding = 0;
    attributeDescriptions[7].location = 7;
    attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[7].offset = offsetof(Vertex, mWeights);

    return attributeDescriptions;
}

Mesh::Mesh(VKRenderer& pRenderer, const std::vector<Vertex>& pVertex, const std::vector<uint32_t>& pIndice) : mPosition(pVertex), mIndices(pIndice), mRenderer(pRenderer)
{
    init();
}

void Mesh::init()
{
    createVertexBuffer();
    createIndexBuffer();
}

void Mesh::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(mPosition[0]) * mPosition.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mRenderer.mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mPosition.data(), (size_t)bufferSize);
    vkUnmapMemory(mRenderer.mDevice, stagingBufferMemory);

    mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);
    
    {
        std::unique_lock<std::mutex> lock(mRenderer.mMainThreadMuxtex);
        mRenderer.mMainThread.push([=]
            {
                mRenderer.copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);
                vkDestroyBuffer(mRenderer.mDevice, stagingBuffer, nullptr);
                vkFreeMemory(mRenderer.mDevice, stagingBufferMemory, nullptr);
            });
    }
}

void Mesh::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(mIndices[0]) * mIndices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mRenderer.mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mIndices.data(), (size_t)bufferSize);
    vkUnmapMemory(mRenderer.mDevice, stagingBufferMemory);

    mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

    {
        std::unique_lock<std::mutex> lock(mRenderer.mMainThreadMuxtex);
        mRenderer.mMainThread.push([=]
            {
                mRenderer.copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);
                vkDestroyBuffer(mRenderer.mDevice, stagingBuffer, nullptr);
                vkFreeMemory(mRenderer.mDevice, stagingBufferMemory, nullptr);
            });
    }
}

void Mesh::draw(Shader& pShader)
{
    VkBuffer vertexBuffers[] = { mVertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(mRenderer.mCommandBuffers[mRenderer.mCurrentFrame], static_cast<uint32_t>(mIndices.size()), 1, 0, 0, 0);
}

Mesh::~Mesh()
{
    vkDestroyBuffer(mRenderer.mDevice, mIndexBuffer, nullptr);
    vkFreeMemory(mRenderer.mDevice, mIndexBufferMemory, nullptr);

    vkDestroyBuffer(mRenderer.mDevice, mVertexBuffer, nullptr);
    vkFreeMemory(mRenderer.mDevice, mVertexBufferMemory, nullptr);
}