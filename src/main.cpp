#include <stdexcept>
#include <iostream>

#include "vk/vk_application.hpp"
#include "test.cpp"

int main()
{
    volkInitialize();

    try
    {
        vk::vk_application::getInstance().run();

        return EXIT_SUCCESS;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return EXIT_FAILURE;
}