#include <iostream>
#include "render.hpp"
#include "scene.hpp"

/*
Data related to how things are placed in the scene is
passed through the scene file. These args are related
to how the renderer generates the scene.

Render args:
- resolution
- anti-aliasing level
- scene file

Meta:
- job count
*/

int main(int argc, char *argv[])
{
    Scene s;
    std::cout << "Building scene..." << std::endl;
    s.load("scenes/test.json");

    Render render(s, 1024, 768, 1, 10);
    std::cout << "Launching renderer..." << std::endl;
    render.run();
    std::cout << "Saving output..." << std::endl;
    render.save("test");

    std::cout << "Done." << std::endl;
    return 0;
}