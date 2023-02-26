#include "UniformBuffer.h"

using namespace Renderer;

UniformBuffer::UniformBuffer(VKRenderer& pRenderer) : mRenderer(pRenderer)
{
}

void Renderer::UniformBuffer::init(unsigned int pSize, VkShaderStageFlagBits pStage)
{
	pSize = mRenderer.padUniformBufferSize(pSize);
	createUniformBuffers(pSize);
	createDescriptor(pSize, pStage);
}

UniformBuffer::~UniformBuffer()
{
	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(mRenderer.mDevice, mUniformBuffers[i], nullptr);
		vkFreeMemory(mRenderer.mDevice, mUniformBuffersMemory[i], nullptr);
	}
}

void UniformBuffer::createUniformBuffers(unsigned int pSize)
{
	VkDeviceSize bufferSize = pSize;

	mUniformBuffers.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mUniformBuffersMemory.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mUniformBuffersMapped.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
	{
		mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBuffersMemory[i]);
		vkMapMemory(mRenderer.mDevice, mUniformBuffersMemory[i], 0, bufferSize, 0, &mUniformBuffersMapped[i]);
	}
}

void UniformBuffer::createDescriptor(unsigned int pSize, VkShaderStageFlagBits pStage)
{
	mDescriptorSets.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = pSize;

		vkutil::DescriptorBuilder::begin(mRenderer.mDescriptorLayoutCache, mRenderer.mDescriptorAllocator)
			.bind_buffer(0, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pStage)
			.build(mDescriptorSets[i]);
	}
}