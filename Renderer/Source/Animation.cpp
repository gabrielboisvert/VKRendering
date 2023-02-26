#include "Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace Renderer;

Animation::Animation(const std::string& pAnimationPath, Model* pModel)
{
    if (pModel == nullptr)
        return;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(pAnimationPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_LimitBoneWeights);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    if (scene->HasAnimations())
    {
        aiAnimation* animation = scene->mAnimations[0];
        mDuration = animation->mDuration;
        mTicksPerSecond = animation->mTicksPerSecond;
        readHeirarchyData(mRootNode, scene->mRootNode);
        readMissingBones(animation, *pModel);
    }
}

Animation::Animation(const aiScene* pScene, Model* pModel)
{
    if (pModel == nullptr)
        return;

    if (pScene->HasAnimations())
    {
        aiAnimation* animation = pScene->mAnimations[0];
        mDuration = animation->mDuration;
        mTicksPerSecond = animation->mTicksPerSecond;
        readHeirarchyData(mRootNode, pScene->mRootNode);
        readMissingBones(animation, *pModel);
    }
}

Animation::~Animation() {}

Bone* Animation::findBone(const std::string& pName)
{
    std::vector<Bone>::iterator iter = std::find_if(mBones.begin(), mBones.end(),
        [&](const Bone& Bone) {
            return Bone.mName == pName;
        }
    );

    if (iter == mBones.end())
        return nullptr;
    else
        return &(*iter);
}

void Animation::readMissingBones(const aiAnimation* pAnimation, Model& pModel)
{
    int size = pAnimation->mNumChannels;

    std::map<std::string, BoneInfo>& boneInfoMap = pModel.mBoneInfoMap;
    int& boneCount = pModel.mBoneCounter;


    for (int i = 0; i < size; i++)
    {
        aiNodeAnim* channel = pAnimation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].mId = boneCount;
            boneCount++;
        }
        mBones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].mId, channel));
    }

    mBoneInfoMap = boneInfoMap;
}

void Animation::readHeirarchyData(AssimpNodeData& pDest, const aiNode* pSrc)
{
    if (pSrc == nullptr)
        return;

    pDest.mName = pSrc->mName.data;
    pDest.mTransformation = Model::convertMatrix(pSrc->mTransformation);
    pDest.mChildrenCount = pSrc->mNumChildren;

    for (int i = 0; i < pSrc->mNumChildren; i++)
    {
        AssimpNodeData newData;
        readHeirarchyData(newData, pSrc->mChildren[i]);
        pDest.mChildren.push_back(newData);
    }
}