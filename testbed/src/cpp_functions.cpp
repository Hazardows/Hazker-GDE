// cpp_functions.cpp
#include "cpp_functions.hpp"

#include <iostream>

extern "C" {
    // Test function
    void cpp_function() {
        std::cout << "Hello from C++ function!" << std::endl;
        std::cout << " pd: Lorem ipsum dolor sit amet, consectetur adipiscing";
        std::cout << std::endl << ":)" << std::endl;
        std::cout << "Bye from C++ function!" << std::endl;
    }
}