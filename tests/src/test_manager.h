#pragma once

#include <defines.h>

#ifdef __cplusplus 
extern "C" { 
#endif

#define BYPASS 2

typedef u8 (*PFN_test)();

void test_manager_init();

void test_manager_register_test(PFN_test, char* desc);

void test_manager_run_tests();

#ifdef __cplusplus
} 
#endif