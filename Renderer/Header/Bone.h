#pragma once
#include <assimp/anim.h>
#include <vector>
#include "Quat/Quat.h"

namespace Renderer
{
    struct BoneInfo
    {
        int mId;
        lm::mat4 mOffset;
    };

    struct KeyPosition
    {
        lm::vec3 mPosition;
        float mTimeStamp;
    };

    struct KeyRotation
    {
        lm::quat mOrientation;
        float mTimeStamp;
    };

    struct KeyScale
    {
        lm::vec3 mScale;
        float mTimeStamp;
    };

    class Bone
    {
    public:
        std::vector<KeyPosition> mPositions;
        std::vector<KeyRotation> mRotations;
        std::vector<KeyScale> mScales;
        int mNumPositions;
        int mNumRotations;
        int mNumScalings;

        lm::mat4 mLocalTransform;
        std::string mName;
        int mID;

        Bone(const std::string& pName, int pId, const aiNodeAnim* pChannel);

        void update(float pAnimationTime);

        int getPositionIndex(float pAnimationTime);
        int getRotationIndex(float pAnimationTime);
        int getScaleIndex(float pAnimationTime);
        float getScaleFactor(float pLastTimeStamp, float pNextTimeStamp, float pAnimationTime);

        lm::mat4 interpolatePosition(float pAnimationTime);
        lm::mat4 interpolateRotation(float pAnimationTime);
        lm::mat4 interpolateScaling(float pAnimationTime);

        lm::vec3 getVec(const aiVector3D& pVec);
        lm::quat getQuat(const aiQuaternion& pOrientation);
    };
}