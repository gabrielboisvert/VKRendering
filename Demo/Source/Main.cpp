#include "Application.h"
#include <cstdlib>

int main()
{
	Renderer::Application app("Vulkan", 800, 800);
	app.run();

	return EXIT_SUCCESS;
}