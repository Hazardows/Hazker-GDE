// cpp_functions.cpp
#include "cpp_functions.hpp"

#include <iostream>

#include <core/input.h>
#include <core/logger.h>

extern "C" {
    // Test function
    void cpp_function() {
        std::cout << "Hello from C++ function!" << std::endl;
        std::cout << " pd: Lorem ipsum dolor sit amet, consectetur adipiscing";
        std::cout << std::endl << ":)" << std::endl;
        std::cout << "Bye from C++ function!" << std::endl;
    }

    void cpp_button() {
        if (buttonJustPressed(BUTTON_LEFT)) {
            HINFO("Left button was just pressed");
        }
        else if (buttonJustReleased(BUTTON_LEFT)) {
            HINFO("Left button was just released");
        }
        else if (buttonJustPressed(BUTTON_RIGHT)) {
            HINFO("Right button was just pressed");
        }
        else if (buttonJustReleased(BUTTON_RIGHT)) {
            HINFO("Right button was just released");
        }
        else if (buttonJustPressed(BUTTON_MIDDLE)) {
            HINFO("Middle button was just pressed");
        }
        else if (buttonJustReleased(BUTTON_MIDDLE)) {
            HINFO("Middle button was just released");
        }
    }
}