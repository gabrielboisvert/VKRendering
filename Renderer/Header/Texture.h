#pragma once
#include "VKRenderer.h"
#include "ResourceManager.h"

namespace Renderer
{
	class Texture : public IResource
	{
		public:
			uint32_t mMipLevels;
			VKRenderer& mRenderer;

			VkImage mTextureImage;
			VkImageView mTextureImageView;
			VkDeviceMemory mTextureImageMemory;
			VkSampler mTextureSampler;

			VkDescriptorImageInfo mImageInfo{};
			std::vector<VkDescriptorSet> mTextureSets;
			

			Texture(VKRenderer& pRenderer, const std::string& pFileName);
			~Texture() override;
			void createTextureImage(const std::string& pFileName);
			void copyBufferToImage(VkBuffer pBuffer, VkImage pImage, uint32_t pWidth, uint32_t pHeight);
			void createTextureImageView();
			void createTextureSampler();
			void generateMipmaps(VkImage pImage, VkFormat pImageFormat, int32_t pTexWidth, int32_t pTexHeight, uint32_t pMipLevels);

			void createDescriptor();
			void createDescritorSet();
	};
}