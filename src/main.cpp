#include <iostream>
#include "render.hpp"
#include "scene.hpp"

int main(int argc, char *argv[])
{
    Render render(1024, 768, 10);
    render.run();
    render.save("test");

    Scene s;
    s.load("scenes/sample.json");

    std::cout << "hello world" << std::endl;
    return 0;
}