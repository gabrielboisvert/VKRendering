#pragma once

#undef min
#undef max
#include <assimp/scene.h>
#include "Mesh.h"
#include <map>
#include "Bone.h"
#include "Animation.h"

namespace Renderer
{
	class Animation;

	class Model : public IResource
	{
		public:
			VKRenderer& mRenderer;
			std::vector<Mesh*> mMeshes;
			std::map<std::string, BoneInfo> mBoneInfoMap;
			int mBoneCounter = 0;

			Animation* mAnimation = nullptr;

			Model(VKRenderer& pRenderer, const std::string& pFilePath);
			~Model() override;

			void loadModel(const std::string& pPath);
			void processNode(const aiNode* pNode, const aiScene* pScene);
			void setVertexBoneDataToDefault(Mesh::Vertex& pVertex);
			void processMesh(const aiMesh* pMesh, const aiScene* pScene);
			void setVertexBoneData(Mesh::Vertex& pVertex, int pBoneId, float pWeight);
			void extractBoneWeightForVertices(std::vector<Mesh::Vertex>& pVertices, const aiMesh* pMesh, const aiScene* pScene);

			void draw(Shader& pShader);

			static lm::mat4 convertMatrix(const aiMatrix4x4& pFrom);
	};
}