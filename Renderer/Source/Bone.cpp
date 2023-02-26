#include "Bone.h"

using namespace Renderer;

Bone::Bone(const std::string& pName, int pId, const aiNodeAnim* pChannel) :
    mName(pName),
    mID(pId)
{
    if (pChannel == nullptr)
        return;

    mNumPositions = pChannel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < mNumPositions; ++positionIndex)
    {
        aiVector3D aiPosition = pChannel->mPositionKeys[positionIndex].mValue;
        float timeStamp = pChannel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.mPosition = getVec(aiPosition);
        data.mTimeStamp = timeStamp;
        mPositions.push_back(data);
    }

    mNumRotations = pChannel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < mNumRotations; ++rotationIndex)
    {
        aiQuaternion aiOrientation = pChannel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = pChannel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.mOrientation = getQuat(aiOrientation);
        data.mTimeStamp = timeStamp;
        mRotations.push_back(data);
    }

    mNumScalings = pChannel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < mNumScalings; ++keyIndex)
    {
        aiVector3D scale = pChannel->mScalingKeys[keyIndex].mValue;
        float timeStamp = pChannel->mScalingKeys[keyIndex].mTime;
        KeyScale data;
        data.mScale = getVec(scale);
        data.mTimeStamp = timeStamp;
        mScales.push_back(data);
    }
}

void Bone::update(float pAnimationTime)
{
    lm::mat4 translation = interpolatePosition(pAnimationTime);
    lm::mat4 rotation = interpolateRotation(pAnimationTime);
    lm::mat4 scale = interpolateScaling(pAnimationTime);
    mLocalTransform = translation * rotation * scale;
}

int Bone::getPositionIndex(float pAnimationTime)
{
    for (int index = 0; index < mNumPositions - 1; ++index)
        if (pAnimationTime < mPositions[index + 1].mTimeStamp)
            return index;
    return 0;
}

int Bone::getRotationIndex(float pAnimationTime)
{
    for (int index = 0; index < mNumRotations - 1; ++index)
        if (pAnimationTime < mRotations[index + 1].mTimeStamp)
            return index;
    return 0;
}

int Bone::getScaleIndex(float pAnimationTime)
{
    for (int index = 0; index < mNumScalings - 1; ++index)
        if (pAnimationTime < mScales[index + 1].mTimeStamp)
            return index;
    return 0;
}

float Bone::getScaleFactor(float pLastTimeStamp, float pNextTimeStamp, float pAnimationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = pAnimationTime - pLastTimeStamp;
    float framesDiff = pNextTimeStamp - pLastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

lm::mat4 Bone::interpolatePosition(float pAnimationTime)
{
    if (1 == mNumPositions)
        return lm::mat4::translation(mPositions[0].mPosition);

    int p0Index = getPositionIndex(pAnimationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(mPositions[p0Index].mTimeStamp, mPositions[p1Index].mTimeStamp, pAnimationTime);
    lm::vec3 finalPosition = lm::vec3::lerp(mPositions[p0Index].mPosition, mPositions[p1Index].mPosition, scaleFactor);
    return lm::mat4::translation(finalPosition);
}

lm::mat4 Bone::interpolateRotation(float pAnimationTime)
{
    if (1 == mNumRotations)
    {
        lm::quat rotation = mRotations[0].mOrientation;
        return rotation.toMat4();
    }

    
    int p0Index = getRotationIndex(pAnimationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(mRotations[p0Index].mTimeStamp, mRotations[p1Index].mTimeStamp, pAnimationTime);
    lm::quat finalRotation = lm::quat::slerp(mRotations[p0Index].mOrientation, mRotations[p1Index].mOrientation, scaleFactor);
    return finalRotation.toMat4();
}

lm::mat4 Bone::interpolateScaling(float pAnimationTime)
{
    if (1 == mNumScalings)
        return lm::mat4::scale(mScales[0].mScale);

    int p0Index = getScaleIndex(pAnimationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(mScales[p0Index].mTimeStamp, mScales[p1Index].mTimeStamp, pAnimationTime);
    lm::vec3 finalScale = lm::vec3::lerp(mScales[p0Index].mScale, mScales[p1Index].mScale, scaleFactor);
    return lm::mat4::scale(finalScale);
}

lm::vec3 Bone::getVec(const aiVector3D& pVec)
{
    return lm::vec3(pVec.x, pVec.y, pVec.z);
}

lm::quat Bone::getQuat(const aiQuaternion& pOrientation)
{
    return lm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}