#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>

#include "CLIApp.h"

int main(int argc, char* argv[])
{
    try {
        DTE::CLIApp app(argc, argv);
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
} 