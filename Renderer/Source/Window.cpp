#include "Window.h"
#include "VKRenderer.h"
#include <cstdlib>
#include <Application.h>

using namespace Renderer;

void Window::framebufferSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
{
	Renderer::Application* app = (Renderer::Application*)glfwGetWindowUserPointer(pWindow);
	app->mRenderer.mFramebufferResized = true;
	app->mCamera.changeViewPortSize(lm::mat4::perspectiveProjection(-45, float(pWidth) / float(pHeight), 0.1f, 500.f));
}

void Window::mousePosition(GLFWwindow* pWindow, double pX, double pY)
{
	Application* app = (Application*)glfwGetWindowUserPointer(pWindow);
	app->mCamera.setMousePosition(pX, pY);
}

void Window::mouseButton(GLFWwindow* pWindow, int pButton, int pAction, int pMods)
{
}

void Window::keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMods)
{
	Application* app = (Application*)glfwGetWindowUserPointer(pWindow);
	app->keyCallback(pKey, pScancode, pAction, pMods);
}

Window::Window(const char* pTitle, const unsigned int& pWidth, const unsigned int& pHeight) : mWidth(pWidth), mHeight(pHeight)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	mWindow = glfwCreateWindow(pWidth, pHeight, pTitle, nullptr, nullptr);
	if (mWindow == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Center window
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	if (primary != nullptr)
	{
		const GLFWvidmode* desktop = glfwGetVideoMode(primary);
		glfwSetWindowPos(mWindow, (desktop->width - pWidth) / 2, (desktop->height - pHeight) / 2);
	}

	glfwMakeContextCurrent(mWindow);
	glfwSetFramebufferSizeCallback(mWindow, Window::framebufferSizeCallback);
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(mWindow, Window::mousePosition);
	glfwSetMouseButtonCallback(mWindow, Window::mouseButton);
	glfwSetKeyCallback(mWindow, Window::keyCallback);
}

Window::~Window()
{
}

void Window::close()
{
	glfwSetWindowShouldClose(mWindow, true);
}

void Window::shutDown()
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(mWindow);
}

void Window::pollEvents()
{
	glfwPollEvents();
}

void Window::waitEvent()
{
	glfwWaitEvents();
}

void Window::setWindowUserPointer(void* pointer)
{
	glfwSetWindowUserPointer(mWindow, pointer);
}

int Window::getKey(const unsigned int& key)
{
	return glfwGetKey(mWindow, key);
}

void Window::bindContext(bool pBinded)
{
	if (pBinded)
		glfwMakeContextCurrent(mWindow);
	else
		glfwMakeContextCurrent(nullptr);
}

const char** Window::getRequiredInstanceExtensions(uint32_t& pExtensionCount)
{
	return glfwGetRequiredInstanceExtensions(&pExtensionCount);
}

void Window::getFramebufferSize(int* pWidth, int* pHeight)
{
	glfwGetFramebufferSize(mWindow, pWidth, pHeight);
}

int Window::createWindowSurface(VKRenderer& pRenderer)
{
	return glfwCreateWindowSurface(pRenderer.mInstance, mWindow, nullptr, &pRenderer.mSurface);
}