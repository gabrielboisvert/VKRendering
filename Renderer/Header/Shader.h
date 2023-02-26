#pragma once
#include "Texture.h"

namespace Renderer
{
	class Shader : public IResource
	{
		public:
			VKRenderer& mRenderer;
			VkPipeline mGraphicsPipeline = nullptr;
			VkPipelineLayout mPipelineLayout = nullptr;

			VkDescriptorSetLayout mGlobalSetLayout;
			VkDescriptorSetLayout mObjectSetLayout;
			VkDescriptorSetLayout mSingleTextureSetLayout;

			Texture* mDefaultTexture = nullptr;

			Shader(VKRenderer& pRenderer, const char* pVertex, const char* pFragment);
			~Shader() override;

			static std::vector<char> readFile(const std::string& pFilename);
			VkShaderModule createShaderModule(const std::vector<char>& pCode);
			void createGraphicsPipeline(const char* pVertex, const char* pFragment);
			void createDescriptorSetLayout();


			void bind();
			void setLight(VkDescriptorSet* pDescriptor, void* pUniformBuffer, void* pData, size_t pSize);
			void setTexture(const VkDescriptorSet& pDescriptor);
			void setMVP(const VkDescriptorSet& pDescriptor, void* pUniformBuffer, void* pData, size_t pSize);
	};
}