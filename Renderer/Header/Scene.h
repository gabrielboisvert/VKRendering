#pragma once
#include "Light.h"
#include "StorageBuffer.h"
#include "Shader.h"

#define MAX_LIGHT 10

namespace Renderer
{
	struct DirLights
	{
		int mSize = 0;
		DirectionalLight mData[MAX_LIGHT];
	};

	struct PointLights
	{
		int mSize = 0;
		PointLight mData[MAX_LIGHT];
	};

	struct SpotLights
	{
		int mSize = 0;
		SpotLight mData[MAX_LIGHT];
	};

	template <class T> class Scene
	{
		public:
			std::vector<T*> mGameObjects;

			DirLights mDirLights;
			PointLights mPointLights;
			SpotLights mSpotLights;

			StorageBuffer mStoreBuffer;

			VKRenderer& mRenderer;
			

			Scene(VKRenderer& pRenderer) : mRenderer(pRenderer), mStoreBuffer(pRenderer) {}

			void init()
			{
				mStoreBuffer.init(VK_SHADER_STAGE_FRAGMENT_BIT);
			}

			T* addNode(T* pNode)
			{
				mGameObjects.push_back(pNode);
				return mGameObjects.back();
			}

			void removeNode(T& pNode)
			{
				std::vector<T*>::iterator it = std::find(mGameObjects.begin(), mGameObjects.end(), &pNode);
				if (it != mGameObjects.end())
					mGameObjects.erase(it);
			}

			void draw()
			{
				for (unsigned int i = 0; i < this->mGameObjects.size(); i++)
					mGameObjects[i]->draw();
			}

			void update(float pDeltaTime)
			{
				for (unsigned int i = 0; i < this->mGameObjects.size(); i++)
					mGameObjects[i]->update(pDeltaTime);
			}

			std::vector<T*>& getGameObjects()
			{
				return mGameObjects;
			}

			~Scene()
			{
				clear();
			}

			void clear()
			{
				for (unsigned int i = 0; i < this->mGameObjects.size(); i++)
					delete mGameObjects[i];
				mGameObjects.clear();
			}

			void addLight(DirectionalLight* pLight)
			{
				mDirLights.mData[mDirLights.mSize++] = *pLight;
			}

			void removeLight(DirectionalLight* pLight)
			{
				for (unsigned int i = 0; i < mDirLights.mSize; i++)
				{
					if (mDirLights.mData[i] == *pLight)
					{
						for (unsigned int j = i; j < mDirLights.mSize - 1; j++)
							mDirLights.mData[j] = mDirLights.mData[j + 1];

						mDirLights.mSize--;
						break;
					}
				}
			}

			void addLight(PointLight* pLight)
			{
				mPointLights.mData[mPointLights.mSize++] = *pLight;
			}

			void removeLight(PointLight* pLight)
			{
				for (unsigned int i = 0; i < mPointLights.mSize; i++)
				{
					if (mPointLights.mData[i] == *pLight)
					{
						for (unsigned int j = i; j < mPointLights.mSize - 1; j++)
							mPointLights.mData[j] = mPointLights.mData[j + 1];

						mPointLights.mSize--;
						break;
					}
				}
			}

			void addLight(SpotLight* pLight)
			{
				mSpotLights.mData[mSpotLights.mSize++] = *pLight;
			}

			void removeLight(SpotLight* pLight)
			{
				for (unsigned int i = 0; i < mSpotLights.mSize; i++)
				{
					if (mSpotLights.mData[i] == *pLight)
					{
						for (unsigned int j = i; j < mSpotLights.mSize - 1; j++)
							mSpotLights.mData[j] = mSpotLights.mData[j + 1];

						mSpotLights.mSize--;
						break;
					}
				}
			}

			void sendLight(Shader& pShader)
			{
				pShader.bind();

				if (mDirLights.mSize != 0)
					pShader.setLight(&mStoreBuffer.DescriptorSets[mRenderer.mCurrentFrame], mStoreBuffer.mDirectionalLightStorageBuffersMapped[mRenderer.mCurrentFrame], &mDirLights, sizeof(DirLights));

				if (mPointLights.mSize != 0)
					pShader.setLight(&mStoreBuffer.DescriptorSets[mRenderer.mCurrentFrame], mStoreBuffer.mPointLightStorageBuffersMapped[mRenderer.mCurrentFrame], &mPointLights, sizeof(PointLight));

				if (mSpotLights.mSize != 0)
					pShader.setLight(&mStoreBuffer.DescriptorSets[mRenderer.mCurrentFrame], mStoreBuffer.mSpotLightStorageBuffersMapped[mRenderer.mCurrentFrame], &mSpotLights, sizeof(SpotLight));
			}
	};
}