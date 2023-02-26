#pragma once
#include "Vec2/Vec2.h"
#include "Shader.h"

#define MAX_BONE_INFLUENCE 4

namespace Renderer
{
	class Mesh
	{
	public:
		struct Vertex
		{
			lm::vec3 mPosition;
			lm::vec3 mNormal;
			lm::vec2 mTextureUV;
			lm::vec4 mColor;
			lm::vec3 mTangent;
			lm::vec3 mBiTangent;

			int mBoneIDs[MAX_BONE_INFLUENCE];
			float mWeights[MAX_BONE_INFLUENCE];

			static VkVertexInputBindingDescription getBindingDescription();
			static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions();
		};

		std::vector<Vertex> mPosition;
		std::vector<uint32_t> mIndices;

		VkBuffer mVertexBuffer;
		VkDeviceMemory mVertexBufferMemory;

		VkBuffer mIndexBuffer;
		VkDeviceMemory mIndexBufferMemory;

		VKRenderer& mRenderer;

		Mesh(VKRenderer& pRenderer, const std::vector<Vertex>& pVertex, const std::vector<uint32_t>& pIndice);
		void init();
		void createVertexBuffer();
		void createIndexBuffer();
		void draw(Shader& pShader);
		~Mesh();
	};
}