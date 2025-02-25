#include "test_manager.h"

#include <containers/darray.h>
#include <core/logger.h>
#include <utils/hstring.h>
#include <core/hclock.h>

typedef struct test_entry {
    PFN_test func;
    char* desc;
} test_entry;

static test_entry* tests;

void test_manager_init() {
    tests = darray_create(test_entry);
}

void test_manager_register_test(u8 (*PFN_test)(), char* desc) {
    test_entry e;
    e.func = PFN_test;
    e.desc = desc;
    darray_push(tests, e);
}

void test_manager_run_tests() {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = darray_length(tests);

    hclock total_time;
    startClock(&total_time);

    for (u32 i = 0; i < count; i++) {
        hclock test_time;
        startClock(&test_time);
        u8 result = tests[i].func();
        updateClock(&test_time);

        if (result == true) {
            ++passed;
        } 
        else if (result == BYPASS) {
            HWARNING("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        } 
        else {
            HERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
        string_format(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        updateClock(&total_time);
        HINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    stopClock(&total_time);

    HINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}