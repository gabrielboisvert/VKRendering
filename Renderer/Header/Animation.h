#pragma once
#include "Model.h"

namespace Renderer
{
    class Model;

    struct AssimpNodeData
    {
        lm::mat4 mTransformation;
        std::string mName;
        int mChildrenCount;
        std::vector<AssimpNodeData> mChildren;
    };

    class Animation
    {
        public:
            float mDuration = 0;
            int mTicksPerSecond = 0;
            std::vector<Bone> mBones;
            AssimpNodeData mRootNode;
            std::map<std::string, BoneInfo> mBoneInfoMap;

            Animation(const std::string& pAnimationPath, Model* pModel);
            Animation(const aiScene* pScene, Model* pModel);
            ~Animation();

            Bone* findBone(const std::string& pName);
            void readMissingBones(const aiAnimation* pAnimation, Model& pModel);
            void readHeirarchyData(AssimpNodeData& pDest, const aiNode* pSrc);
    };
}