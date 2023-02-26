#pragma once
#include <VKRenderer.h>

namespace Renderer
{

	class StorageBuffer
	{
		public:
			VKRenderer& mRenderer;
			std::vector<VkBuffer> mDirectionalLightStorageBuffers;
			std::vector<VkDeviceMemory> mDirectionalLightStorageBuffersMemory;
			std::vector<void*> mDirectionalLightStorageBuffersMapped;

			std::vector<VkBuffer> mPointLightStorageBuffers;
			std::vector<VkDeviceMemory> mPointLightStorageBuffersMemory;
			std::vector<void*> mPointLightStorageBuffersMapped;

			std::vector<VkBuffer> mSpotLightStorageBuffers;
			std::vector<VkDeviceMemory> mSpotLightStorageBuffersMemory;
			std::vector<void*> mSpotLightStorageBuffersMapped;

			std::vector<VkDescriptorSet> DescriptorSets;

			StorageBuffer(VKRenderer& pRenderer);
			~StorageBuffer();

			void init(VkShaderStageFlagBits pStage);
			void createStorageBuffers();
			void createDescriptor(VkShaderStageFlagBits pStage);
	};
}