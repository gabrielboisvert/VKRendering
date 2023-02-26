#pragma once
#include "VKRenderer.h"

namespace Renderer
{

	class UniformBuffer
	{
		public:
			VKRenderer& mRenderer;
			std::vector<VkBuffer> mUniformBuffers;
			std::vector<VkDeviceMemory> mUniformBuffersMemory;
			std::vector<void*> mUniformBuffersMapped;

			std::vector<VkDescriptorSet> mDescriptorSets;


			UniformBuffer(VKRenderer& pRenderer);
			~UniformBuffer();

			void init(unsigned int pSize, VkShaderStageFlagBits pStage);
			void createUniformBuffers(unsigned int pSize);
			void createDescriptor(unsigned int pSize, VkShaderStageFlagBits pStage);
	};
}