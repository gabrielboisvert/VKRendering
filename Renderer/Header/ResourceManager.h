#pragma once

#include "ThreadPool.h"
#include <unordered_map>
#include <string>


namespace Renderer
{
	class IResource 
	{
		public:	
			virtual ~IResource() {};
	};

	class ResourceManager
	{
		public:
			std::unordered_map<std::string, IResource*> mManager;
			ThreadPool mPool;

			~ResourceManager();

			template <typename T, typename ...Args> T** create(const std::string& pStr, Args&... pArgs);

			template <typename T> T* get(const std::string& pStr);

			void deleteRes(const std::string& str);
			void clear();
	};


	template <typename T, typename ...Args> T** ResourceManager::create(const std::string& pStr, Args&... pArgs)
	{
		std::unordered_map<std::string, IResource*>::iterator it = mManager.find(pStr);
		if (it != mManager.end())
			return (T**)&it->second;

		mManager[pStr] = nullptr;
		mPool.queueJob([this, pStr, &pArgs...]
			{
				mManager[pStr] = new T(pArgs...);
			});

		return (T**)&mManager[pStr];
	}

	template <typename T> T* ResourceManager::get(const std::string& pStr)
	{
		std::unordered_map<std::string, IResource*>::iterator it = mManager.find(pStr);
		if (it == mManager.end())
			return nullptr;

		return (T*)it->second;
	}
}