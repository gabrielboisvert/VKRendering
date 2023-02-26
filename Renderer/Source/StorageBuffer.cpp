#include "StorageBuffer.h"
#include "Scene.h"

using namespace Renderer;

StorageBuffer::StorageBuffer(VKRenderer& pRenderer) : mRenderer(pRenderer)
{
}

void Renderer::StorageBuffer::init(VkShaderStageFlagBits pStage)
{
	createStorageBuffers();
	createDescriptor(pStage);
}

StorageBuffer::~StorageBuffer()
{
	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(mRenderer.mDevice, mDirectionalLightStorageBuffers[i], nullptr);
		vkFreeMemory(mRenderer.mDevice, mDirectionalLightStorageBuffersMemory[i], nullptr);

		vkDestroyBuffer(mRenderer.mDevice, mPointLightStorageBuffers[i], nullptr);
		vkFreeMemory(mRenderer.mDevice, mPointLightStorageBuffersMemory[i], nullptr);

		vkDestroyBuffer(mRenderer.mDevice, mSpotLightStorageBuffers[i], nullptr);
		vkFreeMemory(mRenderer.mDevice, mSpotLightStorageBuffersMemory[i], nullptr);
	}
}

void StorageBuffer::createStorageBuffers()
{
	VkDeviceSize bufferSize1 = mRenderer.padStorageBufferSize(sizeof(DirLights));
	VkDeviceSize bufferSize2 = mRenderer.padStorageBufferSize(sizeof(PointLight));
	VkDeviceSize bufferSize3 = mRenderer.padStorageBufferSize(sizeof(SpotLight));

	VkDeviceSize bufferSize = bufferSize1 + bufferSize2 + bufferSize3;


	mDirectionalLightStorageBuffers.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mDirectionalLightStorageBuffersMemory.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mDirectionalLightStorageBuffersMapped.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);

	mPointLightStorageBuffers.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mPointLightStorageBuffersMemory.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mPointLightStorageBuffersMapped.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);

	mSpotLightStorageBuffers.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mSpotLightStorageBuffersMemory.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	mSpotLightStorageBuffersMapped.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
	{
		mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mDirectionalLightStorageBuffers[i], mDirectionalLightStorageBuffersMemory[i]);
		vkMapMemory(mRenderer.mDevice, mDirectionalLightStorageBuffersMemory[i], 0, bufferSize1, 0, &mDirectionalLightStorageBuffersMapped[i]);

		mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mPointLightStorageBuffers[i], mPointLightStorageBuffersMemory[i]);
		vkMapMemory(mRenderer.mDevice, mPointLightStorageBuffersMemory[i], bufferSize1, bufferSize2, 0, &mPointLightStorageBuffersMapped[i]);

		mRenderer.createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mSpotLightStorageBuffers[i], mSpotLightStorageBuffersMemory[i]);
		vkMapMemory(mRenderer.mDevice, mSpotLightStorageBuffersMemory[i], bufferSize1 + bufferSize2, bufferSize3, 0, &mSpotLightStorageBuffersMapped[i]);
	}
}

void StorageBuffer::createDescriptor(VkShaderStageFlagBits pStage)
{
	VkDeviceSize bufferSize1 = mRenderer.padStorageBufferSize(sizeof(DirLights));
	VkDeviceSize bufferSize2 = mRenderer.padStorageBufferSize(sizeof(PointLight));
	VkDeviceSize bufferSize3 = mRenderer.padStorageBufferSize(sizeof(SpotLight));

	DescriptorSets.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo1{};
		bufferInfo1.buffer = mDirectionalLightStorageBuffers[i];
		bufferInfo1.offset = 0;
		bufferInfo1.range = bufferSize1;

		VkDescriptorBufferInfo bufferInfo2{};
		bufferInfo2.buffer = mPointLightStorageBuffers[i];
		bufferInfo2.offset = bufferSize1;
		bufferInfo2.range = bufferSize2;

		VkDescriptorBufferInfo bufferInfo3{};
		bufferInfo3.buffer = mSpotLightStorageBuffers[i];
		bufferInfo3.offset = bufferSize1 + bufferSize2;
		bufferInfo3.range = bufferSize3;

		vkutil::DescriptorBuilder::begin(mRenderer.mDescriptorLayoutCache, mRenderer.mDescriptorAllocator)
			.bind_buffer(0, &bufferInfo1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pStage)
			.bind_buffer(1, &bufferInfo2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pStage)
			.bind_buffer(2, &bufferInfo3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pStage)
			.build(DescriptorSets[i]);
	}
}