#include "test_manager.h"
#include "memory/linear_allocator_tests.h"

#include <core/logger.h>

int main() {
    // Always initialize the test manager at first.
    test_manager_init();

    // TODO: add test registrations here.
    linear_allocator_register_tests();

    HDEBUG("Starting tests...");

    // Execute tests
    test_manager_run_tests();

    return 0;
}