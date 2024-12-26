// cpp_functions.cpp

#include <iostream>
#include <cpp_functions.hpp>

extern "C" {
    void cpp_function() {
        std::cout << "Hello from C++ function!" << std::endl;
    }
}
