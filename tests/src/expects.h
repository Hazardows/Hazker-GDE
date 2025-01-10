#include <core/logger.h>
#include <math/hmath.h>

/**
 * @brief Expects expected to be equal to actual
 */
#define expect_should_be(expected, actual)                                                              \
    if (expected != actual) {                                                                           \
        HERROR("--> Expected %lld, but got %lld. File: %s:%d", expected, actual, __FILE__, __LINE__);   \
        return false;                                                                                   \
    }

/**
 * @brief Expects expected to not be equal to actual
 */
#define expect_should_not_be(expected, actual)                                                                          \
    if (expected == actual) {                                                                                       \
        HERROR("--> Expected %d != %d, but they are equal. File: %s:%d", expected, actual, __FILE__, __LINE__);     \
        return false;                                                                                               \
    }

/**
 * @brief Expects expected to be actual given a tolerance of H_FLOAT_EPSILON
 */
#define expect_float_to_be(expected, actual)                                                        \
    if (habs(expected- actual) > 0.001f) {                                                          \
        HERROR("--> Expected %f, but got %f. File: %s:%d", expected, actual, __FILE__, __LINE__);   \
        return false;                                                                               \
    }

/**
 * @brief Expects actual to be true.
 */
#define expect_to_be_true(actual)                                                       \
    if (!actual) {                                                                      \
        HERROR("--> Expected true, but got false. File: %s:%d", __FILE__, __LINE__);    \
        return 0;                                                                       \
    }

/**
 * @brief Expects actual to be false.
 */
#define expect_to_be_false(actual)                                                      \
    if (actual) {                                                                       \
        HERROR("--> Expected false, but got true. File: %s:%d", __FILE__, __LINE__);    \
        return 0;                                                                       \
    }