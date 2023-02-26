#include "ResourceManager.h"


using namespace Renderer;

ResourceManager::~ResourceManager()
{
	mPool.stop();
	for (std::unordered_map<std::string, IResource*>::iterator it = mManager.begin(); it != mManager.end(); it++)
		delete it->second;
	mManager.clear();
}

void ResourceManager::deleteRes(const std::string& pStr)
{
	std::unordered_map<std::string, IResource*>::iterator it = mManager.find(pStr);
	if (it != mManager.end())
	{
		delete it->second;
		mManager.erase(it);
	}
}

void ResourceManager::clear()
{
	for (std::unordered_map<std::string, IResource*>::iterator it = mManager.begin(); it != mManager.end();)
		if (it->second != nullptr)
		{
			delete it->second;
			mManager.erase(it++);
		}
		else
			++it;
}