#include <stdexcept>
#include <iostream>

#include "vk/vk_application.hpp"

int main()
{
    volkInitialize();

    try
    {
        vk::vk_application app;
        app.run();

        return EXIT_SUCCESS;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return EXIT_FAILURE;
}