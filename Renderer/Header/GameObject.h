#pragma once
#include "Camera.h"
#include "Animator.h"
#include "UniformBuffer.h"

namespace Renderer
{
	struct UniformBufferObject {
		__declspec(align(16)) lm::mat4 mModel;
		__declspec(align(16)) lm::mat3 mInverseModel;
		__declspec(align(16)) lm::mat4 mVP;
		__declspec(align(16)) lm::vec3 mView;
		__declspec(align(16)) lm::mat4 mFinalBonesMatrices[MAX_BONE];
		__declspec(align(16)) bool mHasAnimation;
	};

	class GameObject
	{
	public:
		VKRenderer& mRenderer;
		UniformBuffer mUniBuffer;
		UniformBufferObject mData{};

		lm::mat4 mLocal = lm::mat4::identity;
		lm::mat4 mGlobal = lm::mat4::identity;
		lm::vec3* mV = nullptr;
		lm::mat4* mVP = nullptr;


		Model** mModel = nullptr;
		Shader** mShader = nullptr;
		Texture** mTexture = nullptr;

		lm::vec3 mRight = lm::vec3::right;
		lm::vec3 mForward = lm::vec3::forward;
		lm::vec3 mUp = lm::vec3::up;

		lm::vec3 mPosition = lm::vec3::zero;
		lm::vec3 mRotation = lm::vec3::zero;
		lm::vec3 mScale = lm::vec3::unitVal;

		GameObject* mParent = nullptr;
		std::list<GameObject*> mChilds;

		Animator* mAnimator = nullptr;

		GameObject(VKRenderer& pRenderer, Camera& pCamera, Model** pModel, Shader** pShader, Texture** pTexture, const lm::vec3& pPosition, const lm::vec3& pRotation, const lm::vec3& pScale);
		~GameObject();

		void addChild(GameObject& pChild);
		void removeChild(GameObject& pChild);

		void updateGlobal();
		void updateLocal();
		void update(float pDeltaTime);
		void draw();

		lm::vec3 GameObject::getGlobalScale(const GameObject& pObj, lm::vec3& pScale);

	};
}