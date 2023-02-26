#include "Texture.h"

#ifndef STB_IMAGE_IMPLEMENTATION_
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image/stb_image.h"

using namespace Renderer;

Texture::Texture(VKRenderer& pRenderer, const std::string& pFileName) : mRenderer(pRenderer)
{
	createTextureImage(pFileName);
	createTextureImageView();
	createTextureSampler();
	createDescriptor();
	createDescritorSet();
}

Texture::~Texture()
{
	vkDestroySampler(mRenderer.mDevice, mTextureSampler, nullptr);
	vkDestroyImageView(mRenderer.mDevice, mTextureImageView, nullptr);
	vkDestroyImage(mRenderer.mDevice, mTextureImage, nullptr);
	vkFreeMemory(mRenderer.mDevice, mTextureImageMemory, nullptr);
}

void Texture::createTextureImage(const std::string& pFileName)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(pFileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels)
		throw std::runtime_error("failed to load texture image!");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	mRenderer.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mRenderer.mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(mRenderer.mDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	mRenderer.createImage(texWidth, texHeight, mMipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImage, mTextureImageMemory);
	
	
	
	
	std::unique_lock<std::mutex> lock(mRenderer.mMainThreadMuxtex);
	mRenderer.mMainThread.push([=]
		{
			mRenderer.transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mMipLevels);
			copyBufferToImage(stagingBuffer, mTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			vkDestroyBuffer(mRenderer.mDevice, stagingBuffer, nullptr);
			vkFreeMemory(mRenderer.mDevice, stagingBufferMemory, nullptr);

			generateMipmaps(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mMipLevels);
		});
	

	
}

void Texture::copyBufferToImage(VkBuffer pBuffer, VkImage pImage, uint32_t pWidth, uint32_t pHeight)
{
	VkCommandBuffer commandBuffer = mRenderer.beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { pWidth, pHeight, 1 };

	vkCmdCopyBufferToImage(commandBuffer, pBuffer, pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	mRenderer.endSingleTimeCommands(commandBuffer);
}

void Texture::createTextureImageView()
{
	mTextureImageView = mRenderer.createImageView(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels);
}

void Texture::createTextureSampler()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(mRenderer.mPhysicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mMipLevels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(mRenderer.mDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler!");
}

void Texture::createDescriptor()
{
	mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	mImageInfo.imageView = mTextureImageView;
	mImageInfo.sampler = mTextureSampler;
}

void Texture::createDescritorSet()
{
	mTextureSets.resize(VKRenderer::MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < VKRenderer::MAX_FRAMES_IN_FLIGHT; i++)
		vkutil::DescriptorBuilder::begin(mRenderer.mDescriptorLayoutCache, mRenderer.mDescriptorAllocator)
			.bind_image(0, &mImageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(mTextureSets[i]);
}

void Texture::generateMipmaps(VkImage pImage, VkFormat pImageFormat, int32_t pTexWidth, int32_t pTexHeight, uint32_t pMipLevels)
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(mRenderer.mPhysicalDevice, pImageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = mRenderer.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = pImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = pTexWidth;
	int32_t mipHeight = pTexHeight;

	for (uint32_t i = 1; i < pMipLevels; i++) 
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			pImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = pMipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	mRenderer.endSingleTimeCommands(commandBuffer);
}