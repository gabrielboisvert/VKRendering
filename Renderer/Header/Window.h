#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer
{
	class VKRenderer;

	class Window
	{
	public:
		GLFWwindow* mWindow;
		unsigned int mWidth;
		unsigned int mHeight;

		static void framebufferSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight);
		static void mousePosition(GLFWwindow* pWindow, double pX, double pY);
		static void mouseButton(GLFWwindow* pWindow, int pButton, int pAction, int pMods);
		static void keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMods);

		Window(const char* pTitle, const unsigned int& pWidth, const unsigned int& pHeight);
		~Window();

		void close();
		void shutDown();
		bool shouldClose();
		void pollEvents();
		void waitEvent();
		void setWindowUserPointer(void* pPointer);
		int getKey(const unsigned int& pKey);
		void bindContext(bool pBinded);

		const char** getRequiredInstanceExtensions(uint32_t& pExtensionCount);
		void getFramebufferSize(int* pWidth, int* pHeight);
		int createWindowSurface(VKRenderer& pRenderer);
	};
}