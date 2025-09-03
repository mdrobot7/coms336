#include <iostream>
#include "render.hpp"

int main(int argc, char *argv[])
{
    Render render(1024, 768, 10);
    render.run();
    render.save("test");
    std::cout << "hello world" << std::endl;
    return 0;
}