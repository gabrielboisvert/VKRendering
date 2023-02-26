#include "GameObject.h"

using namespace Renderer;

GameObject::GameObject(VKRenderer& pRenderer, Camera& pCamera, Model** pModel, Shader** pShader, Texture** pTexture, const lm::vec3& pPosition, const lm::vec3& pRotation, const lm::vec3& pScale) :
	mRenderer(pRenderer),
	mV(&pCamera.mPosition),
	mVP(&pCamera.mVp),
	mModel(pModel),
	mShader(pShader),
	mTexture(pTexture),
	mLocal(lm::mat4::createTransformMatrix(pPosition, pRotation, pScale)),
	mGlobal(mLocal),
	mPosition(pPosition),
	mRotation(pRotation),
	mScale(pScale),
	mUniBuffer(mRenderer)
{
	mUniBuffer.init(sizeof(UniformBufferObject), VK_SHADER_STAGE_VERTEX_BIT);
}

GameObject::~GameObject()
{
	for (std::list<GameObject*>::iterator it = mChilds.begin(); it != mChilds.end(); it++)
		delete* it;
	mChilds.clear();

	if (mAnimator != nullptr)
		delete mAnimator;
}

void GameObject::addChild(GameObject& pChild)
{
	if (pChild.mParent == this)
		return;

	if (pChild.mParent != nullptr)
		pChild.mParent->removeChild(pChild);

	pChild.mParent = this;
	mChilds.push_back(&pChild);


	lm::vec3 scale = getGlobalScale(*this, lm::vec3(1, 1, 1));
	pChild.mScale = lm::vec3(pChild.mScale.X() / scale.X(), pChild.mScale.Y() / scale.Y(), pChild.mScale.Z() / scale.Z());
	pChild.mPosition = lm::vec3((pChild.mPosition.X() - mPosition.X()) / scale.X(), (pChild.mPosition.Y() - mPosition.Y()) / scale.Y(), (pChild.mPosition.Z() - mPosition.Z()) / scale.Z());
	pChild.updateLocal();
	updateGlobal();
}

void GameObject::removeChild(GameObject& pChild)
{
	if (pChild.mParent != this)
		return;

	pChild.mParent = nullptr;
	mChilds.remove(&pChild);


	lm::vec3 scale = getGlobalScale(*this, lm::vec3(1, 1, 1));
	pChild.mScale = lm::vec3(pChild.mScale.X() * scale.X(), pChild.mScale.Y() * scale.Y(), pChild.mScale.Z() * scale.Z());
	pChild.mPosition = lm::vec3((pChild.mPosition.X() * scale.X()) + mPosition.X(), (pChild.mPosition.Y() * scale.Y()) + mPosition.Y(), (pChild.mPosition.Z() * scale.Z()) + mPosition.Z());
	pChild.updateLocal();
	updateGlobal();
}

void GameObject::updateGlobal()
{
	if (mParent != nullptr)
		mGlobal = mParent->mGlobal * mLocal;
	else
		mGlobal = mLocal;

	for (std::list<GameObject*>::iterator it = mChilds.begin(); it != mChilds.end(); it++)
		(*it)->updateGlobal();
}

void GameObject::updateLocal()
{
	mLocal = lm::mat4::createTransformMatrix(mPosition, mRotation, mScale);
	updateGlobal();
}

void GameObject::update(float pDeltaTime)
{
	if (pDeltaTime == 0)
		return;

	if (mModel != nullptr && *mModel != nullptr && (*mModel)->mAnimation != nullptr)
	{
		if (mAnimator == nullptr)
			mAnimator = new Animator((*mModel)->mAnimation);
		else
			mAnimator->updateAnimation(pDeltaTime);
	}

	updateGlobal();
}

void GameObject::draw()
{
	for (std::list<GameObject*>::iterator it = mChilds.begin(); it != mChilds.end(); it++)
		(*it)->draw();

	if (*mShader == nullptr || *mModel == nullptr)
		return;

	(*mShader)->bind();

	if (mTexture != nullptr && *mTexture != nullptr)
		(*mShader)->setTexture((*mTexture)->mTextureSets[mRenderer.mCurrentFrame]);

	
	mData.mModel = mGlobal;
	mData.mInverseModel = lm::mat3(mGlobal).inverse().transpose();
	mData.mVP = (*mVP) * mGlobal;
	mData.mView = (*mV);

	if (mAnimator != nullptr)
	{
		std::vector<lm::mat4>& transforms = mAnimator->mFinalBoneMatrices;
		for (int i = 0; i < transforms.size(); ++i)
			mData.mFinalBonesMatrices[i] = transforms[i];

		mData.mHasAnimation = true;
	}
	else
		mData.mHasAnimation = false;

	(*mShader)->setMVP(mUniBuffer.mDescriptorSets[mRenderer.mCurrentFrame], mUniBuffer.mUniformBuffersMapped[mRenderer.mCurrentFrame], &mData, sizeof(UniformBufferObject));
	
	(*mModel)->draw(*(*mShader));
}

lm::vec3 GameObject::getGlobalScale(const GameObject& pObj, lm::vec3& pScale)
{
	if (pObj.mParent == nullptr)
		return pScale.scaled(pObj.mScale);
	return getGlobalScale(*pObj.mParent, pScale.scaled(pObj.mScale));
}