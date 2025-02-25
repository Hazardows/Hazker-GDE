#pragma once

#include "renderer/vulkan/vk_types.inl"

/**
 * Returns the string representation of result.
 * @param result The result to get the string for.
 * @param get_exetended Indicates whether to also return an extended result.
 * @return The error code and/or extended error message in string form. Default to success for unknown result types.
 */
const char* vkr_toString(VkResult result, b8 get_exetended);

/**
 * Indicates if the passed result is a success or an error as defined by the Vulkan spec.
 * @returns true if success, otherwise false. Defaults to true for unknown result types.
 */
b8 vkr_isSuccess(VkResult result);